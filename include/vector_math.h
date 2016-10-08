//
// Created by mkg on 10/7/2016.
//

#ifndef CS430_PROJ3_ILLUMINATION_VECTOR_MATH_H
#define CS430_PROJ3_ILLUMINATION_VECTOR_MATH_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef double V3[3];   // represents a 3d vector


static inline double sqr(double v) {
    return v*v;
}

static inline void normalize(double *v) {
    double len = sqr(v[0]) + sqr(v[1]) + sqr(v[2]);
    len = sqrt(len);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

static inline double v3_len(V3 a) {
    return sqrt(sqr(a[0]) + sqr(a[1]) + sqr(a[2]));
}

static inline void v3_add(V3 a, V3 b, V3 c) {
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

static inline void v3_sub(V3 a, V3 b, V3 c) {
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

static inline void v3_scale(V3 a, double s, V3 b) {
    b[0] = s * a[0];
    b[1] = s * a[1];
    b[2] = s * a[2];
}

static inline double v3_dot(V3 a, V3 b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline void v3_cross(V3 a, V3 b, V3 c) {
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}


/* testing/debug functions */
//void print_v3(V3 data){
//    printf("V3: (%lf, %lf, %lf)\n", data[0], data[1], data[2]);
//}
#endif //CS430_PROJ3_ILLUMINATION_VECTOR_MATH_H
