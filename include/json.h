//
// Created by mkg on 10/7/2016.
//

#ifndef CS430_PROJ3_ILLUMINATION_JSON_H
#define CS430_PROJ3_ILLUMINATION_JSON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_OBJECTS 128     // maximume number of objects supported in json file
#define CAMERA 1
#define SPHERE 2
#define PLANE 3

// structs to store different types of objects
typedef struct camera_t {
    double width;
    double height;
} camera;

typedef struct sphere_t {
    double *color;
    double *position;
    double radius;
} sphere;

typedef struct plane_t {
    double *color;
    double *position;
    double *normal;
} plane;

// object datatype to store json data
typedef struct object_t {
    int type;  // -1 so we can check if the object has been populated
    union {
        camera cam;
        sphere sph;
        plane pln;
    };
} object;

/* global variables */
extern int line;
extern object objects[MAX_OBJECTS];

/* function definitions */
void read_json(FILE *json);
void print_objects(object *obj);

#endif //CS430_PROJ3_ILLUMINATION_JSON_H
