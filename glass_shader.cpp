#include "glass_shader.h"
#include "ray.h"
#include "render_world.h"

// Conversion helpers
inline glm::float3 to_glm(const vec3& v) { return glm::float3(v[0], v[1], v[2]); }
inline vec3 from_glm(const glm::float3& v) { return vec3(v.x, v.y, v.z); }

vec3 Glass_Shader::Shade_Surface(const Ray& ray, const vec3& intersection_point,
    const vec3& normal, int recursion_depth) const
{
    if (recursion_depth >= world.recursion_depth_limit) return {0, 0, 0};

    // 1. Setup Vectors
    glm::float3 V = glm::normalize(-to_glm(ray.direction));
    glm::float3 N = glm::normalize(to_glm(normal));
    float eta_i = 1.0f, eta_t = ior;

    // Handle internal vs external hits
    if (glm::dot(N, V) < 0) {
        N = -N;
        std::swap(eta_i, eta_t);
    }

    // 2. Calculate Fresnel using brdf.h
    // For glass, F0 is derived from IOR: ((1-ior)/(1+ior))^2
    float r0 = pow((eta_i - eta_t) / (eta_i + eta_t), 2);
    glm::float3 F0 = glm::float3(r0);
    float cosTheta = glm::clamp(glm::dot(N, V), 0.0f, 1.0f);
    
    // Use the library's Fresnel function
    glm::float3 F = brdf::evalFresnel(F0, 1.0f, cosTheta);
    float reflection_chance = glm::clamp(F.r, 0.0f, 1.0f);

    // 3. Reflection
    vec3 reflect_color(0,0,0);
    glm::float3 R = glm::reflect(-V, N);
    Ray reflect_ray(intersection_point + from_glm(N) * 0.001, from_glm(R));
    reflect_color = world.Cast_Ray(reflect_ray, recursion_depth + 1);

    // 4. Refraction (Transmittance)
    vec3 refract_color(0,0,0);
    float eta = eta_i / eta_t;
    float k = 1.0f - eta * eta * (1.0f - cosTheta * cosTheta);

    if (k >= 0) {
        glm::float3 T = eta * (-V) + (eta * cosTheta - sqrt(k)) * N;
        Ray refract_ray(intersection_point - from_glm(N) * 0.001, from_glm(T));
        refract_color = world.Cast_Ray(refract_ray, recursion_depth + 1);
        
        // Final Color: Mix by Fresnel
        return reflect_color * reflection_chance + refract_color * (1.0f - reflection_chance);
    } else {
        // Total Internal Reflection
        return reflect_color;
    }
}