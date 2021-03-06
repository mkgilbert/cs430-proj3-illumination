//
// Created by mkg on 10/13/2016.
//

#ifndef CS430_PROJ3_ILLUMINATION_ILLUMINATION_H
#define CS430_PROJ3_ILLUMINATION_ILLUMINATION_H

#include "json.h"

/* function declarations */
void calculate_diffuse(double *normal_vector,
                       double *light_vector,
                       double *light_color,
                       double *obj_color,
                       double *out_color);

void calculate_specular(double ns,
                        double *L,
                        double *R,
                        double *N,
                        double *V,
                        double *KS,
                        double *IL,
                        double *out_color);

double clamp(double color_val);

double calculate_angular_att(Light *light, double direction_to_object[3]);

double calculate_radial_att(Light *light, double distance_to_light);

#endif //CS430_PROJ3_ILLUMINATION_ILLUMINATION_H
