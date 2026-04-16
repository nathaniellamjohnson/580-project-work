#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    std::cout << "Reflective Shader shade surface should not be used." << std::endl;

    vec3 base_color = shader->Shade_Surface(ray, intersection_point, normal, recursion_depth); 
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

    // Combine results
    return (base_color * (1.0 - this->reflectivity)) + ((this->reflectivity) * reflected_color);
}

// Monte Carlo Path Tracing Functions (Per shader basis)
vec3 Reflective_Shader::Emission() const 
{
    return vec3(0.0, 0.0, 0.0); // not emissive
}

BSDF_Sample Reflective_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const 
{
    BSDF_Sample result; 

    // For a perfect mirror, there is zero % of any reflection except in the 
    // perfect reflection direction. 
    vec3 ray_dir = -wo;
    vec3 reflection_dir = (ray_dir - (2.0 * dot(ray_dir, normal) * normal)).normalized();

    // From RenderWorld.cpp - vec3 Lo = Le + s.brdf * Li * (cosTheta / s.pdf);
    // From RenderWorld.cpp - float cosTheta = std::max(0.0, dot(normal_at_intersection_point, s.direction));
    // we need to make (cosTheta * s.brdf) / (s.pdf) == 1 so Li is the only contribution
    // s.brdf should = 1, 1, 1 (b/c it's a vector)
    // thus, cosTheta / s.pdf == 1 & thus cosTheta = s.pdf

    result.brdf = vec3(1.0, 1.0, 1.0); // what % of each light type to let through, since perfect mirror, let it all through
    result.direction = reflection_dir;
    result.pdf = std::max(0.0, dot(normal, result.direction));

    return result;
};