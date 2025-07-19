#include "vec3.h"
#include <math.h>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4244)
#endif

Vec3f vec3fmul(Vec3f a, F_TYPE b)
{
    a.x = a.x * b;
    a.y = a.y * b;
    a.z = a.z * b;

    return a;
}

Vec3f vec3fsumV(Vec3f a, Vec3f b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    a.z = a.z + b.z;

    return a;
}

Vec3f vec3fsubV(Vec3f a, Vec3f b)
{
    a.x = a.x - b.x;
    a.y = a.y - b.y;
    a.z = a.z - b.z;

    return a;
}

Vec3f vec3fsum(Vec3f a, F_TYPE b)
{
    a.x = a.x + b;
    a.y = a.y + b;
    a.z = a.z + b;

    return a;
}

F_TYPE vec3Dot(Vec3f a, Vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f vec3f(F_TYPE x, F_TYPE y, F_TYPE z)
{
    return (Vec3f){x,y,z};
}

Vec3f vec3Cross(Vec3f a, Vec3f b)
{

    return (Vec3f) {a.y * b.z - b.y * a.z,
                    a.z * b.x - b.z * a.x,
                    a.x * b.y - b.x * a.y};
}

Vec3f vec3Normalize(Vec3f v)
{
    F_TYPE length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0) {
        return (Vec3f){0, 0, 0};
    }
    return (Vec3f){v.x / length, v.y / length, v.z / length};
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif