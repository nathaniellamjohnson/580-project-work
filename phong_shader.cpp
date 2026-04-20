#include <algorithm>
#include "brdf.h"
#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    std::cout << "(Phong_Shader) - Shade surface should not be called in MCPT" << std::endl;
    vec3 color;
    
    // Lights array from world object linked in shader
    // Relevant vars from light are the vec3 position & their Emitted_Light function
    auto lights_array = this->world.lights;

    // Psuedocode:
    // grant ambient light to the object -> every object gets this regardless of lighting 
    color += this->color_ambient * this->world.ambient_intensity * this->world.ambient_color; // ambient
    
    // for each light in lights array
    for (Light* light : lights_array)
    {
        // Shadow ray logic
        // If the shadows enabled in .txt file
        if (this->world.enable_shadows)
        {
            // cast a shadow ray starting from the intersection point towards the light position
            vec3 shadow_ray_dir = (light->position - intersection_point).normalized();
            double shadow_ray_distance = (light->position - intersection_point).magnitude();
            Ray shadow_ray = Ray(intersection_point, shadow_ray_dir);
            Hit shadow_ray_hit = this->world.Closest_Intersection(shadow_ray);

            // If we find something AND that something is not the intersection we know about, then no non-ambient light gets through.
            // Continue on to rest of the lights
            if (shadow_ray_hit.object != nullptr && (shadow_ray_distance - shadow_ray_hit.dist) > small_t) 
            {
                continue; // something found between light and intersection point
            }
        }

        vec3 to_light_vec = (light->position - intersection_point); // unnormalized version, points directly to light?
        vec3 to_light = (light->position - intersection_point).normalized();

        // Diffuse light logic
        vec3 object_diffuse = this->color_diffuse;
        double diffuse_intensity = std::min(std::max(0.0,dot(to_light, normal)), 1.0); // dot(light, normal)
        color += object_diffuse * diffuse_intensity * light->Emitted_Light(to_light_vec); // Problem here -> need to multiply by the inverse square law & the emitted light also tells you the light color!

        // Specular light logic
        vec3 object_specular = this->color_specular;
        vec3 reflected = 2.0 * (normal * (std::min(std::max(0.0,dot(to_light, normal)), 1.0))) - to_light; // 2 * (normal(dot(to_light, normal))) - to_light
        double specular_intensity = pow(std::min(std::max(0.0,dot(reflected, -1.0 * ray.direction)), 1.0), specular_power);
        color += object_specular * specular_intensity * light->Emitted_Light(to_light_vec);         
    }

    return color;
}

// Monte Carlo Path Tracing Functions (Per shader basis)
vec3 Phong_Shader::Emission() const 
{
    return vec3(0.0, 0.0, 0.0); // phong shader not emissive
}

BSDF_Sample Phong_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const
{
    // this version uses branching weighted by luminance 
    // can also do a mixing situation where we calculate 
    BSDF_Sample result;

    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    float xi = uniform(rng); // random [0, 1] float used to randomly choose between diffuse and specular branch

    glm::float3 N(normal[0], normal[1], normal[2]); // glm normal
    glm::float3 V(wo[0], wo[1], wo[2]); // outgoing light

    // Luminance from linear rgb values - https://en.wikipedia.org/wiki/Relative_luminance
    // Used to determine if diffuse or specular has a higher important
    auto luminance = [](const vec3& c)
    {
        return 0.2126*c.x[0] + 0.7152*c.x[1] + 0.0722*c.x[2];
    };

    float diffuse_luminance = luminance(color_diffuse);
    float specular_luminance = luminance(color_specular);
    float sum_luminance = diffuse_luminance + specular_luminance + 1e-6f;

    float probability_diffuse = diffuse_luminance / sum_luminance;
    float probability_luminance = specular_luminance / sum_luminance;

    glm::float3 Llocal;
    float pdf = 0.0f;
    glm::float3 f(0,0,0);

    // Build local frame
    glm::float4 quatRotationToZ = brdf::getRotationToZAxis(N);
    glm::float4 quatRotationFromZ = brdf::invertRotation(quatRotationToZ);

    glm::float3 Vlocal = brdf::rotatePoint(quatRotationToZ, V);

    glm::float2 u(uniform(rng), uniform(rng)); // Polar coords for diffuse 

    if (xi < probability_diffuse) // Diffuse branch
    {
        // see Flat_Shader::sample for indepth comments
        Llocal = brdf::sampleHemisphere(u, pdf);

        float cosTheta = Llocal.z;

        // Lambertian BRDF
        f = glm::float3(color_diffuse[0], color_diffuse[1], color_diffuse[2]) * (1.0f / PI);

        // mixture pdf
        pdf = probability_diffuse * pdf;
    }
    else // Specular branch
    {
        float shininess = specular_power;

        // samplePhong returns a direction in lobe space centered around +Z.
        // rotate the lobe to align with the perfect reflection direction
        glm::float3 Lphong = brdf::samplePhong(Vlocal, shininess, u, pdf);
        const glm::float3 Nlocal(0.0f, 0.0f, 1.0f);
        glm::float3 lobe_direction = reflect(-Vlocal, Nlocal);
        Llocal = brdf::rotatePoint(brdf::getRotationFromZAxis(lobe_direction), Lphong);

        // Evaluate Phong BRDF
        float norm = (shininess + 2.0f) / (2.0f * PI);

        glm::float3 R = reflect(-Llocal, Nlocal);
        float spec = pow(glm::max(0.0f, dot(R, Vlocal)), shininess);

        f = glm::float3(color_specular.x[0], color_specular.x[1], color_specular.x[2]) * norm * spec;

        pdf = probability_luminance * pdf;
    }

    // Transform back to world
    glm::float3 Lworld = brdf::rotatePoint(quatRotationFromZ, Llocal);

    result.direction = vec3(Lworld.x, Lworld.y, Lworld.z).normalized();
    result.brdf = vec3(f.x, f.y, f.z);
    result.pdf = pdf;

    return result;
}


