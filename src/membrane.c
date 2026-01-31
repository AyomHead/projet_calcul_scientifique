#include "membrane.h"
#include <stdlib.h>
#include <math.h>

double default_tension(double x, double y) {
    return 1.0 + 0.5 * sin(2 * PI * x) * cos(2 * PI * y);
}

double default_density(double x, double y) {
    return 1.0 + 0.3 * x * y;
}

double default_potential(double x, double y) {
    double x0 = 0.5, y0 = 0.5;
    double r2 = (x - x0) * (x - x0) + (y - y0) * (y - y0);
    return 50.0 * exp(-50.0 * r2);
}

MembraneParams* create_default_params() {
    MembraneParams* params = (MembraneParams*)malloc(sizeof(MembraneParams));
    if (!params) return NULL;
    
    params->tension = default_tension;
    params->density = default_density;
    params->potential = default_potential;
    params->obstacle_center_x = 0.5;
    params->obstacle_center_y = 0.5;
    params->obstacle_strength = 50.0;
    params->obstacle_width = 50.0;
    
    return params;
}

void free_membrane_params(MembraneParams* params) {
    if (params) free(params);
}
