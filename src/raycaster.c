//
// Created by mkg on 10/7/2016.
//

#include "../include/raycaster.h"
#include "../include/vector_math.h"
#include "../include/json.h"
#include "../include/illumination.h"

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
    img->pixmap[row * img->width + col].r = (unsigned char)(MAX_COLOR_VAL * clamp(color[0]));
    img->pixmap[row * img->width + col].g = (unsigned char)(MAX_COLOR_VAL * clamp(color[1]));
    img->pixmap[row * img->width + col].b = (unsigned char)(MAX_COLOR_VAL * clamp(color[2]));
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
 * @param max_distance - This is the maximum distance we care to check. e.g. distance to a light source
 * @param ret_index - the index in objects array of the closest object we intersected
 * @param ret_best_t - the distance of the closest object
 */
void get_dist_and_idx_closest_obj(Ray *ray, int self_index, double max_distance, int *ret_index, double *ret_best_t) {
    int i;
    int best_o = -1;
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
        if (max_distance != INFINITY && t > max_distance)
            continue;
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

    // find new ray origin
    v3_scale(ray->direction, t, new_origin);
    v3_add(new_origin, ray->origin, new_origin);

    Ray ray_new = {
            .origin = {new_origin[0], new_origin[1], new_origin[2]},
            .direction = {new_dir[0], new_dir[1], new_dir[2]}
    };

    for (int i=0; i<nlights; i++) {

        // find new ray direction
        v3_sub(lights[i].position, ray_new.origin, ray_new.direction);
        double distance_to_light = v3_len(ray_new.direction);

        int best_o;     // index of closest object
        double best_t;  // distance of closest object

        // new check new ray for intersections with other objects
        get_dist_and_idx_closest_obj(&ray_new, obj_index, distance_to_light, &best_o, &best_t);

        double normal_vector[3];
        double obj_diff_color[3];
        if (best_o == -1) { // this means there was no object in the way between the current one and the light
            v3_zero(normal_vector); // zero out these vectors each time
            v3_zero(obj_diff_color);
            //double obj_spec_color[3];
            // find normal and color
            if (objects[obj_index].type == PLANE) {
                v3_copy(objects[obj_index].plane.normal, normal_vector);
                v3_copy(objects[obj_index].plane.diff_color, obj_diff_color);
                /*obj_spec_color[0] = objects[obj_index].plane.spec_color[0];
                obj_spec_color[1] = objects[obj_index].plane.spec_color[1];
                obj_spec_color[2] = objects[obj_index].plane.spec_color[2];*/
            } else if (objects[obj_index].type == SPHERE) {
                v3_sub(ray_new.origin, objects[obj_index].sphere.position, normal_vector);
                v3_copy(objects[obj_index].sphere.diff_color, obj_diff_color);
                /*obj_spec_color[0] = objects[obj_index].sphere.spec_color[0];
                obj_spec_color[1] = objects[obj_index].sphere.spec_color[1];
                obj_spec_color[2] = objects[obj_index].sphere.spec_color[2];*/
            } else {
                fprintf(stderr, "Error: shade: Trying to shade unsupported type of object\n");
                exit(1);
            }
            //printf("%lf %lf %lf\n", obj_spec_color[0], obj_spec_color[1], obj_spec_color[2]);
            // find light, reflection and camera vectors
            double light_vector[3] = {ray_new.direction[0], ray_new.direction[1], ray_new.direction[2]};
            double reflection_vector[3];
            v3_reflect(light_vector, normal_vector, reflection_vector);
            //double camera_vector[3] = {ray->direction[0], ray->direction[1], ray->direction[2]};
            double diffuse[3];
            //double specular[3];
            v3_zero(diffuse);

            calculate_diffuse(normal_vector, light_vector, lights[i].color, obj_diff_color, diffuse);
            // TODO: calculate frad(), fang(), and specular
            color[0] += diffuse[0];
            color[1] += diffuse[1];
            color[2] += diffuse[2];
        }
        // there was an object in the way, so we don't do anything. It's shadow
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
    //int i;  // x coord iterator
    //int j;  // y coord iterator
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

    for (int i = 0; i < img->height; i++) {
        for (int j = 0; j < img->width; j++) {
            point[0] = vp_pos[0] - cam_width/2.0 + pixwidth*(j + 0.5);
            point[1] = -(vp_pos[1] - cam_height/2.0 + pixheight*(i + 0.5));
            point[2] = vp_pos[2];    // set intersecting point Z to viewplane Z
            normalize(point);   // normalize the point
            // store normalized point as our ray direction
            v3_copy(point, ray.direction);

            int best_o = -1;     // index of 'best' or closest object
            double best_t = INFINITY;  // closest distance
            get_dist_and_idx_closest_obj(&ray, -1, INFINITY, &best_o, &best_t);

            // set ambient color
            //double *color = malloc(sizeof(double)*3);
            double color[3];
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;

            if (best_t > 0 && best_t != INFINITY && best_o != -1) {// there was an intersection
                //printf("#");    // ascii ray tracer "hit"
                //printf("type: %d\n", objects[best_i].type);
                shade(&ray, best_o, best_t, color);
                shade_pixel(color, i, j, img);
                //shade_pixel(objects[best_o].plane.diff_color, i, j, img);
            }
            else {
                //printf(".");    // ascii ray tracer "miss"
            }
        }
        //printf("\n");
    }
}