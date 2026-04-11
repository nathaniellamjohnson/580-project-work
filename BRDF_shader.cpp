#include <BRDF_shader.h>

#include <algorithm>
#include "light.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 BRDF_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color = vec3(0,0,0);

    //incoming light direction vector
    vec3 light_in_dir = ray.direction.normalized();
    
    //Changing math for how light is calculated to use BRDF Lambert style 
    //Might be adding 3 different functions?
    auto lights_array = this->world.lights;
    for (Light* light : lights_array){
        //Outgoing/reflected light vector
        vec3 light_out_dir = (light->position - intersection_point).normalized();
        double light_in_out_dot = dot(light_in_dir, light_out_dir);

        if(light_in_out_dot > 0){
            //BRDF math--------(Will grab later)
            //Albedo is diffuse within the book. Use FLam to do color
            vec3 fLam = albedo / pi;

            vec3 to_light_vec = (light->position - intersection_point);
            color += fLam * light->Emitted_Light(to_light_vec) * light_in_out_dot;
        }
    }

    return color;
}