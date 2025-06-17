//
// Created by shiva on 5/22/2025.
//

#ifndef SCENE_H
#define SCENE_H

#endif //SCENE_H
#include <vector>
#include <glm/glm.hpp>

struct Material {
    glm::vec3 Albedo{1.0f};
    float Roughness = 1.0f;
    bool isMetallic = false;
    float EmissionPower = 0.0f;
    glm::vec3 EmissionColor{ 0.0f };
    glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
};

inline glm::vec3 skyColor(0.75f, 0.75f, 0.8f);
inline float imageGamma = 1.2f;
struct Sphere
{
    glm::vec3 Position{0.0f};
    float Radius = 0.5f;
    int MaterialIndex = 0;
};

struct Scene
{
    std::vector<Sphere> Spheres;
    std::vector<Material> Materials;
};