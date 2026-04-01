#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    vec3 ray_origin_to_center = ray.endpoint - this->center; 

    double a = dot(ray.direction, ray.direction); // d^2 
    double b = 2.0 * dot(ray.direction, ray_origin_to_center); // 2*(dot(d,o-c))
    double c = dot(ray_origin_to_center, ray_origin_to_center) - (this->radius * this->radius); // (o-c)^2 - r^2

    double det = b*b - 4.0*a*c;
    if (det < 0.0)
    {
        return {nullptr, 0.0, part}; // imaginary solution, no intersection, return nothing
    }
    
    double t = (-b - sqrt(det)) / (2.0 * a); // subtraction for the smaller one, potential source of error if negative?
    
    if (t >= 0.0)
    {
        return {this, t, part}; // either 1 or 2 solutions, choose the 
    }
    else 
    {
        return {nullptr, 0.0, part};
    }
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;
    normal = (point - this->center).normalized();
    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    TODO; // calculate bounding box
    return box;
}
