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
    std::cout << "Reflective Shader Emission() not finished." << std::endl;
    return vec3();
}

BSDF_Sample Reflective_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const 
{
    std::cout << "Reflective Shader Sample() not finished." << std::endl;
    BSDF_Sample result; 
    return result;
};