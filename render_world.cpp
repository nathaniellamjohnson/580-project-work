#include <algorithm>
#include <cmath>
#include <random>
#include <utility>
#include "nanoflann.hpp" // part of nanoflann
#include "render_world.h"
#include "flat_shader.h"
#include "glass_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"
#include "reflective_shader.h"

//#include <iostream>
//using namespace std;

extern bool disable_hierarchy;

class Render_World::Caustic_Photon_Map_Impl
{
public:
    struct Photon_Point_Cloud
    {
        const std::vector<Render_World::Photon>* photons;

        Photon_Point_Cloud()
            :photons(0)
        {}

        inline size_t kdtree_get_point_count() const
        {
            return photons ? photons->size() : 0;
        }

        inline double kdtree_get_pt(const size_t idx, const size_t dim) const
        {
            return (*photons)[idx].position[(int)dim];
        }

        template <class BBOX>
        bool kdtree_get_bbox(BBOX&) const
        {
            return false;
        }
    };

    typedef nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<double, Photon_Point_Cloud>,
        Photon_Point_Cloud,
        3,
        size_t> KD_Tree;

    Photon_Point_Cloud point_cloud;
    std::unique_ptr<KD_Tree> kdtree;
};

namespace
{
Hit Find_Closest_Hit(const std::vector<Object*>& objects,const Ray& ray)
{
    Hit best_hit = {nullptr,0,-1};
    for(Object* object : objects)
    {
        Hit hit = object->Intersection(ray,-1);
        if(hit.object!=nullptr && hit.dist>=small_t &&
            (best_hit.object==nullptr || hit.dist<best_hit.dist))
            best_hit = hit;
    }
    return best_hit;
}

vec3 Reflect_Direction(const vec3& incoming,const vec3& normal)
{
    return (incoming - 2.0*dot(incoming,normal)*normal).normalized();
}

vec3 Random_Unit_Vector(std::mt19937& rng)
{
    std::uniform_real_distribution<double> dist(0.0,1.0);
    double z = 1.0 - 2.0*dist(rng);
    double phi = 2.0*pi*dist(rng);
    double r = std::sqrt(std::max(0.0,1.0-z*z));
    return vec3(r*std::cos(phi),r*std::sin(phi),z);
}

bool Is_Caustic_Specular(const Shader* shader)
{
    if(dynamic_cast<const Glass_Shader*>(shader)!=nullptr) return true;
    const Reflective_Shader* reflective = dynamic_cast<const Reflective_Shader*>(shader);
    return reflective!=nullptr && reflective->reflectivity>0;
}

vec3 Estimate_Caustic_Irradiance(const Render_World& world,
    const Shader* receiver_shader,const vec3& position,const vec3& normal)
{
    if(Is_Caustic_Specular(receiver_shader)) return vec3();
    if(!world.Has_Caustic_Photon_Map()) return vec3();

    // Initial gather parameters for caustics-only density estimate.
    const double gather_radius = 0.18;
    const size_t max_photons = 200;

    std::vector<size_t> photon_indices;
    const size_t found = world.Query_Caustic_Photons(position,gather_radius,
        max_photons,photon_indices);
    if(found==0) return vec3();

    vec3 flux;
    vec3 n = normal.normalized();
    for(size_t idx : photon_indices)
    {
        const Render_World::Photon& photon = world.caustic_photons[idx];
        double weight = std::max(0.0,dot(n,photon.direction));
        flux += photon.power*weight;
    }

    // Photon density estimate over disk area.
    const double area = pi*gather_radius*gather_radius;
    const double caustic_intensity_scale = 0.85;
    return flux*(caustic_intensity_scale/area);
}
}

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{
    caustic_map_impl.reset(new Caustic_Photon_Map_Impl());
    caustic_map_impl->point_cloud.photons = &caustic_photons;
}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    Hit closest_hit;
    closest_hit = {nullptr, 0, 0};
    double min_t = std::numeric_limits<double>::max();

    // The dumb way to do it is to scan thru everything and then check for the closest intersection
    for (Object* obj : objects)
    {
        Hit obj_intersection_hit = obj->Intersection(ray , -1); // potential source of error, check against all parts 
        double dist = obj_intersection_hit.dist;

        if (dist >= small_t && dist <= min_t)
        {
            closest_hit = obj_intersection_hit;
        }
    }
    return closest_hit;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    // Ray class has vec3 endpoint & vec3 direction
    Ray ray;
    
    // Get ray endpoint via pixel world position
    // get direction via normalize(pix_pos - cam_pos)
    ray.endpoint = camera.World_Position(pixel_index);
    ray.direction = (ray.endpoint - camera.position).normalized();

    vec3 color=Cast_Ray(ray,this->recursion_depth_limit - 1);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy(); //ignore this untill the last 2 test cases

    Build_Caustic_Photon_Map();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

