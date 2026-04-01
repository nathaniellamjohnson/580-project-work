#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

//#include <iostream>
//using namespace std;

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

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

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
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
        color = minimum_dist_hit.object->material_shader->Shade_Surface(ray, intersection_point, minimum_dist_hit.object->Normal(intersection_point, minimum_dist_hit.part), recursion_depth);
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
