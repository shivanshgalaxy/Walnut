//
// Created by sh on 18/05/25.
//
#include "Renderer.h"
#include <algorithm>
#include <string.h>
#include <execution>
#include "Walnut/Random.h"
#include "Camera.h"

namespace Utils
{
    static uint32_t ConvertToRGBA(glm::vec4& color)
    {
        uint8_t r = (uint8_t)(color.r * 255.0f);
        uint8_t g = (uint8_t)(color.g * 255.0f);
        uint8_t b = (uint8_t)(color.b * 255.0f);
        uint8_t a = (uint8_t)(color.a * 255.0f);

        uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r);
        return result;
    }

    static uint32_t PCG_Hash(uint32_t input)
    {
        uint32_t state = input * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    static float RandomFloat(uint32_t& seed)
    {
        seed = PCG_Hash(seed);
        return (float)seed/(float)std::numeric_limits<uint32_t>::max();
    }

    static glm::vec3 InUnitSphere(uint32_t& seed)
    {
        return glm::normalize(glm::vec3(RandomFloat(seed) * 2.0 - 1.0f,
            RandomFloat(seed) * 2.0 - 1.0f,
            RandomFloat(seed) * 2.0 - 1.0f));
    }
}

glm::vec3 RandomVec3(float min = -0.5f, float max = 0.5f) {
    static  std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);

    return glm::vec3(dist(gen), dist(gen), dist(gen));
}


void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if (m_FinalImage)
    {
        // No resize necessary
        if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
            return;
        m_FinalImage->Resize(width, height);
    }
    else
    {
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[width * height];

    delete[] m_AccumulationData;
    m_AccumulationData = new glm::vec4[width * height];

    m_ImageHorizonalIter.resize(width);
    m_ImageVerticalIter.resize(height);
    for (uint32_t i = 0; i < width; i++)
        m_ImageHorizonalIter[i] = i;
    for (uint32_t i = 0; i < height; i++)
        m_ImageVerticalIter[i] = i;

}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    m_ActiveScene = &scene;
    m_ActiveCamera = &camera;

    if (m_FrameIndex == 1)
        memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

    float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();

#define MT 1
#if MT
    std::for_each(std::execution::par,m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
        [this](uint32_t y)
        {
            #if 1
            std::for_each(std::execution::par, m_ImageHorizonalIter.begin(), m_ImageHorizonalIter.end(),
                [this, y](uint32_t x)
                {
                    glm::vec4 color = PerPixel(x, y);
                    m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

                    glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
                    accumulatedColor /= (float)m_FrameIndex;

                    accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
                    m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
                });
            #else
            for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
            {
                glm::vec4 color = PerPixel(x, y);
                m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

                glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
                accumulatedColor /= (float)m_FrameIndex;

                accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
                m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
            }
            #endif
        });
#else
    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
        {
            glm::vec4 color = PerPixel(x, y);
            m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

            glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
            accumulatedColor /= (float)m_FrameIndex;

            accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
        }
    }
#endif
    m_FinalImage->SetData(m_ImageData);

    if (m_Settings.Accumulate)
        m_FrameIndex++;
    else
        m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

    glm::vec3 light(0.0f);
    glm::vec3 colorContribution(1.0f);

    uint32_t seed = x + y * m_FinalImage->GetWidth();
    seed *= m_FrameIndex;
    int bounces = 5;
    for (int i = 0; i < bounces; i++)
    {
        seed += i;
        Renderer::HitPayload payload = TraceRay(ray);
        if (payload.HitDistance < 0.0f)
        {
            glm::vec3 skyColor(0.4f, 0.6f, 0.8f);
            // light += skyColor * colorContribution;
            break;
        }

        // glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));
        // float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDirection), 0.0f); // same as cos(angle)

        const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
        const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];
        light += material.GetEmission();
        colorContribution *= material.Albedo;
        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
        // ray.Direction = glm::reflect(ray.Direction,
        //     payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5, 0.5));
        ray.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));
    }

    return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
    // +ve z axis values correspond to moving backwards
    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // ^ a coefficient    ^ b coefficient     ^ c coefficient
    //
    // a = ray origin
    // b = ray direction
    // t = hit distance
    // r = sphere radius

    if (m_ActiveScene->Spheres.size() == 0)
        return Miss(ray);

    int closestSphere = -1;
    float hitDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
    {
        const Sphere& sphere = m_ActiveScene->Spheres[i];
        // Calculating coefficient values
        glm::vec3 origin = ray.Origin - sphere.Position;
        // Equivalent to rayDirection.x * rayDirection.x + rayDirection.y + rayDirection.y + rayDirection.z + rayDirection.z
        float a = glm::dot(ray.Direction, ray.Direction);
        float b = 2.0f * glm::dot(origin, ray.Direction);
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

        // Quadratic equation discriminant
        // b^2 - 4ac

        float discriminant = b * b - 4.0f * a * c;


        if (discriminant < 0.0f)
            continue;

        // (-b +- sqrt(discriminant)) / 2a
        // float furthestT = (-b + glm::sqrt(discriminant)) / (2.0f * a); // currently unused
        // assuming -b is positive, this will give the smaller (i.e. closer) value
        float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

        if (closestT > 0.0f && closestT < hitDistance)
        {
            hitDistance = closestT;
            closestSphere = (int)i;
        }
    }

    if (closestSphere < 0)
        return Miss(ray);

    return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray &ray, float hitDistance, int objectIndex)
{
    Renderer::HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;
    const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];
    // the closer value, i.e. the 'hit point'
    glm::vec3 origin = ray.Origin - closestSphere.Position;
    payload.WorldPosition = origin + ray.Direction * hitDistance;
    payload.WorldNormal = glm::normalize(payload.WorldPosition);

    payload.WorldPosition += closestSphere.Position;

    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray &ray)
{
    Renderer::HitPayload payload;
    payload.HitDistance = -1.0f;
    return payload;
}
