#include "vec2.h"

Vector2I vector2ISum(Vector2I l, Vector2I r)
{
    return (Vector2I){l.x+r.x,l.y+r.y};
}

Vec2f vecItoF(Vector2I v)
{
    return (Vec2f){v.x,v.y};
}

Vector2I vecFtoI(Vec2f v)
{
    return (Vector2I){v.x,v.y};
}
