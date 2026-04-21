#include "caustic_map.h"

vec3 Random_Unit_Vector(std::mt19937& rng)
{
    // slides from 580 recommend for 3d spheres and up to just 
    // generate and normalize after
    std::normal_distribution<double> dist(0.0, 1.0);
    vec3 v(dist(rng), dist(rng), dist(rng));
    return v.normalized();
}

bool Is_Caustic_Specular(const Shader* shader)
{
    if(dynamic_cast<const Glass_Shader*>(shader)!=nullptr)
    {
        return true;
    }
    const Reflective_Shader* reflective = dynamic_cast<const Reflective_Shader*>(shader);
    return reflective!=nullptr && reflective->reflectivity>0;
}


