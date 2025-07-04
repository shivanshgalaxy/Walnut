#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"
#include "Renderer.h"
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() :
		m_Camera(45.0f, 0.1f, 100.0f)
	{
		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Roughness = 0.1f;
		pinkSphere.isMetallic = false;
		pinkSphere.Albedo = {0.9f, 0.0f, 0.1f};

		Material& greenSphere = m_Scene.Materials.emplace_back();
		greenSphere.Roughness = 0.35f;
		greenSphere.isMetallic = false;
		greenSphere.Albedo = {0.2f, 0.8f, 0.2f};

		Material& orangeSphere = m_Scene.Materials.emplace_back();
		orangeSphere.Roughness = 0.35f;
		orangeSphere.Albedo = {0.8f, 0.2f, 0.1f};
		orangeSphere.EmissionColor = orangeSphere.Albedo;
		orangeSphere.EmissionPower = 15.0f;

		// Blue Metallic Sphere
		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Roughness = 0.05f;
		blueSphere.isMetallic = true;
		blueSphere.Albedo = {0.1f, 0.2f, 0.9f};

		// Yellow Diffuse Sphere
		Material& yellowSphere = m_Scene.Materials.emplace_back();
		yellowSphere.Roughness = 0.6f;
		yellowSphere.isMetallic = false;
		yellowSphere.Albedo = {1.0f, 0.9f, 0.1f};

		// Purple Emissive Sphere
		Material& purpleSphere = m_Scene.Materials.emplace_back();
		purpleSphere.Roughness = 0.2f;
		purpleSphere.isMetallic = false;
		purpleSphere.Albedo = {0.5f, 0.0f, 0.8f};
		purpleSphere.EmissionColor = purpleSphere.Albedo;
		purpleSphere.EmissionPower = 10.0f;

		// Cyan Glossy Sphere
		Material& cyanSphere = m_Scene.Materials.emplace_back();
		cyanSphere.Roughness = 0.15f;
		cyanSphere.isMetallic = false;
		cyanSphere.Albedo = {0.0f, 0.9f, 0.9f};

		{
			Sphere sphere;
			sphere.Position = {-2.5f, 0.0f, 0.0f};
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 3; // Blue Metallic
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = {5.0f, 0.0f, 0.0f};
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 4; // Yellow Diffuse
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = {-5.0f, 0.0f, 0.0f};
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 5; // Purple Emissive
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = {2.5f, 0.0f, 0.0f};
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 6; // Cyan Glossy
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = {0.0f, 0.0f, 0.0f};
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = {0.0f, -101.0f, 0.0f};
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();

	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		};
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();

		ImGui::End();

		ImGui::Begin("Scene");
		ImGui::ColorEdit3("Sky Color", glm::value_ptr(skyColor));
		ImGui::DragFloat("Gamma", &imageGamma, 0.05f, 0.0f, 5.0f);
		ImGui::Text("Objects");
		for (int i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius",&sphere.Radius, 0.1f);
			ImGui::DragInt("Material",&sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::Text("Materials");
		for (int i = 0; i < m_Scene.Materials.size(); i++) {
			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::Checkbox("Metallic", &material.isMetallic);
			ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			// Flip the UV coordinates to make the top-left correspond to 0, 1 and bottom-right to 1, 0
			ImGui::Image(image->GetDescriptorSet(), {(float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	uint32_t* m_ImageData = nullptr;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}


	});
	return app;
}