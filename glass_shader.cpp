#include "glass_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Glass_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 reflected_color = {0.0, 0.0, 0.0};

    // Base Cases
    // ____________________________________
    // Ray Miss -> handled in cast_ray, gets background shader
    // Ray Bounce Limit/Recursion Depth

    // Ray hit a light source -> not implemented?
    // Ray hit a non-reflective surface... -> "implemented" below based on reflectivity constants

    // ===========================================

    // Recursive Case -> if recursion depth > 0, then we can shoot off another ray
    // If it's <= to 0, then just use regular color
    if (recursion_depth > 0)
    {
        // Get reflected ray
        vec3 reflected_dir = (ray.direction - (2.0 * dot(ray.direction, normal) * normal)).normalized();
        vec3 reflected_start_point = (intersection_point) + (reflected_dir * small_t);
        Ray reflection_ray = Ray(reflected_start_point, reflected_dir);

        // Cast new ray from there
        reflected_color = this->world.Cast_Ray(reflection_ray, recursion_depth - 1);
    }
    vec3 refract_color = {0.0, 0.0, 0.0};
    float reflectivity = 0.05;
    float tint = 0.05;

    if (recursion_depth > 0)
    {
        // Get refracted ray
        float n1 = 1.0;
        float n2 = ior;
        vec3 norm = normal;
        if(dot(ray.direction, normal) > 0){
            n1 = ior;
            n2 = 1.0;
            norm = -norm;
        }
        float iorRatio = n1/n2;
        float temp = 1 - pow(iorRatio,2) * pow(dot(-norm,ray.direction),2);
        if (temp < 0){
            reflectivity = 1;
            tint = 0;
        }
        else{
            vec3 refract_dir = ((ray.direction * iorRatio) + norm*(iorRatio*dot(-norm,ray.direction) 
                                - sqrt(1 - pow(iorRatio,2) * pow(dot(-norm,ray.direction),2)))).normalized();
            vec3 refract_start_point = (intersection_point) + (refract_dir * small_t);
            Ray refract_ray = Ray(refract_start_point, refract_dir);
             // Cast new ray from there
            refract_color = this->world.Cast_Ray(refract_ray, recursion_depth - 1);
        }

       
        
    }

    // Combine results
    return (this->color * tint) + ( reflected_color * reflectivity) + ((1.0 - tint - reflectivity) * refract_color);
}

vec3 Glass_Shader::Emission() const 
{
    return vec3(0.0, 0.0, 0.0); // not emissive
}

BSDF_Sample Glass_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const 
{
    BSDF_Sample result; 

    // For a perfect mirror, there is zero % of any reflection except in the 
    // perfect reflection direction. 
    
    vec3 ray_dir = -wo;
    vec3 norm = normal;
    float n1 = 1.0;
    float n2 = ior;
    if(dot(ray_dir, normal) > 0){
        n1 = ior;
        n2 = 1.0;
        norm = -norm;
    }
    vec3 reflection_dir = (ray_dir - (2.0 * dot(ray_dir, norm) * norm)).normalized();

    // From RenderWorld.cpp - vec3 Lo = Le + s.brdf * Li * (cosTheta / s.pdf);
    // From RenderWorld.cpp - float cosTheta = std::max(0.0, dot(normal_at_intersection_point, s.direction));
    // we need to make (cosTheta * s.brdf) / (s.pdf) == 1 so Li is the only contribution
    // s.brdf should = 1, 1, 1 (b/c it's a vector)
    // thus, cosTheta / s.pdf == 1 & thus cosTheta = s.pdf
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    // Generate a random float
    float random_val = uniform(rng);
    float reflectivity = 0.05;
    float tint = 0.05;

    float iorRatio = n1/n2;
    float temp = 1 - pow(iorRatio,2) * pow(dot(-norm,ray_dir),2);
    
    if (random_val > 1-tint){
        // copied from flat_shader.cpp
        glm::float2 u(uniform(rng), uniform(rng)); // create 2 real numbers 

        // Float 4s are quaternions here
        glm::float4 quatRotationFromZ = brdf::getRotationFromZAxis(glm::float3(norm[0], norm[1], norm[2]));

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
    }
    else if((random_val < reflectivity) || temp < 0){
        result.brdf = vec3(1.0, 1.0, 1.0); // what % of each light type to let through, since perfect mirror, let it all through
        result.direction = reflection_dir;
        result.pdf = std::max(0.0, dot(norm, result.direction));
    }
    else{
        vec3 refract_dir = ((ray_dir * iorRatio) + norm*(iorRatio*dot(-norm,ray_dir) - sqrt(temp))).normalized();
        result.brdf = vec3(1.0, 1.0, 1.0); // what % of each light type to let through, since perfect mirror, let it all through
        result.direction = refract_dir;
        result.pdf = abs(dot(-norm, result.direction));
    }

    return result;
};