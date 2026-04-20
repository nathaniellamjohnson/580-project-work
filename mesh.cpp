#include "mesh.h"
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <cstdint>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts). Given.
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{

    TODO; //implement Mesh+ray Intersection
    Hit hit;

    if (part >= 0) // Intersect only specific part
    {
        double tri_dist = 0.0;
        bool tri_intersect_bool = Intersect_Triangle(ray, part, tri_dist);

        if (tri_intersect_bool)
        {
            hit.object = this; 
            hit.dist = tri_dist;
            hit.part = part;
        }
        else
        {
            hit.object = nullptr;
        }
    }
    else // Intersect over all parts, get min distance
    {
        // Base case of no intersections
        hit.object = nullptr;
        hit.dist = INFINITY;
        
        for (uint64_t part = 0; part < number_parts; part++)
        {
            double tri_dist = 0.0;
            bool tri_intersect_bool = Intersect_Triangle(ray, part, tri_dist);

            if (tri_intersect_bool && hit.dist > tri_dist)
            {
                hit.object = this; 
                hit.dist = tri_dist;
                hit.part = part;
            }
        }
    }

    return hit;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
    assert(part>=0);

    ivec3 current_triangle = triangles[part];

    vec3 tri_normal;

    vec3 vertex_a = vertices[current_triangle[0]];
    vec3 vertex_b = vertices[current_triangle[1]];
    vec3 vertex_c = vertices[current_triangle[2]];

    vec3 ab_vec = (vertex_b - vertex_a);
    vec3 ac_vec = (vertex_c - vertex_a);

    tri_normal = cross(ab_vec, ac_vec).normalized();

    TODO; //implement tri normal calculation

    // i'm guessing that the part points to triangles
    // and triangles is 3 integers that are the vertex indicies
    // can make 2 vectors from a, b, c -> ab & ac
    // get the cross product of ab x ac
    // FIXME: how to verify they are the correct way? -> does the order of triangle help?
    // clockwise or counter clockwise winding? wtf is winding

    return tri_normal;
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
    ivec3 points = triangles[tri];
    vec3 vertex_a = vertices[points[0]];
    vec3 vertex_b = vertices[points[1]];
    vec3 vertex_c = vertices[points[2]];

    TODO; //implement tri+ray intersection

    // plane def by point and normal
    // Get plane normal 
    vec3 plane_normal = cross(vertex_b - vertex_a, vertex_c - vertex_a); // whoa that's fucked up! -> why is plane.normal not == to what this is 

    // can get intersection via plane intersection
    double denominator = dot(ray.direction, plane_normal); // divide by zero check
    if (abs(denominator) < small_t)
    {
        return false;
    }

    double numerator = dot((-1.0 * plane_normal), (ray.endpoint - vertex_a));
    dist = numerator / denominator;

    // get distance & compare to small_t
    if (dist <= small_t)
    {
        return false; // distance too small
    }

    vec3 intersection_point = ray.endpoint + ray.direction * dist;

    // convert to barycentric coordinates
    // see fundamentals of computer graphics pg 49
    vec3 n_a = cross((vertex_c - vertex_b), (intersection_point - vertex_b));
    vec3 n_b = cross((vertex_a - vertex_c), (intersection_point - vertex_c));
    vec3 n_c = cross((vertex_b - vertex_a), (intersection_point - vertex_a));

    double alpha = dot(plane_normal, n_a) / (plane_normal.magnitude_squared());
    double beta = dot(plane_normal, n_b) / (plane_normal.magnitude_squared());
    double gamma = dot(plane_normal, n_c) / (plane_normal.magnitude_squared());

    // check to see if the coords are in side 
    // Consider a triangle to intersect a ray if the ray intersects the plane of the
    // triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
    bool alpha_in_weight_tolerance = (!(alpha < -weight_tolerance) && (alpha < 1+weight_tolerance)); // FIXME better way to do this
    bool beta_in_weight_tolerance = (!(beta < -weight_tolerance) && (beta < 1+weight_tolerance));
    bool gamma_in_weight_tolerance = (!(gamma < -weight_tolerance) && (gamma < 1+weight_tolerance));

    if (alpha_in_weight_tolerance && beta_in_weight_tolerance && gamma_in_weight_tolerance)
    {
        return true;
    }

    return false;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    TODO; // FIXME
    
    ivec3 triangle = triangles[part];
    for (int vertex_idx : triangle.x)
    {
        b.Include_Point(vertices[vertex_idx]);
    }
    
    return b;
}
