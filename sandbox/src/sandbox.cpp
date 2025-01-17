#include "pch.hpp"

#include "sandbox.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/components/lightComponent.h"
#include "src/components/meshComponent.h"
#include "src/components/renderComponent.h"
#include "src/components/transformComponent.h"
#include "src/core/camera.h"
#include "src/core/node.h"
#include "src/core/renderable.h"
#include "src/core/scene.h"
#include "src/renderer/primitives.h"

#include <external/glfw/include/GLFW/glfw3.h>

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

    u32            data = 0xFF0000FF; // AABBGGRR
    Pinut::Texture redTexture(1, 1, VK_FORMAT_R8G8B8A8_UNORM, &data);
    Pinut::Texture t("../assets/viking_room/viking_room_diffuse.png");

    m_currentScene = new Pinut::Scene();
    auto& registry = m_currentScene->Registry();

    auto monkey = m_currentScene->CreateEntity();
    registry.emplace<Pinut::Component::MeshComponent>(monkey,
                                                      *LoadAsset<Pinut::Mesh>("suzanne.obj"));
    registry.emplace<Pinut::Component::RenderComponent>(monkey, redTexture);

    auto& monkeyTransformComponent = registry.get<Pinut::Component::TransformComponent>(monkey);
    monkeyTransformComponent.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 10.0f));

    const auto vikingRoom = m_currentScene->CreateEntity();
    registry.emplace<Pinut::Component::MeshComponent>(vikingRoom,
                                                      *LoadAsset<Pinut::Mesh>("viking_room.obj"));
    registry.emplace<Pinut::Component::RenderComponent>(vikingRoom, t);

    auto& vikingRoomTransformComponent =
      registry.get<Pinut::Component::TransformComponent>(vikingRoom);
    vikingRoomTransformComponent.model =
      glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));

    auto  directionalLight = m_currentScene->CreateEntity();
    auto& light            = registry.emplace<Pinut::Component::LightComponent>(directionalLight);
    light.m_color          = glm::vec3(1.0f, 0.0f, 0.0f);
    registry.get<Pinut::Component::TransformComponent>(directionalLight).model =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 5.0f));
}

void Sandbox::OnUpdate()
{
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT))
        m_currentCamera->speed = 10.0f;
    else
        m_currentCamera->speed = 5.0f;

    const auto dt = static_cast<f32>(GetDeltaTime());

    if (glfwGetKey(m_window, GLFW_KEY_W))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Forward() * dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_A))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Right() * dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_S))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Forward() * dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_D))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Right() * dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_Z))
    {
        m_currentCamera->UpdateCameraWASD(-m_currentCamera->Up() * dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_X))
    {
        m_currentCamera->UpdateCameraWASD(m_currentCamera->Up() * dt);
    }

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_2))
    {
        m_currentCamera->UpdateRotation(dt, -m_mouse.mouseOffset.x, -m_mouse.mouseOffset.y);
    }
}
