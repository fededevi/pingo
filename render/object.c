#include "object.h"
#include "backend.h"
#include "depth.h"
#include "math/fun.h"
#include "math/mat4.h"
#include "mesh.h"
#include "render/material.h"
#include "renderer.h"
#include "state.h"

int object_render(void *this, Mat4 m, Renderer *r)
{
    Object *o = this;

    IF_NULL_RETURN(o, RENDER_ERROR);
    IF_NULL_RETURN(r, RENDER_ERROR);

    const Vec2i scrSize = r->framebuffer.size;

    // VIEW MATRIX
    Mat4 v = mat4Inverse( &r->camera_view );
    Mat4 p = r->camera_projection;

    // Prefetch frequently used pointers/values
    Pixel *color_buffer = r->framebuffer.frameBuffer;
    const int framebuffer_width = scrSize.x;
    const int framebuffer_height = scrSize.y;
    PingoDepth *z_buffer = r->backend->getZetaBuffer(r, r->backend);

    for (int i = 0; i < o->mesh->indexes_count; i += 3) {
        Vec3f *ver1 = &o->mesh->positions[o->mesh->pos_indices[i + 0]];
        Vec3f *ver2 = &o->mesh->positions[o->mesh->pos_indices[i + 1]];
        Vec3f *ver3 = &o->mesh->positions[o->mesh->pos_indices[i + 2]];

        Vec2f tca = {0, 0};
        Vec2f tcb = {0, 0};
        Vec2f tcc = {0, 0};

        if (o->material != 0) {
            tca = o->mesh->textCoord[o->mesh->tex_indices[i + 0]];
            tcb = o->mesh->textCoord[o->mesh->tex_indices[i + 1]];
            tcc = o->mesh->textCoord[o->mesh->tex_indices[i + 2]];
        }

        Vec4f a = {ver1->x, ver1->y, ver1->z, 1};
        Vec4f b = {ver2->x, ver2->y, ver2->z, 1};
        Vec4f c = {ver3->x, ver3->y, ver3->z, 1};

        Mat4 vm = mat4MultiplyM(&v,&m);

        a = mat4MultiplyVec4(&a, &vm);
        b = mat4MultiplyVec4(&b, &vm);
        c = mat4MultiplyVec4(&c, &vm);

        //Calc Face Normal
        Vec3f na = vec3fsubV(*((Vec3f *) (&a)), *((Vec3f *) (&b)));
        Vec3f nb = vec3fsubV(*((Vec3f *) (&a)), *((Vec3f *) (&c)));
        Vec3f normal = vec3Normalize(vec3Cross(na, nb));
        Vec3f light = vec3Normalize((Vec3f){-8, 5, 5});
        float diffuseLight = (1.0 + vec3Dot(normal, light)) * 0.5;
        diffuseLight = MIN(1.0, MAX(diffuseLight, 0));

        a = mat4MultiplyVec4(&a, &p);
        b = mat4MultiplyVec4(&b, &p);
        c = mat4MultiplyVec4(&c, &p);


        //Triangle is completely behind camera
        if (a.z > 0 && b.z > 0 && c.z > 0)
            continue;

        // convert to device coordinates by perspective division
        a.x /= a.w;
        a.y /= a.w;
        a.z /= a.w;
        a.w = 1;
        b.x /= b.w;
        b.y /= b.w;
        b.z /= b.w;
        b.w = 1;
        c.x /= c.w;
        c.y /= c.w;
        c.z /= c.w;
        c.w = 1;

        float clocking = isClockWise(a.x, a.y, b.x, b.y, c.x, c.y);
        if (clocking >= 0)
            continue;

        //Compute Screen coordinates
        float halfX = scrSize.x / 2;
        float halfY = scrSize.y / 2;
        Vec2i a_s = {a.x * halfX + halfX, a.y * halfY + halfY};
        Vec2i b_s = {b.x * halfX + halfX, b.y * halfY + halfY};
        Vec2i c_s = {c.x * halfX + halfX, c.y * halfY + halfY};

        int32_t minX = MIN(MIN(a_s.x, b_s.x), c_s.x);
        int32_t minY = MIN(MIN(a_s.y, b_s.y), c_s.y);
        int32_t maxX = MAX(MAX(a_s.x, b_s.x), c_s.x);
        int32_t maxY = MAX(MAX(a_s.y, b_s.y), c_s.y);

        minX = MIN(MAX(minX, 0), framebuffer_width);
        minY = MIN(MAX(minY, 0), framebuffer_height);
        maxX = MIN(MAX(maxX, 0), framebuffer_width);
        maxY = MIN(MAX(maxY, 0), framebuffer_height);

        // Barycentric coordinates at minX/minY corner
        Vec2i minTriangle = {minX, minY};

        int32_t area = orient2d(a_s, b_s, c_s);
        if (area == 0)
            continue;
        float areaInverse = 1.0f / (float)area;

        int32_t A01 = (a_s.y - b_s.y); //Barycentric coordinates steps
        int32_t B01 = (b_s.x - a_s.x); //Barycentric coordinates steps
        int32_t A12 = (b_s.y - c_s.y); //Barycentric coordinates steps
        int32_t B12 = (c_s.x - b_s.x); //Barycentric coordinates steps
        int32_t A20 = (c_s.y - a_s.y); //Barycentric coordinates steps
        int32_t B20 = (a_s.x - c_s.x); //Barycentric coordinates steps

        int32_t w0_row = orient2d(b_s, c_s, minTriangle);
        int32_t w1_row = orient2d(c_s, a_s, minTriangle);
        int32_t w2_row = orient2d(a_s, b_s, minTriangle);

        // Prepare perspective-correct interpolation if textured
        Texture *texture = 0;
        Pixel *tex_buffer = 0;
        int tex_w = 0, tex_h = 0;
        int pow2_mask_x = 0, pow2_mask_y = 0;
        int use_pow2_wrap = 0;

        if (o->material != 0 && a.z != 0 && b.z != 0 && c.z != 0) {
            // Pre-divide texture coordinates by depth for perspective-correct interpolation
            tca.x /= a.z; tca.y /= a.z;
            tcb.x /= b.z; tcb.y /= b.z;
            tcc.x /= c.z; tcc.y /= c.z;

            texture   = o->material->texture;
            tex_buffer = texture->frameBuffer;
            tex_w = texture->size.x;
            tex_h = texture->size.y;
            // Power-of-two wrap optimization
            int pow2x = (tex_w & (tex_w - 1)) == 0;
            int pow2y = (tex_h & (tex_h - 1)) == 0;
            use_pow2_wrap = (pow2x && pow2y);
            if (use_pow2_wrap) {
                pow2_mask_x = tex_w - 1;
                pow2_mask_y = tex_h - 1;
            }
        }

        // Precompute incremental depth and texture coordinate steps
        float depth_row_start = -((float)w0_row * a.z + (float)w1_row * b.z + (float)w2_row * c.z) * areaInverse;
        const float depth_dx = -((float)A12 * a.z + (float)A20 * b.z + (float)A01 * c.z) * areaInverse;
        const float depth_dy = -((float)B12 * a.z + (float)B20 * b.z + (float)B01 * c.z) * areaInverse;

        float s_row_start = 0.0f, t_row_start = 0.0f;
        float s_dx = 0.0f, t_dx = 0.0f;
        float s_dy = 0.0f, t_dy = 0.0f;
        if (texture != 0) {
            s_row_start = -((float)w0_row * tca.x + (float)w1_row * tcb.x + (float)w2_row * tcc.x) * areaInverse;
            t_row_start = -((float)w0_row * tca.y + (float)w1_row * tcb.y + (float)w2_row * tcc.y) * areaInverse;
            s_dx = -((float)A12 * tca.x + (float)A20 * tcb.x + (float)A01 * tcc.x) * areaInverse;
            t_dx = -((float)A12 * tca.y + (float)A20 * tcb.y + (float)A01 * tcc.y) * areaInverse;
            s_dy = -((float)B12 * tca.x + (float)B20 * tcb.x + (float)B01 * tcc.x) * areaInverse;
            t_dy = -((float)B12 * tca.y + (float)B20 * tcb.y + (float)B01 * tcc.y) * areaInverse;
        }

        for (int16_t y = minY; y < maxY; y++, w0_row += B12, w1_row += B20, w2_row += B01, depth_row_start += depth_dy, s_row_start += s_dy, t_row_start += t_dy) {
            int32_t w0 = w0_row;
            int32_t w1 = w1_row;
            int32_t w2 = w2_row;

            float depth = depth_row_start;
            float s_lin = s_row_start;
            float t_lin = t_row_start;

            int row_index = y * framebuffer_width;

            for (int32_t x = minX; x < maxX; x++, w0 += A12, w1 += A20, w2 += A01, depth += depth_dx, s_lin += s_dx, t_lin += t_dx) {
                if ((w0 | w1 | w2) < 0)
                    continue;

                if (depth < -1.0f || depth > 1.0f)
                    continue;

                int idx = row_index + x;
                if (depth_check(z_buffer, idx, depth))
                    continue;

                depth_write(z_buffer, idx, depth);

                if (texture != 0) {
                    // Perspective-correct texture coordinates
                    float u = s_lin * depth;
                    float v = t_lin * depth;

                    // Map [0,1) to [0, tex_size)
                    uint32_t ux = (uint32_t)(u * (float)tex_w);
                    uint32_t vy = (uint32_t)(v * (float)tex_h);
                    if (use_pow2_wrap) {
                        ux &= (uint32_t)pow2_mask_x;
                        vy &= (uint32_t)pow2_mask_y;
                    } else {
                        ux %= (uint32_t)tex_w;
                        vy %= (uint32_t)tex_h;
                    }
                    Pixel text = tex_buffer[ux + vy * (uint32_t)tex_w];
                    color_buffer[idx] = pixelMul(text, diffuseLight);
                } else {
                    color_buffer[idx] = pixelMul(pixelFromUInt8(255), diffuseLight);
                }
            }
        }
    }

    return OK;
};

int object_init(Object *this, Mesh *mesh, Material *material)
{
    IF_NULL_RETURN(this, INIT_ERROR);
    IF_NULL_RETURN(mesh, INIT_ERROR);
    //IF_NULL_RETURN(material, INIT_ERROR);

    this->material = material;
    this->mesh = mesh;
    this->renderable.render = &object_render;

    return OK;
}
