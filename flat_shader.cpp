#include "flat_shader.h"

vec3 Flat_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{//given
    std::cerr << "Monte Carlo Path Tracing should not use shade_surface" << std::endl;
    return color;
}

vec3 Flat_Shader::Emission() const
{
    if (isEmissive)
    {
        return color; // basically a light!
    }
    else
    {
        return vec3(0.0, 0.0, 0.0); // Black, full absorber
    }
}

BSDF_Sample Flat_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const
{
    BSDF_Sample result; 
    // Emissive surfaces in this integrator are light sources only; stop path continuation here.
    // If the object isEmissive, then we treat it as a light source.
    // Basically, no need to continue if we hit a light
    if (isEmissive)
    {
        result.direction = vec3(0.0, 0.0, 0.0);
        result.brdf = vec3(0.0, 0.0, 0.0);
        result.pdf = 0.0;
        return result;
    }

    // flat shader is lambertian / ideal matte 
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    glm::float2 u(uniform(rng), uniform(rng)); // create 2 real numbers 

    // Float 4s are quaternions here
    glm::float4 quatRotationFromZ = brdf::getRotationFromZAxis(glm::float3(normal[0], normal[1], normal[2]));

    // local space sampling 
    // maps 2d [0,1] polar coords to 3d direction vec around +Z axis
    float pdf; 
    glm::float3 sampleLocal = brdf::sampleHemisphere(u, pdf);

    // world space
    // we then rotate the local +Z-based direction vec to world space
    glm::float3 sampleWorld = brdf::rotatePoint(quatRotationFromZ, sampleLocal);

    result.direction = vec3(sampleWorld.x, sampleWorld.y, sampleWorld.z);

    // lambertian / ideal matte -> brdf = albedo / pi 
    // brdf lib already handles cosine weighting in sampleHemisphere
    result.brdf = color * (1.0f / (float) pi);

    result.pdf = pdf; // equation for pdf = (cos(theta) / pi) from sample hemisphere

    return result; 
}
