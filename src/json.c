//
// Created by mkg on 10/7/2016.
//
/* json.c parses json files for view objects */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "../include/json.h"

#define MAX_COLOR_VAL 255       // maximum value to use for colors 0-255

/* global variables */
int line = 1;                   // global var for line numbers as we parse
object objects[MAX_OBJECTS];    // allocate space for all objects in json file

/* helper functions */

// next_c wraps the getc function that provides error checking and line #
// Problem: if we do ungetc, it could screw us up on the line #
int next_c(FILE* json) {
    int c = fgetc(json);
#ifdef DEBUG
    printf("next_c: '%c'\n", c);
#endif
    if (c == '\n') {
        line++;;
    }
    if (c == EOF) {
        fprintf(stderr, "Error: next_c: Unexpected EOF: %d\n", line);
        exit(1);
    }
    return c;
}

/* skips any white space from current position to next character*/
void skip_ws(FILE *json) {
    int c = next_c(json);
    while (isspace(c)) {
        c = next_c(json);
    }
    if (c == '\n')
        line--;         // we backed up to the previous line
    ungetc(c, json);    // move back one character (instead of fseek)
}

/* checks that the next character is d */
void expect_c(FILE* json, int d) {
    int c = next_c(json);
    if (c == d) return;
    fprintf(stderr, "Error: Expected '%c': %d\n", d, line);
    exit(1);
}

/* gets the next value from a file - This is *expected* to be a number */
double next_number(FILE* json) {
    double val;
    int res = fscanf(json, "%lf", &val);
    if (res == EOF) {
        fprintf(stderr, "Error: Expected a number but found EOF: %d\n", line);
        exit(1);
    }
    return val;
}

/* since we could use 0-255 or 0-1 or whatever, this function checks bounds */
int check_color_val(double v) {
    if (v < 0.0 || v > MAX_COLOR_VAL)
        return 0;
    return 1;
}

/* gets the next 3 values from FILE as vector coordinates */
double* next_vector(FILE* json) {
    double* v = malloc(sizeof(double)*3);
    skip_ws(json);
    expect_c(json, '[');
    skip_ws(json);
    v[0] = next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[1] = next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[2] = next_number(json);
    skip_ws(json);
    expect_c(json, ']');
    return v;
}

/* Checks that the next 3 values in the FILE are valid rgb numbers */
double* next_rgb_color(FILE* json) {
    double* v = malloc(sizeof(double)*3);
    skip_ws(json);
    expect_c(json, '[');
    skip_ws(json);
    v[0] = MAX_COLOR_VAL * next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[1] = MAX_COLOR_VAL * next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[2] = MAX_COLOR_VAL * next_number(json);
    skip_ws(json);
    expect_c(json, ']');
    // check that all values are valid
    if (!check_color_val(v[0]) ||
        !check_color_val(v[1]) ||
        !check_color_val(v[2])) {
        fprintf(stderr, "Error: next_rgb_color: rgb value out of range: %d\n", line);
        exit(1);
    }
    return v;
}

/* grabs a string wrapped in quotes from FILE */
char* parse_string(FILE *json) {
    skip_ws(json);
    int c = next_c(json);
    if (c != '"') {
        fprintf(stderr, "Error: Expected beginning of string but found '%c': %d\n", c, line);
        exit(1); // not a string
    }
    c = next_c(json); // should be first char in the string
    char buffer[128]; // strings are gauranteed to be 128 or less
    int i = 0;
    while (c != '"') {
        if (isspace(c)) {
            continue;
        }
        buffer[i] = c;
        i++;
        c = next_c(json);
    }
    buffer[i] = 0;
    return strdup(buffer); // returns a malloc'd version of buffer
}

/**
 * Reads all scene info from a json file and stores it in the global object
 * array. This does a lot of work...It checks for specific values and keys in
 * the file and places the values into the appropriate portion of the current
 * object.
 * @param json file handler with ASCII json data
 */
