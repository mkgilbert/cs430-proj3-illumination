//
// Created by mkg on 10/7/2016.
//

#include "../include/raycaster.h"
#include "../include/vector_math.h"
#include "../include/json.h"

/* raycast.c - provides raycasting functionality */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * Finds and gets the index in objects that has the camera width and height
 * @param objects - array of object types that represent the scene
 * @return int - non-negative if the object was found, -1 otherwise
 */
int get_camera(object *objects) {
    int i = 0;
    while (i < MAX_OBJECTS && objects[i].type != 0) {
        if (objects[i].type == CAMERA) {
            return i;
        }
        i++;
    }
    // no camera found in data
    return -1;
}

/**
 * colors the values of a pixel based on the color array that is passed in
 * @param color - array of 3 color values for r,g,b
 * @param row - which row the pixel is on
 * @param col - which column the pixel is on
 * @param img - image struct that allows for indexing the appropriate spot
 */
void shade_pixel(double *color, int row, int col, image *img) {
    // fill in pixel color values
    // the color vals are stored as values between 0 and 1, so we need to adjust
    img->pixmap[row * img->width + col].r = color[0];
    img->pixmap[row * img->width + col].g = color[1];
    img->pixmap[row * img->width + col].b = color[2];
}

/** Tests for an intersection between a ray and a plane
 * @param Ro - 3d vector of ray origin
 * @param Rd - 3d vector of ray direction
 * @param Pos - 3d vector of the plane's position
 * @param Norm - 3d vector of the normal to the plane
 * @return - distance to the object if intersects, otherwise, -1
 */
double plane_intersect(Ray *ray, double *Pos, double *Norm) {
    normalize(Norm);
    // determine if plane is parallel to the ray
    double vd = v3_dot(Norm, ray->direction);

    if (fabs(vd) < 0.0001) return -1;

    double vector[3];
    v3_sub(Pos, ray->origin, vector);
    double t = v3_dot(vector, Norm) / vd;

    // no intersection
    if (t < 0.0)
        return -1;

    return t;
}

/**
 * Tests for an intersection between a ray and a sphere
 * @param Ro - 3d vector of ray origin
 * @param Rd - 3d vector of ray direction
 * @param C - 3d vector of the center of the sphere
 * @param r - radius of the sphere
 * @return - distance to the object if intersects, otherwise, -1
 */
double sphere_intersect(Ray *ray, double *C, double r) {
    double b, c;
    double vector_diff[3];
    v3_sub(ray->direction, C, vector_diff);

    // calculate quadratic formula
    b = 2 * (ray->direction[0]*vector_diff[0] + ray->direction[1]*vector_diff[1] + ray->direction[2]*vector_diff[2]);
    c = sqr(vector_diff[0]) + sqr(vector_diff[1]) + sqr(vector_diff[2]) - sqr(r);

    // check that discriminant is <, =, or > 0
    double disc = sqr(b) - 4*c;
    double t;  // solutions
    if (disc < 0) {
        return -1; // no solution
    }
    disc = sqrt(disc);
    t = (-b - disc) / 2.0;
    if (t < 0.0)
        t = (-b + disc) / 2.0;

    if (t < 0.0)
        return -1;
    return t;
}

/**
 *
 * @param ray - the ray we are shooting out to find an intersection with
 * @param self_index - if < 0, ignore this. If >= 0, it is the index of the object we are getting distance FROM
 * @param ret_index - the index in objects array of the closest object we intersected
 * @param ret_best_t - the distance of the closest object
 */
void get_dist_and_idx_closest_obj(Ray *ray, int self_index, int *ret_index, double *ret_best_t) {
    //TODO: possibly put another parameter for max_t...for "distance_to_light"...we wouldn't want to look at anything farther away than the light
    int i;
    int best_o = 0;
    double best_t = INFINITY;
    for (i=0; objects[i].type != 0; i++) {
        // if self_index was passed in as > 0, we must ignore object i because we are checking distance to another
        // object from the one at self_index.
        if (self_index == i) continue;

        // we need to run intersection test on each object
        double t = 0;
        switch(objects[i].type) {
            case 0:
                printf("no object found\n");
                break;
            case CAMERA:
                break;
            case SPHERE:
                t = sphere_intersect(ray, objects[i].sphere.position,
                                     objects[i].sphere.radius);
                break;
            case PLANE:
                t = plane_intersect(ray, objects[i].plane.position,
                                    objects[i].plane.normal);
                break;
            default:
                // Error
                exit(1);
        }
        if (t > 0 && t < best_t) {
            best_t = t;
            best_o = i;
        }
    }
    (*ret_index) = best_o;
    (*ret_best_t) = best_t;
}

