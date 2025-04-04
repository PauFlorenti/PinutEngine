#include "pch.hpp"

#include <GLFW/glfw3.h>

#include "pinut/assets/material.h"
#include "pinut/assets/mesh.h"
#include "pinut/assets/texture.h"
#include "pinut/components/lightComponent.h"
#include "pinut/components/meshComponent.h"
#include "pinut/components/renderComponent.h"
#include "pinut/components/skyComponent.h"
#include "pinut/components/transformComponent.h"
#include "pinut/core/camera.h"
#include "pinut/core/scene.h"
#include "pinut/renderer/primitives.h"
#include "render_device/textureFormat.h"

#include "sandbox.h"

int main()
{
    Run(std::make_unique<Sandbox>("Sandbox application"));
    return 0;
}

Sandbox::Sandbox(const std::string& name) : Pinut::Application(name) {};

void Sandbox::OnCreate()
{
    m_currentCamera = new Pinut::Camera();
    m_currentCamera->LookAt(glm::vec3(6.0f, 18.0f, 30.0f), glm::vec3(0.0f));
    m_currentCamera->SetProjection(glm::radians(45.0f), (f32)m_width / m_height, 0.01f, 1000.0f);

    m_assetManager.ImportAsset("monkey_smooth\\suzanne.obj");
    m_assetManager.ImportAsset("viking_room\\viking_room.obj");
    m_assetManager.ImportAsset("meshes\\sphere.obj");
    m_assetManager.ImportAsset("textures\\ciel_diffuse.jpg");

    m_currentScene = new Pinut::Scene();
    auto& registry = m_currentScene->Registry();

    const auto monkey = m_currentScene->CreateEntity();
    registry.emplace<Pinut::Component::MeshComponent>(
      monkey,
      m_assetManager.GetAsset<Pinut::Mesh>("meshes\\monkey_smooth\\suzanne.obj.mesh"));

    const auto monkeyMaterial =
      m_assetManager.GetAsset<Pinut::Material>("meshes\\monkey_smooth\\suzanne.mat");
    registry.emplace<Pinut::Component::RenderComponent>(monkey, monkeyMaterial);

    registry.emplace_or_replace<Pinut::Component::TransformComponent>(monkey,
                                                                      glm::vec3(0.0f, 5.0f, 10.0f));

    const auto vikingRoom = m_currentScene->CreateEntity();
    registry.emplace<Pinut::Component::MeshComponent>(
      vikingRoom,
      m_assetManager.GetAsset<Pinut::Mesh>("viking_room\\viking_room.obj.mesh"));

    registry.emplace<Pinut::Component::RenderComponent>(
      vikingRoom,
      m_assetManager.GetAsset<Pinut::Material>("viking_room\\viking_room.mat"));

    registry.emplace_or_replace<Pinut::Component::TransformComponent>(
      vikingRoom,
      glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));

    auto sky = m_currentScene->CreateEntity();
    registry.emplace<Pinut::Component::SkyComponent>(
      sky,
      m_assetManager.GetAsset<Pinut::Texture>("textures\\ciel_diffuse.jpg"),
      m_assetManager.GetAsset<Pinut::Mesh>("meshes\\sphere.obj.mesh"));

    {
        auto  pointLight           = m_currentScene->CreateEntity();
        auto& lightComponent       = registry.emplace<Pinut::Component::LightComponent>(pointLight);
        lightComponent.m_color     = glm::vec3(1.0f);
        lightComponent.m_radius    = 100.0f;
        lightComponent.m_intensity = 10.0f;
        registry.get<Pinut::Component::TransformComponent>(pointLight)
          .SetTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 10.0f)));
    }
    // {
    //     auto  pointLight           = m_currentScene->CreateEntity();
    //     auto& lightComponent       = registry.emplace<Pinut::Component::LightComponent>(pointLight);
    //     lightComponent.m_color     = glm::vec3(1.0f, 0.0f, 0.0f);
    //     lightComponent.m_radius    = 100.0f;
    //     lightComponent.m_intensity = 10.0f;
    //     registry.get<Pinut::Component::TransformComponent>(pointLight)
    //       .SetTransform(glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 10.0f, 5.0f)));
    // }
    // {
    //     auto  pointLight           = m_currentScene->CreateEntity();
    //     auto& lightComponent       = registry.emplace<Pinut::Component::LightComponent>(pointLight);
    //     lightComponent.m_color     = glm::vec3(0.0f, 0.0f, 1.0f);
    //     lightComponent.m_radius    = 100.0f;
    //     lightComponent.m_intensity = 10.0f;
    //     registry.get<Pinut::Component::TransformComponent>(pointLight)
    //       .SetTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 10.0f, 5.0f)));
    // }
}

void Sandbox::OnUpdate()
{
    if (glfwGetKey(m_window.get(), GLFW_KEY_LEFT_SHIFT))
        m_currentCamera->speed = 10.0f;
    else
        m_currentCamera->speed = 5.0f;

    const auto dt = static_cast<f32>(GetDeltaTime());

    if (glfwGetKey(m_window.get(), GLFW_KEY_W))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Forward() * dt);
    }
    if (glfwGetKey(m_window.get(), GLFW_KEY_A))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Right() * dt);
    }
    if (glfwGetKey(m_window.get(), GLFW_KEY_S))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Forward() * dt);
    }
    if (glfwGetKey(m_window.get(), GLFW_KEY_D))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Right() * dt);
    }
    if (glfwGetKey(m_window.get(), GLFW_KEY_Z))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Up() * dt);
    }
    if (glfwGetKey(m_window.get(), GLFW_KEY_X))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Up() * dt);
    }

    if (glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_2))
    {
        m_currentCamera->UpdateRotation(dt, -m_mouse.mouseOffset.x, -m_mouse.mouseOffset.y);
    }
}
