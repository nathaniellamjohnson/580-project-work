#include <algorithm>
#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;
    
    // Lights array from world object linked in shader
    // Relevant vars from light are the vec3 position & their Emitted_Light function
    auto lights_array = this->world.lights;

    // Psuedocode:
    // grant ambient light to the object -> every object gets this regardless of lighting 
    color += this->color_ambient * this->world.ambient_intensity * this->world.ambient_color; // ambient
    
    // for each light in lights array
    for (Light* light : lights_array)
    {
        // Shadow ray logic
        // If the shadows enabled in .txt file
        if (this->world.enable_shadows)
        {
            // cast a shadow ray starting from the intersection point towards the light position
            vec3 shadow_ray_dir = (light->position - intersection_point).normalized();
            double shadow_ray_distance = (light->position - intersection_point).magnitude();
            Ray shadow_ray = Ray(intersection_point, shadow_ray_dir);
            Hit shadow_ray_hit = this->world.Closest_Intersection(shadow_ray);

            // If we find something AND that something is not the intersection we know about, then no non-ambient light gets through.
            // Continue on to rest of the lights
            if (shadow_ray_hit.object != nullptr && (shadow_ray_distance - shadow_ray_hit.dist) > small_t) 
            {
                continue; // something found between light and intersection point
            }
        }

        vec3 to_light_vec = (light->position - intersection_point); // unnormalized version, points directly to light?
        vec3 to_light = (light->position - intersection_point).normalized();

        // Diffuse light logic
        vec3 object_diffuse = this->color_diffuse;
        double diffuse_intensity = std::min(std::max(0.0,dot(to_light, normal)), 1.0); // dot(light, normal)
        color += object_diffuse * diffuse_intensity * light->Emitted_Light(to_light_vec); // Problem here -> need to multiply by the inverse square law & the emitted light also tells you the light color!

        // Specular light logic
        vec3 object_specular = this->color_specular;
        vec3 reflected = 2.0 * (normal * (std::min(std::max(0.0,dot(to_light, normal)), 1.0))) - to_light; // 2 * (normal(dot(to_light, normal))) - to_light
        double specular_intensity = pow(std::min(std::max(0.0,dot(reflected, -1.0 * ray.direction)), 1.0), specular_power);
        color += object_specular * specular_intensity * light->Emitted_Light(to_light_vec);         
    }

    return color;
}