/**
 *
 * @param ray - original ray -- starting point for testing shade
 * @param obj_index  - index of the current object we are running shade on
 * @param t - distance to the object
 * @param color - this will be the output color after shade calculations are done
 */
void shade(Ray *ray, int obj_index, double t, double color[3]) {
    // loop through lights and do shadow test
    double new_origin[3];
    double new_dir[3];

    for (int i=0; i<nlights; i++) {
        // find new ray origin
        v3_scale(ray->direction, t, new_origin);
        v3_add(new_origin, ray->origin, new_origin);
        // find new ray direction
        v3_sub(lights[i].position, new_origin, new_dir);

        Ray ray_new = {
                .origin = {new_origin[0], new_origin[1], new_origin[2]},
                .direction = {new_dir[0], new_dir[1], new_dir[2]}
        };

        // TODO: closest_shadow_object = ...????
        int best_o;     // index of closest object
        double best_t;  // distance of closest object

        // new check new ray for intersections with other objects
        // TODO: maybe add param in this function call for max_t as distance_to_light?
        get_dist_and_idx_closest_obj(&ray_new, obj_index, &best_o, &best_t);

        //TODO: if (closest_shadow_object == NULL) ?????
        /*if (objects[best_o].type == PLANE) {
            double normal_vector[3] = {
                    objects[best_o].plane.normal[0],
                    objects[best_o].plane.normal[1],
                    objects[best_o].plane.normal[2]
            };
        }
        else if (objects[best_o].type == SPHERE) {
            double normal_vector[3];
            // find normal
            v3_sub(ray_new.origin, objects[best_o].sphere.position, normal_vector);
        }
        else {
            fprintf(stderr, "Error: shade: Trying to shade unsupported type of object\n");
            exit(1);
        }*/
        //double light_vector[3] = {ray_new.origin[0], ray_new.origin[1], ray_new.origin[2]};
        // TODO: implement reflection function
        //double reflection_vector[3];
        //double camera_vector[3] = {ray->direction[0], ray->direction[1], ray->direction[2]};

    }
}

/**
 * Shoots out rays over a viewplane of dimensions stored in img and looks through
 * the array of objects for an intersection for each pixel.
 * @param img - image data (width, height, pixmap...)
 * @param cam_width - camera width
 * @param cam_height - camera height
 * @param objects - array of objects in the scene
 */
void raycast_scene(image *img, double cam_width, double cam_height, object *objects) {
    // loop over all pixels and test for intesections with objects.
    // store results in pixmap
    int i;  // x coord iterator
    int j;  // y coord iterator
    double vp_pos[3] = {0, 0, 1};   // view plane position
    //double Ro[3] = {0, 0, 0};       // camera position (ray origin)
    double point[3] = {0, 0, 0};    // point on viewplane where intersection happens

    double pixheight = (double)cam_height / (double)img->height;
    double pixwidth = (double)cam_width / (double)img->width;
    //double Rd[3] = {0, 0, 0};       // direction of Ray

    Ray ray = {
            .origin = {0, 0, 0},
            .direction = {0, 0, 0}
    };

    for (i = 0; i < img->height; i++) {
        for (j = 0; j < img->width; j++) {
            point[0] = vp_pos[0] - cam_width/2.0 + pixwidth*(j + 0.5);
            point[1] = -(vp_pos[1] - cam_height/2.0 + pixheight*(i + 0.5));
            point[2] = vp_pos[2];    // set intersecting point Z to viewplane Z
            normalize(point);   // normalize the point
            // store normalized point as our ray direction
            ray.direction[0] = point[0];
            ray.direction[1] = point[1];
            ray.direction[2] = point[2];

            int best_o;     // index of 'best' or closest object
            double best_t;  // closest distance
            get_dist_and_idx_closest_obj(&ray, -1, &best_o, &best_t);

            // set ambient color
            double *color = malloc(sizeof(double)*3);
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;

            if (best_t > 0 && best_t != INFINITY) {// there was an intersection
                //printf("#");    // ascii ray tracer "hit"
                //printf("type: %d\n", objects[best_i].type);
                shade(&ray, best_o, best_t, color);
                if (objects[best_o].type == PLANE) {
                    // TODO: change these calls to account for diffuse and specular
                    shade_pixel(objects[best_o].plane.spec_color, i, j, img);
                }
                else if (objects[best_o].type == SPHERE) {
                    //printf("shade\n");
                    shade_pixel(objects[best_o].sphere.spec_color, i, j, img);
                }
            }
            else {
                //printf(".");    // ascii ray tracer "miss"
            }
        }
        //printf("\n");
    }
}