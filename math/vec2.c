#include "vec2.h"

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4244)
#endif

Vec2i vector2ISum(Vec2i l, Vec2i r)
{
    return (Vec2i){l.x+r.x,l.y+r.y};
}

Vec2f vecItoF(Vec2i v)
{
    return (Vec2f){v.x,v.y};
}

Vec2i vecFtoI(Vec2f v)
{
    return (Vec2i){v.x,v.y};
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif