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
