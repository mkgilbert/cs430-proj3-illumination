//
// Created by mkg on 10/13/2016.
//

#ifndef CS430_PROJ3_ILLUMINATION_ILLUMINATION_H
#define CS430_PROJ3_ILLUMINATION_ILLUMINATION_H
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
#endif //CS430_PROJ3_ILLUMINATION_ILLUMINATION_H
