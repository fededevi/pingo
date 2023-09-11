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
        //a.w = 1.0 / a.w;
        //b.w = 1.0 / b.w;
        //c.w = 1.0 / c.w;
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

        minX = MIN(MAX(minX, 0), r->framebuffer.size.x);
        minY = MIN(MAX(minY, 0), r->framebuffer.size.y);
        maxX = MIN(MAX(maxX, 0), r->framebuffer.size.x);
        maxY = MIN(MAX(maxY, 0), r->framebuffer.size.y);

        // Barycentric coordinates at minX/minY corner
        Vec2i minTriangle = {minX, minY};

        int32_t area = orient2d(a_s, b_s, c_s);
        if (area == 0)
            continue;
        float areaInverse = 1.0 / area;

        int32_t A01 = (a_s.y - b_s.y); //Barycentric coordinates steps
        int32_t B01 = (b_s.x - a_s.x); //Barycentric coordinates steps
        int32_t A12 = (b_s.y - c_s.y); //Barycentric coordinates steps
        int32_t B12 = (c_s.x - b_s.x); //Barycentric coordinates steps
        int32_t A20 = (c_s.y - a_s.y); //Barycentric coordinates steps
        int32_t B20 = (a_s.x - c_s.x); //Barycentric coordinates steps

        int32_t w0_row = orient2d(b_s, c_s, minTriangle);
        int32_t w1_row = orient2d(c_s, a_s, minTriangle);
        int32_t w2_row = orient2d(a_s, b_s, minTriangle);

        if (o->material != 0) {
            tca.x /= a.z;
            tca.y /= a.z;
            tcb.x /= b.z;
            tcb.y /= b.z;
            tcc.x /= c.z;
            tcc.y /= c.z;
        }

        for (int16_t y = minY; y < maxY; y++, w0_row += B12, w1_row += B20, w2_row += B01) {
            int32_t w0 = w0_row;
            int32_t w1 = w1_row;
            int32_t w2 = w2_row;

            for (int32_t x = minX; x < maxX; x++, w0 += A12, w1 += A20, w2 += A01) {
                if ((w0 | w1 | w2) < 0)
                    continue;

                float depth = -(w0 * a.z + w1 * b.z + w2 * c.z) * areaInverse;
                if (depth < -1.0 || depth > 1.0)
                    continue;

                if (depth_check(r->backend->getZetaBuffer(r, r->backend),
                                x + y * scrSize.x,
                                depth))
                    continue;

                depth_write(r->backend->getZetaBuffer(r, r->backend), x + y * scrSize.x, depth);

                if (o->material != 0) {
                    //Texture lookup

                    float textCoordx = -(w0 * tca.x + w1 * tcb.x + w2 * tcc.x) * areaInverse * depth;
                    float textCoordy = -(w0 * tca.y + w1 * tcb.y + w2 * tcc.y) * areaInverse * depth;

                    Pixel text = texture_readF(o->material->texture,
                                               (Vec2f){textCoordx, textCoordy});
                    texture_draw(&r->framebuffer, (Vec2i){x, y}, pixelMul(text, diffuseLight));
                } else {
                    texture_draw(&r->framebuffer,
                                 (Vec2i){x, y},
                                 pixelMul(pixelFromUInt8(255), diffuseLight));
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