// Multiple importance sampling example
// Smooths out final image noise


// BSDF_Sample Phong_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const
// {
//     BSDF_Sample result;

//     std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
//     float xi = uniform(rng);

//     glm::float3 N(normal[0], normal[1], normal[2]);
//     glm::float3 V(wo[0], wo[1], wo[2]);

//     auto luminance = [](const vec3& c)
//     {
//         return 0.2126*c.x[0] + 0.7152*c.x[1] + 0.0722*c.x[2];
//     };

//     float diffuse_luminance = luminance(color_diffuse);
//     float specular_luminance = luminance(color_specular);
//     float sum_luminance = diffuse_luminance + specular_luminance;

//     if (sum_luminance <= 1e-6f)
//     {
//         result.direction = vec3(0.0, 0.0, 0.0);
//         result.brdf = vec3(0.0, 0.0, 0.0);
//         result.pdf = 0.0;
//         return result;
//     }

//     float probability_diffuse = diffuse_luminance / sum_luminance;
//     float probability_specular = specular_luminance / sum_luminance;

//     glm::float3 Llocal;
//     float sampled_pdf = 0.0f;

//     glm::float4 quatRotationToZ = brdf::getRotationToZAxis(N);
//     glm::float4 quatRotationFromZ = brdf::invertRotation(quatRotationToZ);
//     glm::float3 Vlocal = brdf::rotatePoint(quatRotationToZ, V);

//     glm::float2 u(uniform(rng), uniform(rng));
//     const glm::float3 Nlocal(0.0f, 0.0f, 1.0f);
//     float shininess = (float)specular_power;

//     if (xi < probability_diffuse)
//     {
//         Llocal = brdf::sampleHemisphere(u, sampled_pdf);
//     }
//     else
//     {
//         // taken from brdf.h sampleSpecularPhong function
//         // Sampled LPhong is in "lobe space" - where Phong lobe is centered around +Z axis
// 	    // We need to rotate it in direction of perfect reflection
//         glm::float3 Lphong = brdf::samplePhong(Vlocal, shininess, u, sampled_pdf);
//         glm::float3 lobe_direction = reflect(-Vlocal, Nlocal);
//         Llocal = brdf::rotatePoint(brdf::getRotationFromZAxis(lobe_direction), Lphong);
//     }

//     float NdotL = glm::max(0.0f, Llocal.z);
//     if (NdotL <= 0.0f)
//     {
//         result.direction = vec3(0.0, 0.0, 0.0);
//         result.brdf = vec3(0.0, 0.0, 0.0);
//         result.pdf = 0.0;
//         return result;
//     }

//     glm::float3 Rlocal = reflect(-Llocal, Nlocal);
//     float RdotV = glm::max(0.0f, dot(Rlocal, Vlocal));

//     glm::float3 diffuse_brdf = glm::float3(color_diffuse[0], color_diffuse[1], color_diffuse[2]) * (1.0f / PI);
//     float phong_brdf_norm = (shininess + 2.0f) / (2.0f * PI);
//     glm::float3 specular_brdf = glm::float3(color_specular[0], color_specular[1], color_specular[2])
//         * (phong_brdf_norm * pow(RdotV, shininess));

//     float diffuse_pdf = NdotL / PI;
//     float specular_pdf = brdf::phongNormalizationTerm(shininess) * pow(RdotV, shininess);
//     float mixture_pdf = probability_diffuse * diffuse_pdf + probability_specular * specular_pdf;

//     if (mixture_pdf <= 0.0f)
//     {
//         result.direction = vec3(0.0, 0.0, 0.0);
//         result.brdf = vec3(0.0, 0.0, 0.0);
//         result.pdf = 0.0;
//         return result;
//     }

//     glm::float3 Lworld = brdf::rotatePoint(quatRotationFromZ, Llocal);

//     result.direction = vec3(Lworld.x, Lworld.y, Lworld.z).normalized();
//     result.brdf = vec3(diffuse_brdf.x + specular_brdf.x,
//                        diffuse_brdf.y + specular_brdf.y,
//                        diffuse_brdf.z + specular_brdf.z);
//     result.pdf = mixture_pdf;

//     return result;
// }