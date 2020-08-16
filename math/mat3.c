#include "mat3.h"
#include "vec2.h"

#include <math.h>
#include <stdint.h>

const Mat3 transformIdentity = {  1,  0,  0,
                                  0,  1,  0,
                                  0,  0,  1};

/*
Builds a clean translation matrix with x and y translation along relative axes

 |1|0|x|
 |0|1|y|
 |0|0|1|

*/
Mat3 transformTranslate(Vec2f l) {
    T x = l.x;
    T y = l.y;
    return (Mat3){
        1,  0,  x,
        0,  1,  y,
        0,  0,  1
    };
}

/*
Builds a clean rotation matrix of Θ angle

 | c(Θ) | -s(Θ)  | 0 |
 | s(Θ) | c(Θ)   | 0 |
 | 0    | 0      | 1 |

*/
Mat3 transformRotate(float theta) {
    T s = sin(theta);
    T c = cos(theta);
    return (Mat3){
        c, -s,  0,
        s,  c,  0,
        0,  0,  1
    };
}

/*
Builds a clean scale matrix of x, y scaling factors

 | x | 0 | 0 |
 | 0 | y | 0 |
 | 0 | 0 | 1 |

*/
extern Mat3 transformScale(Vec2f s) {
    T p = s.x;
    T q = s.y;
    return (Mat3){
        p,  0,  0,
        0,  q,  0,
        0,  0,  1
    };
}

/*
Multiplies vector by matrix
*/
Vec2f transformMultiply(Vec2f *v, Mat3 *t) {
    T a = v->x * t->elements[0] + v->y * t->elements[1] + 1.0 * t->elements[2];
    T b = v->x * t->elements[3] + v->y * t->elements[4] + 1.0 * t->elements[5];
    //float c = v->x * t->elements[6] + v->y * t->elements[7] + 1.0 * t->elements[8];
    return (Vec2f){a,b};
}

/*
Multiplies m1 and m2 -> Appliers m2 to m1
so m1 is your starting transform, m2 is the added transform
*/
Mat3 transformMultiplyM( Mat3 * m1, Mat3 * m2) {
    Mat3 out;
    T * a = m2->elements;
    T * b = m1->elements;
    out.elements[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
    out.elements[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
    out.elements[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
    out.elements[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
    out.elements[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
    out.elements[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
    out.elements[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
    out.elements[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
    out.elements[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
    return out;
}

//Tells if a matrix is only made of a translation
int transformIsOnlyTranslation(Mat3 *m )
{
    if (m->elements[0] != 1.0) return 0;
    if (m->elements[1] != 0.0) return 0;
    //if (m->elements[2] != 0.0) return 0; This is a translation component
    if (m->elements[3] != 0.0) return 0;
    if (m->elements[4] != 1.0) return 0;
    //if (m->elements[5] != 1.0) return 0; This is a translation component
    if (m->elements[6] != 0.0) return 0;
    if (m->elements[7] != 0.0) return 0;
    if (m->elements[8] != 1.0) return 0;
    return 1;
}
