#ifndef __BRDF_SHADER_H__
#define __BRDF_SHADER_H__

#include "shader.h"
#include "brdf.h"

class BRDF_Shader: public Shader{
public:

    // Attempting Disney (Burley) Diffuse Model 
    
    vec3 color_diffuse;
    float roughness;

    BRDF_Shader(Render_World& world_input,
            const vec3& albedo,
            double roughness)
            :Shader(world_input),
            color_diffuse(albedo),
            roughness(roughness)
            {}

    //Not useful anymore for Monte Carlo
    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;
    

    //Helper function
    glm::float3 Evaluate_Difuse( const glm::float3& wiLocal,
    const glm::float3& woLocal,
    const glm::float3& baseColor,
    float roughness) const;

    //Monte Carlo shading methods
    vec3 Emission() const override;
    BSDF_Sample Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const override;
};
#endif

//Notes for later
//https://www.youtube.com/watch?v=R9iZzaXUaK4
//https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function
//https://boksajak.github.io/files/CrashCourseBRDF.pdf (pg5 3.1)