#ifndef __REFLECTIVE_SHADER_H__
#define __REFLECTIVE_SHADER_H__

#include <algorithm>
#include "shader.h"

class Reflective_Shader : public Shader
{
public:
    Shader* shader;
    double reflectivity;
    
    Reflective_Shader(Render_World& world_input,Shader* shader_input,double reflectivity)
        :Shader(world_input),shader(shader_input),
        reflectivity(std::max(0.0,std::min(1.0,reflectivity)))
    {}

     virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
         const vec3& normal,int recursion_depth) const override;


    // Monte Carlo Path Tracing Functions (Per shader basis)
    vec3 Emission() const override;
    BSDF_Sample Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const override;
};
#endif
