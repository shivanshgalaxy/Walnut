//
// Created by shiva on 5/22/2025.
//

#ifndef SCENE_H
#define SCENE_H

#endif //SCENE_H
#include <vector>
#include <glm/glm.hpp>

struct Sphere
{
    glm::vec3 Position{0.0f};
    float Radius = 0.5f;

    glm::vec3 Albedo{1.0f};
};

struct Scene
{
    std::vector<Sphere> Spheres;
};