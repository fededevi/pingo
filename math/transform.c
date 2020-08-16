#include "transform.h"
#include <math.h>

const Transform transformIdentity = {  1,  0,  0,
                                       0,  1,  0,
                                       0,  0,  1};

/*
Builds a clean translation matrix with x and y translation along relative axes

 |1|0|x|
 |0|1|y|
 |0|0|1|

*/
Transform transformTranslate(Vector2I l)
{
    float x = l.x;
    float y = l.y;
    return (Transform){
        1,  0,  x,
        0,  1,  y,
        0,  0,  1
    };
}

/*
Builds a clean rotation matrix of Θ angle

 |1|0|0|
 |0|1|0|
 |0|0|1|

*/
Transform transformRotate(float Θ)
{
    float s = sin(Θ);
    float c = cos(Θ);
    return (Transform){
        c, -s,  0,
        s,  c,  0,
        0,  0,  1
    };
}

Vector2I transformMultiply(Vector2I *v, Transform *t)
{
    float a = v->x * t->elements[0] + v->y * t->elements[1] + 1.0 * t->elements[2];
    float b = v->x * t->elements[3] + v->y * t->elements[4] + 1.0 * t->elements[5];
    //float c = v->x * t->elements[6] + v->y * t->elements[7] + 1.0 * t->elements[8];
    return (Vector2I){a,b};
}
