#ifndef __GLASS_SHADER_H__
#define __GLASS_SHADER_H__

#include "shader.h"
#include "brdf.h" // The header you shared

class Glass_Shader : public Shader
{
public:
    float ior;            // Index of Refraction (e.g., 1.5 for glass)
    float roughness;      // 0.0 for perfectly smooth glass
    vec3 color;           // Tint of the glass

    Glass_Shader(Render_World& world_input, float ior_input = 1.5f, 
                 float roughness_input = 0.0f, vec3 color_input = {1,1,1})
        : Shader(world_input), ior(ior_input), 
          roughness(roughness_input), color(color_input)
    {}

    virtual vec3 Shade_Surface(const Ray& ray, const vec3& intersection_point,
        const vec3& normal, int recursion_depth) const override;
};

#endif