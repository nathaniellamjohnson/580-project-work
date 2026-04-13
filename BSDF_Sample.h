#ifndef __BSDF_SAMPLE_H__
#define __BSDF_SAMPLE_H__

#include "vec.h"

// One sample represents one d_wi of the rendering equation reflecting branch
// Many Samples added together & averaged represents the approximation of the fully integrated reflection branch of the rendering equation
struct BSDF_Sample {
    vec3 direction;  // wi -> direction incoming 
    vec3 brdf;       // f_r -> BRDF / reflection light/color value
    double pdf;      // probability density -> how likely is this sample?
};

#endif