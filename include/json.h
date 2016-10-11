//
// Created by mkg on 10/7/2016.
//

#ifndef CS430_PROJ3_ILLUMINATION_JSON_H
#define CS430_PROJ3_ILLUMINATION_JSON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "base.h"

#define MAX_OBJECTS 128     // maximum number of objects supported in json file
#define CAMERA 1
#define SPHERE 2
#define PLANE 3
#define LIGHT 4

// structs to store different types of objects
typedef struct camera_t {
    double width;
    double height;
} Camera;

typedef struct sphere_t {
    double *diff_color;
    double *spec_color;
    double *position;
    double radius;
} Sphere;

typedef struct plane_t {
    double *diff_color;     // diffuse color
    double *spec_color;     // specular color
    double *position;
    double *normal;
} Plane;

typedef struct light_t {
    double *color;
    double *position;
    double *direction;
    double rad_att0;
    double rad_att1;
    double rad_att2;
    double ang_att0;
} Light;

// object datatype to store json data
typedef struct object_t {
    int type;  // -1 so we can check if the object has been populated
    union {
        Camera camera;
        Sphere sphere;
        Plane plane;
        Light light;
    };
} object;

/* global variables */
extern int line;
extern object objects[MAX_OBJECTS];

/* function definitions */
void read_json(FILE *json);
void print_objects(object *obj);

#endif //CS430_PROJ3_ILLUMINATION_JSON_H