void Render_World::Build_Caustic_Photon_Map(int photons_per_light,int max_bounces)
{
    caustic_photons.clear();
    caustic_map_impl->kdtree.reset();

    if(photons_per_light<=0 || max_bounces<=0 || lights.empty()) return;

    std::mt19937 rng(1337u);
    std::uniform_real_distribution<double> dist(0.0,1.0);

    caustic_photons.reserve((size_t)photons_per_light*lights.size()/4 + 1);

    for(Light* light : lights)
    {
        vec3 photon_power = (light->color*light->brightness)/(double)photons_per_light;

        for(int i=0;i<photons_per_light;i++)
        {
            Ray photon_ray(light->position,Random_Unit_Vector(rng));
            vec3 throughput = photon_power;
            bool has_specular_path = false;

            for(int bounce=0;bounce<max_bounces;bounce++)
            {
                Hit hit = Find_Closest_Hit(objects,photon_ray);
                if(hit.object==nullptr) break;

                vec3 hit_point = photon_ray.Point(hit.dist);
                vec3 n = hit.object->Normal(hit_point,hit.part).normalized();
                Shader* shader = hit.object->material_shader;
                if(shader==nullptr) break;

                if(!Is_Caustic_Specular(shader))
                {
                    if(has_specular_path)
                    {
                        Photon p;
                        p.position = hit_point;
                        p.direction = -1.0*photon_ray.direction;
                        p.power = throughput;
                        caustic_photons.push_back(p);
                    }
                    break;
                }

                has_specular_path = true;

                if(const Reflective_Shader* reflective = dynamic_cast<const Reflective_Shader*>(shader))
                {
                    vec3 reflected = Reflect_Direction(photon_ray.direction,n);
                    photon_ray = Ray(hit_point + reflected*small_t,reflected);
                    throughput *= reflective->reflectivity;
                }
                else if(const Glass_Shader* glass = dynamic_cast<const Glass_Shader*>(shader))
                {
                    vec3 N = n;
                    vec3 V = (-1.0*photon_ray.direction).normalized();
                    double eta_i = 1.0;
                    double eta_t = std::max(1.0001f,glass->ior);
                    double cos_theta = dot(N,V);
                    if(cos_theta<0)
                    {
                        N = -1.0*N;
                        cos_theta = -cos_theta;
                        std::swap(eta_i,eta_t);
                    }

                    double r0 = (eta_i-eta_t)/(eta_i+eta_t);
                    r0 *= r0;
                    double fresnel = r0 + (1.0-r0)*std::pow(std::max(0.0,1.0-cos_theta),5.0);
                    double eta = eta_i/eta_t;
                    double k = 1.0 - eta*eta*(1.0-cos_theta*cos_theta);

                    vec3 next_dir;
                    if(k<=0 || dist(rng)<fresnel)
                    {
                        next_dir = Reflect_Direction(photon_ray.direction,N);
                        throughput *= fresnel;
                    }
                    else
                    {
                        next_dir = (eta*(-1.0*V) + (eta*cos_theta-std::sqrt(k))*N).normalized();
                        throughput *= (1.0-fresnel);
                        throughput *= glass->color;
                    }

                    photon_ray = Ray(hit_point + next_dir*small_t,next_dir);
                }

                if(throughput.magnitude_squared()<1e-8) break;
            }
        }
    }

    if(caustic_photons.empty()) return;

    caustic_map_impl->kdtree.reset(new Caustic_Photon_Map_Impl::KD_Tree(3,caustic_map_impl->point_cloud,
        nanoflann::KDTreeSingleIndexAdaptorParams(10)));
    caustic_map_impl->kdtree->buildIndex();
}

size_t Render_World::Query_Caustic_Photons(const vec3& position,double radius,
    size_t max_results,std::vector<size_t>& out_indices) const
{
    out_indices.clear();
    if(!caustic_map_impl->kdtree || radius<=0 || max_results==0) return 0;

    std::vector<nanoflann::ResultItem<size_t,double> > matches;
    matches.reserve(max_results);

    double query_pt[3] = {position[0],position[1],position[2]};
    nanoflann::SearchParameters params;
    const size_t match_count = caustic_map_impl->kdtree->radiusSearch(query_pt,
        radius*radius,matches,params);

    size_t count = std::min(max_results,match_count);
    out_indices.reserve(count);
    for(size_t i=0;i<count;i++) out_indices.push_back(matches[i].first);
    return out_indices.size();
}

bool Render_World::Has_Caustic_Photon_Map() const
{
    return caustic_map_impl->kdtree.get()!=0 && !caustic_photons.empty();
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;

    // Hit object of the min dist hit object
    Hit minimum_dist_hit = {};
    minimum_dist_hit.object = nullptr;

    // World has object arrays
    for (Object* o : objects)
    {
        // Check for hit & store minimum dist instance 
        Hit hit_info = o->Intersection(ray, -1); // potential source of error -> negative part in order to check against everything
        if (hit_info.object != NULL && ( minimum_dist_hit.object == nullptr || hit_info.dist < minimum_dist_hit.dist ))
        {
            minimum_dist_hit = hit_info;
        }
    }
    
    // Check for hit at all 
    if (minimum_dist_hit.object == nullptr)
    {
        // No intersections, background color 
        color = background_shader->Shade_Surface(ray, vec3(0, 0, 0), vec3(0, 0, 0), recursion_depth); // potential source of error, what intersection point / normal to use?
    }
    else 
    {
        // Get color of that object via that object's shader
        // fill & return that color
        vec3 intersection_point = ray.endpoint + ray.direction * minimum_dist_hit.dist; // calculated via ray endpoint + ray.dir * dist 
        vec3 surface_normal = minimum_dist_hit.object->Normal(intersection_point, minimum_dist_hit.part);
        Shader* receiver_shader = minimum_dist_hit.object->material_shader;
        color = receiver_shader->Shade_Surface(ray, intersection_point, surface_normal, recursion_depth);
        color += Estimate_Caustic_Irradiance(*this,receiver_shader,intersection_point,surface_normal);
    }

    return color;
}

void Render_World::Initialize_Hierarchy()
{
    TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.

    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}
