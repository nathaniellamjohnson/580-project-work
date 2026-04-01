#include <limits>
#include "box.h"

// Return whether the ray intersects this box.
bool Box::Intersection(const Ray& ray) const
{
    TODO; // FIXME

    // divide by zero check, basically parallel rays
    for (int i = 0; i < 3; i++) 
    {
        if (ray.direction[i] == 0.0) {
            if (ray.endpoint[i] < lo[i] || ray.endpoint[i] > hi[i])
                return false;
        }
    }

    // pulled from textbook page 280 -> slab method on wikipedia
    // basically get parameter t where ray would intersect the x/y_min and x/y_max

    double t_min_x = ( this->lo[0] - ray.endpoint[0]) / ray.direction[0];
    double t_max_x = ( this->hi[0] - ray.endpoint[0]) / ray.direction[0];
    if (t_min_x > t_max_x)
    {
        std::swap(t_min_x, t_max_x);
    }

    double t_min_y = ( this->lo[1] - ray.endpoint[1]) / ray.direction[1];
    double t_max_y = ( this->hi[1] - ray.endpoint[1]) / ray.direction[1];
    if (t_min_y > t_max_y)
    {
        std::swap(t_min_y, t_max_y);
    }

    if ((t_min_x > t_max_y) || (t_min_y > t_max_x))
    {
        return false;
    }

    double t_min = std::max(t_min_x, t_min_y);
    double t_max = std::min(t_max_x, t_max_y); 

    double t_min_z = ( this->lo[2] - ray.endpoint[2]) / ray.direction[2];
    double t_max_z = ( this->hi[2] - ray.endpoint[2]) / ray.direction[2];
    
    if (t_min_z > t_max_z)
    {
        std::swap(t_min_z, t_max_z);
    }

    if ((t_min_z > t_max) || (t_min > t_max_z))
    {
        return false;
    }

    t_min = std::max(t_min, t_min_z);
    t_max = std::min(t_max, t_max_z);

    return true;

}

// Compute the smallest box that contains both *this and bb.
Box Box::Union(const Box& bb) const
{
    // FIXME how to verfiy

    Box box;

    box.lo = vec3(std::min(this->lo[0], bb.lo[0]) , std::min(this->lo[1], bb.lo[1]) , std::min(this->lo[2], bb.lo[2]));
    box.hi = vec3(std::max(this->lo[0], bb.lo[0]) , std::max(this->lo[1], bb.lo[1]) , std::max(this->lo[2], bb.lo[2]));

    TODO;
    return box;
}

// Enlarge this box (if necessary) so that pt also lies inside it.
void Box::Include_Point(const vec3& pt)
{
    // FIXME how to verify

    this->lo = vec3(std::min(this->lo[0], pt[0]) , std::min(this->lo[1], pt[1]) , std::min(this->lo[2], pt[2]));
    this->hi = vec3(std::max(this->lo[0], pt[0]) , std::max(this->lo[1], pt[1]) , std::max(this->lo[2], pt[2]));
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}
