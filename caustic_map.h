#ifndef __CAUSTIC_PHOTON_MAP_H__
#define __CAUSTIC_PHOTON_MAP_H__

#include <random>
#include "photon.h"
#include "shader.h"
#include "reflective_shader.h"
#include "glass_shader.h"
#include "nanoflann/nanoflann.hpp" 

class Caustic_Photon_Map
{
    public:
        typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<double, Photon_Point_Cloud>, Photon_Point_Cloud, 3, size_t> KD_Tree;

        Photon_Point_Cloud point_cloud;
        std::unique_ptr<KD_Tree> kdtree;
};

vec3 Random_Unit_Vector(std::mt19937& rng);
bool Is_Caustic_Specular(const Shader* shader);

#endif