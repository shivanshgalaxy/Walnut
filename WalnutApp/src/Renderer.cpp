//
// Created by sh on 18/05/25.
//
#include "Renderer.h"
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
}

void Renderer::Render(const Camera& camera)
{
    Ray ray{};
    ray.Origin = camera.GetPosition();

    float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();
    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
        {
            ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
            glm::vec4 color = TraceRay(ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
    // uint8_t r = (uint8_t)(coord.x * 255.0f);
    // uint8_t g = (uint8_t)(coord.y * 255.0f);

    // +ve z axis values correspond to moving backwards
    // glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
    // glm::vec3 rayDirection = { coord.x, coord.y, -1.0f };
    float radius = 0.5f;
    // rayDirection = glm::normalize(rayDirection);

    // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
    // ^ a coefficient    ^ b coefficient     ^ c coefficient
    // a = ray origin
    // b = ray direction
    // t = hit distance
    // r = sphere radius


    // Calculating coefficient values

    // Equivalent to rayDirection.x * rayDirection.x + rayDirection.y + rayDirection.y + rayDirection.z + rayDirection.z
    float a = glm::dot(ray.Direction, ray.Direction);
    float b = 2.0f * glm::dot(ray.Origin, ray.Direction);
    float c = glm::dot(ray.Origin, ray.Origin) - radius * radius;

    // Quadratic equation discriminant
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c;


    if (discriminant < 0.0f)
        return glm::vec4(0, 0, 0, 1);

    // (-b +- sqrt(discriminant)) / 2a
    float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
    // assuming -b is positive, this will give the smaller (i.e. closer) value
    float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

    glm::vec3 h0 = ray.Origin + ray.Direction * t0;
    // the closer value, i.e. the 'hit point'
    glm::vec3 h1 = ray.Origin + ray.Direction * t1;
    glm::vec3 normal = glm::normalize(h1);

    glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));

    float d = glm::max(glm::dot(normal, -lightDirection), 0.0f); // == cos(angle)

    glm::vec3 sphereColor(0.596, 1, 0.360);
    // sphereColor = normal * 0.5f + 0.5f;
    sphereColor *= d;
    return glm::vec4(sphereColor, 1.0f);
}
