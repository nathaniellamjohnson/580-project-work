#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part) const
{

    // Plane cuts the 3d space in half, called a half space
    // Normal vector points towards the outside
    // so -1 * normal vector points towards the inside

    // Since normal and endpoint are both normalized, calc dot product to calc cos(theta) between the two
    // If negative, then it starts inside 
    double side = dot(this->normal, (ray.endpoint - this->x1).normalized());
    if (side < 0)
    {
        return {this, 0, part}; // inside?
    }

    // check for intersection of the half space inside
    // get the vector & distance from the point to the plane

    // ray plane intersect math
    double denominator = dot(ray.direction, this->normal);
    if (abs(denominator) < small_t) // divide by zero check, constant is potential source of error
    {
        return {nullptr, 0, part}; 
    }

    double numerator = dot((-1.0 * this->normal), (ray.endpoint - this->x1));
    double distance = numerator / denominator;

    if (distance < 0.0)
    {
        return {nullptr, 0, part};  // if distance is negative, then it makes sense that we would not record a hit
        // implies the hit is behind the endpoint & thus cannot be counted 
    }
    else
    {
        return {this, distance, part};
    }
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    //normal is part of the plane so this one is a gimme
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    //also a gimme
    Box b;
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo=-b.hi;
    return b;
}
