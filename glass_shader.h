#ifndef __GLASS_SHADER_H__
#define __GLASS_SHADER_H__

#include <algorithm>
#include "shader.h"
#include "brdf.h"

class Glass_Shader : public Shader
{
public:
    vec3 color;
    double ior;
    
    Glass_Shader(Render_World& world_input,vec3 color_input,double ior_in)
        :Shader(world_input),color(color_input),
        ior(ior_in)
    {}

     virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
         const vec3& normal,int recursion_depth) const override;
    vec3 Emission() const override;
    BSDF_Sample Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const override;
};
#endif
