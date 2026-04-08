#ifndef __RENDER_WORLD_H__
#define __RENDER_WORLD_H__

#include <memory>
#include <vector>
#include "camera.h"
#include "hierarchy.h"
#include "object.h"

class Light;
class Shader;
class Ray;

class Render_World
{
public:
    struct Photon
    {
        vec3 position;
        vec3 direction;
        vec3 power;
    };

public:
    Camera camera;

    Shader *background_shader;
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    vec3 ambient_color;
    double ambient_intensity;

    bool enable_shadows;
    int recursion_depth_limit;

    std::vector<Photon> caustic_photons;

    Hierarchy hierarchy;

    Render_World();
    ~Render_World();

    void Render_Pixel(const ivec2& pixel_index);
    void Render();
    void Initialize_Hierarchy();

    // Builds a caustic-only photon map (photons that hit diffuse surfaces
    // after at least one specular event).
    void Build_Caustic_Photon_Map(int photons_per_light=6000,int max_bounces=8);

    // Returns indices of photons within radius (up to max_results).
    size_t Query_Caustic_Photons(const vec3& position,double radius,
        size_t max_results,std::vector<size_t>& out_indices) const;

    bool Has_Caustic_Photon_Map() const;

    vec3 Cast_Ray(const Ray& ray,int recursion_depth);
    Hit Closest_Intersection(const Ray& ray);

private:
    class Caustic_Photon_Map_Impl;
    std::unique_ptr<Caustic_Photon_Map_Impl> caustic_map_impl;
};
#endif
