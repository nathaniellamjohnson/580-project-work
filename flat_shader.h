#ifndef __FLAT_SHADER_H__
#define __FLAT_SHADER_H__

#include "shader.h"
#include "brdf.h"

class Flat_Shader : public Shader
{
public:
    vec3 color;
    bool isEmissive; // Determines if emission returns color or not - true is a light of that color

    Flat_Shader(Render_World& world_input,const vec3& color=vec3(1,1,1), bool emissive=false)
        :Shader(world_input),color(color), isEmissive(emissive)
    {}

    // Monte Carlo Path Tracing Functions (Per shader basis)
    vec3 Emission() const override;
    BSDF_Sample Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const override;

    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;
};
#endif
