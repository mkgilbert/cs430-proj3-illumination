//
// Created by mkg on 10/13/2016.
//

#include "../include/illumination.h"
#include "../include/vector_math.h"

void calculate_diffuse(double *normal_vector,
                       double *light_vector,
                       double *light_color,
                       double *obj_color,
                       double *out_color) {
    // K_a*I_a should be added to the beginning of this whole thing, which is a constant and ambient light
    double n_dot_l = v3_dot(normal_vector, light_vector);
    if (n_dot_l > 0) {
        double diffuse_product[3];
        diffuse_product[0] = obj_color[0] * light_color[0];
        diffuse_product[1] = obj_color[1] * light_color[1];
        diffuse_product[2] = obj_color[2] * light_color[2];
        // multiply by n_dot_l and store in out_color
        v3_scale(diffuse_product, n_dot_l, out_color);
    }
    else {
        // would normally return K_a*I_a here...
        out_color[0] = 0;
        out_color[1] = 0;
        out_color[2] = 0;
    }
}