void read_json(FILE *json) {
    //read in data from file
    // expecting square bracket but we need to get rid of whitespace
    skip_ws(json);

    // find beginning of the list
    int c  = next_c(json);
    if (c != '[') {
        fprintf(stderr, "Error: read_json: JSON file must begin with [\n");
        exit(1);
    }
    skip_ws(json);
    c = next_c(json);

    // check if file empty
    if (c == ']' || c == EOF) {
        fprintf(stderr, "Error: read_json: Empty json file\n");
        exit(1);
    }
    skip_ws(json);

    int counter = 0;

    // find the objects
    while (1) {
        //c  = next_c(json);
        if (counter > MAX_OBJECTS) {
            fprintf(stderr, "Error: read_json: Number of objects is too large: %d\n", line);
            exit(1);
        }
        if (c == ']') {
            fprintf(stderr, "Error: read_json: Unexpected ']': %d\n", line);
            fclose(json);
            return;
        }
        if (c == '{') {     // found an object
            skip_ws(json);
            char *key = parse_string(json);
            if (strcmp(key, "type") != 0) {
                fprintf(stderr, "Error: read_json: First key of an object must be 'type': %d\n", line);
                exit(1);
            }
            skip_ws(json);
            // get the colon
            expect_c(json, ':');
            skip_ws(json);

            char *type = parse_string(json);
            int obj_type;
            if (strcmp(type, "camera") == 0) {
                obj_type = CAMERA;
                objects[counter].type = CAMERA;
            }
            else if (strcmp(type, "sphere") == 0) {
                obj_type = SPHERE;
                objects[counter].type = SPHERE;
            }
            else if (strcmp(type, "plane") == 0) {
                obj_type = PLANE;
                objects[counter].type = PLANE;
            }
            else {
                exit(1);
            }

            skip_ws(json);

            while (1) {
                //  , }
                c = next_c(json);
                if (c == '}') {
                    // stop parsing this object
                    break;
                }
                else if (c == ',') {
                    // read another field
                    skip_ws(json);
                    char* key = parse_string(json);
                    skip_ws(json);
                    expect_c(json, ':');
                    skip_ws(json);
                    if (strcmp(key, "width") == 0) {
                        double temp = next_number(json);
                        if (temp <= 0) {
                            fprintf(stderr, "Error: read_json: width must be positive: %d\n", line);
                            exit(1);
                        }
                        objects[counter].cam.width = temp;

                    }
                    else if (strcmp(key, "height") == 0) {
                        double temp = next_number(json);
                        if (temp <= 0) {
                            fprintf(stderr, "Error: read_json: height must be positive: %d\n", line);
                            exit(1);
                        }
                        objects[counter].cam.height = temp;
                    }
                    else if (strcmp(key, "radius") == 0) {
                        double temp = next_number(json);
                        if (temp <= 0) {
                            fprintf(stderr, "Error: read_json: radius must be positive: %d\n", line);
                            exit(1);
                        }
                        objects[counter].sph.radius = temp;
                    }
                    else if (strcmp(key, "color") == 0) {
                        if (obj_type == SPHERE)
                            objects[counter].sph.color = next_rgb_color(json);
                        else if (obj_type == PLANE)
                            objects[counter].pln.color = next_rgb_color(json);
                        else {
                            fprintf(stderr, "Error: read_json: Color vector can't be applied here: %d\n", line);
                            exit(1);
                        }
                    }
                    else if (strcmp(key, "position") == 0) {
                        if (obj_type == SPHERE)
                            objects[counter].sph.position = next_vector(json);
                        else if (obj_type == PLANE)
                            objects[counter].pln.position = next_vector(json);
                        else {
                            fprintf(stderr, "Error: read_json: Position vector can't be applied here: %d\n", line);
                            exit(1);
                        }

                    }
                    else if (strcmp(key, "normal") == 0) {
                        if (obj_type != PLANE) {
                            fprintf(stderr, "Error: read_json: Normal vector can't be applied here: %d\n", line);
                            exit(1);
                        }
                        else
                            objects[counter].pln.normal = next_vector(json);
                    }
                    else {
                        fprintf(stderr, "Error: read_json: '%s' not a valid object: %d\n", key, line);
                        exit(1);
                    }

                    // do something with key and value
                    skip_ws(json);
                }
                else {
                    fprintf(stderr, "Error: read_json: Unexpected value '%c': %d\n", c, line);
                    exit(1);
                }
            }
            skip_ws(json);
            c = next_c(json);
            if (c == ',') {
                // noop
                skip_ws(json);
            }
            else if (c == ']') {
                fclose(json);
                return;
            }
            else {
                fprintf(stderr, "Error: read_json: Expecting comma or ]: %d\n", line);
                exit(1);
            }
        }
        c = next_c(json);
        counter++;
    }
    fclose(json);
}

/* testing/debug functions */
void print_objects(object *obj) {
    int i = 0;
    while (i < MAX_OBJECTS && obj[i].type > 0) {
        printf("object type: %d\n", obj[i].type);
        if (obj[i].type == CAMERA) {
            printf("height: %lf\n", obj[i].cam.height);
            printf("width: %lf\n", obj[i].cam.width);
        }
        else if (obj[i].type == SPHERE) {
            printf("color: %lf %lf %lf\n", obj[i].sph.color[0],
                   obj[i].sph.color[1],
                   obj[i].sph.color[2]);
            printf("position: %lf %lf %lf\n", obj[i].sph.position[0],
                   obj[i].sph.position[1],
                   obj[i].sph.position[2]);
            printf("radius: %lf\n", obj[i].sph.radius);
        }
        else if (obj[i].type == PLANE) {
            printf("color: %lf %lf %lf\n", obj[i].pln.color[0],
                   obj[i].pln.color[1],
                   obj[i].pln.color[2]);
            printf("position: %lf %lf %lf\n", obj[i].pln.position[0],
                   obj[i].pln.position[1],
                   obj[i].pln.position[2]);
            printf("normal: %lf %lf %lf\n", obj[i].pln.normal[0],
                   obj[i].pln.normal[1],
                   obj[i].pln.normal[2]);
        }
        else {
            printf("unsupported value\n");
        }
        i++;
    }
    printf("end at i=%d\n", i);
}
