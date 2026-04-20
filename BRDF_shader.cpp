#include "BRDF_shader.h"

#include <algorithm>
#include <cstdint> 

#include "light.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"
#include "BSDF_Sample.h"
#include "brdf.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

//Helper functions--------------------------------
static float ShlickleWeight(float x){
    float m = glm::clamp(1.0f - x, 0.0f, 1.0f);

    //returning basicallly (1-x)^5
    return (m*m) * (m*m) * m;
}

glm::float3 BRDF_Shader::Evaluate_Difuse( const glm::float3& wiLocal,
    const glm::float3& woLocal,
    const glm::float3& baseColor,
    float roughness) const{
        float NdotL = glm::clamp(wiLocal.z, 0.0f, 1.0f);
        float NdotV = glm::clamp(woLocal.z, 0.0f, 1.0f);

        if(NdotL <= 0.0f || NdotV <= 0.0f){
            //returning same as emission
            return glm::float3(0.0f,0.0f,0.0f);
        }

        glm::float3 H = glm::normalize(wiLocal + woLocal);
        float LdotH = glm::clamp(glm::dot(wiLocal, H), 0.0f, 1.0f);

        //BRDF formula
        float burl_Diffuse = 0.5f + 2.0f * roughness * LdotH * LdotH;
        float FL = 1.0f + (burl_Diffuse  - 1.0f) * ShlickleWeight(NdotL);
        float FV = 1.0f + (burl_Diffuse  - 1.0f) * ShlickleWeight(NdotV);

        //Made own version of pi
        //Maybe fix real quick later on by just type casting lowkey a 2 sec fix but meh works
        return baseColor * ((1.0f / PI) * FL * FV);

}
//End of Helpers----------------------------------

vec3 BRDF_Shader::Emission() const{
    return vec3(0,0,0);
}

BSDF_Sample BRDF_Shader::Sample(const vec3& normal, const vec3& wo, std::mt19937& rng) const{
    BSDF_Sample sample;

    //Converting our vec3 variables into float3
    glm::float3 N(normal[0], normal[1], normal[2]);
    glm::float3 V(wo[0], wo[1], wo[2]);

    //gettiing quanternation that maps the z-axis into a surface normal + inverse of it
    glm::float4 quatRotationToZ = brdf::getRotationToZAxis(N); //world space allignment
    glm::float4 quatRotationFromZ = brdf::invertRotation(quatRotationToZ); //to convert to local space

    //Transforms outgoing vector dir into local space
    glm::float3 woLocal = brdf::rotatePoint(quatRotationToZ, V);

    //Just getting random numbers
    std::uniform_real_distribution<float> uniform (0.0f, 1.0f);
    glm::float2 u (uniform(rng), uniform(rng));

    //getting pdf to use for monte carlo later on by using the incoming light direction
    float pdf = 0.0f;
    glm::float3 wiLocal = brdf::sampleHemisphere(u, pdf);

    //Same thing as first lines. Converting base color into a float3
    glm::float3 baseColor( color_diffuse[0], color_diffuse[1], color_diffuse[2]);

    //Moves to helper function on top. Essentially does the actual BRDF calculations
    //on the sample incoming and outgoing light directions
    glm::float3 f = Evaluate_Difuse(wiLocal, woLocal, baseColor, roughness);

    //Turn wi back into world space using the inverse quaternian
    glm::float3 wiWorld = brdf::rotatePoint(quatRotationFromZ, wiLocal);

    //Setting the values to be returned in sample for Monte Carlo
    sample.direction = vec3(wiWorld.x, wiWorld.y, wiWorld.z).normalized();
    sample.brdf = vec3(f.x, f.y, f.z);
    sample.pdf = pdf;

    return sample;
}

//Not really used put here to debug but is just the same as phong code
vec3 BRDF_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color = vec3(0, 0, 0);
    return color;
}