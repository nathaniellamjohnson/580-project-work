#ifndef __RENDER_WORLD_H__
#define __RENDER_WORLD_H__

#include <vector>
#include <random>

#if defined(__APPLE__)
#define PSTLD_HEADER_ONLY
#define PSTLD_HACK_INTO_STD
#include "pstld.h"
#else
#include <algorithm>
#include <execution>
#endif


#include <algorithm>
#include "camera.h"
#include "hierarchy.h"
#include "object.h"
#include "photon.h"
#include "caustic_map.h"

class Light;
class Shader;
class Ray;

class Render_World
{
public:
    Camera camera;

    Shader *background_shader;
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    vec3 ambient_color;
    double ambient_intensity;

    bool enable_shadows;
    int recursion_depth_limit;

    // Monte Carlo Path Tracing
    std::mt19937 rng; // RNG creator, default seed is 42
    int samples_per_pixel;

    // Caustic-only Photon Mapping
    bool enable_caustics;
    std::vector<Photon> caustic_photons;
    std::unique_ptr<Caustic_Photon_Map> caustic_map;

    int photons_per_light; 
    int max_photons_gathered;
    double gather_radius; 
    
    void Build_Caustic_Photon_Map(int photons_per_light=60000,int max_bounces=8);
    size_t Query_Caustic_Photons(const vec3& position,double radius,
        size_t max_results,std::vector<size_t>& out_indices) const;
    bool Has_Caustic_Photon_Map() const;
    vec3 Estimate_Caustic_Irradiance(const Render_World& world,const Shader* receiver_shader,const vec3& position,const vec3& normal);

    Hierarchy hierarchy;

    Render_World();
    ~Render_World();

    void Render_Pixel(const ivec2& pixel_index);
    void Render();
    void Initialize_Hierarchy();

    vec3 Cast_Ray(const Ray& ray,int recursion_depth);
    Hit Closest_Intersection(const Ray& ray);
};
#endif
