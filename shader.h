#ifndef __SHADER_H__
#define __SHADER_H__

#include <random>
#include "vec.h"
#include "BSDF_Sample.h"
class Render_World;
class Ray;

extern bool debug_pixel;

class Shader
{
public:
    Render_World& world;

    Shader(Render_World& world_input)
        :world(world_input)
    {}

    virtual ~Shader()
    {}

    // Deterministic rendering, deprecated
    virtual vec3 Shade_Surface(const Ray& ray,const vec3& intersection_point,
        const vec3& normal,int recursion_depth) const=0;

    // Monte Carlo Path Tracing Functions
    virtual vec3 Emission() const = 0;
    virtual BSDF_Sample Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const = 0;
};
#endif
