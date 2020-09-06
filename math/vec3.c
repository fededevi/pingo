#include "vec3.h"

Vec3f vec3fmul(Vec3f a, float b)
{
    a.x = a.x * b;
    a.y = a.y * b;
    a.y = a.y * b;

    return a;
}

Vec3f vec3fsumV(Vec3f a, Vec3f b)
{
    a.x = a.x + b.x;
    a.y = a.y + b.y;
    a.y = a.y + b.z;

    return a;
}

Vec3f vec3fsum(Vec3f a, float b)
{
    a.x = a.x + b;
    a.y = a.y + b;
    a.y = a.y + b;

    return a;
}

float vec3Dot(Vec3f a, Vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f vec3f(float x, float y, float z)
{
    return (Vec3f){x,y,z};
}
