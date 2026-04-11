#ifndef __BRDF_SHADER_H__
#define __BRDF_SHADER_H__

#include <algorithm>
#include <shader.h>

//Remembering for later probs not gonna use
enum Material{
        Lambert,
        Reflective,
        Phong,
    };

class BRDF_Shader: public Shader{

    //Gonna make a shader 
    //Note to self: We need the math functions but Ima just set up the shader first so that I can figure out if Im'm doing this right
    //Attemping Lambertian 
    vec3 albedo;


    BRDF_Shader(Render_World& world_input,
        const vec3 albedo)
        :Shader(world_input),
        albedo(albedo)
    {}


    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const override;


};
#endif

//Notes for later
//https://www.youtube.com/watch?v=R9iZzaXUaK4
//https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function
//https://boksajak.github.io/files/CrashCourseBRDF.pdf (pg5 3.1)