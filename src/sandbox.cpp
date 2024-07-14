#include "stdafx.h"

#include <glfw3.h>

#include "sandbox.h"
#include "src/assets/mesh.h"
#include "src/core/assetManager.h"
#include "src/core/camera.h"
#include "src/core/scene.h"
#include "src/renderer/primitives.h"
#include "src/renderer/renderable.h"

int main()
{
    Run(new Sandbox("Sandbox application"));
    return 0;
}

Sandbox::Sandbox(const std::string& name) : Pinut::Application(name){};

void Sandbox::OnCreate()
{
    m_currentCamera = new Pinut::Camera();
    m_currentCamera->LookAt(glm::vec3(3.0f, 2.0f, 3.0f), glm::vec3(0.0f));
    m_currentCamera->SetProjection(glm::radians(45.0f), (f32)m_width / m_height, 0.01f, 1000.0f);

    auto assetManager = Pinut::AssetManager::Get();

    auto floor = std::make_shared<Pinut::Renderable>();
    auto m     = assetManager->GetAsset<Pinut::Mesh>("UnitCube");
    floor->SetMesh(m);
    floor->SetModel(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)),
                               glm::vec3(5.0f, 0.5f, 5.0f)));

    auto cube = std::make_shared<Pinut::Renderable>();
    cube->SetMesh(assetManager->GetAsset<Pinut::Mesh>("UnitCube"));
    cube->SetModel(glm::mat4(1.0f));

    m_currentScene = new Pinut::Scene();
    m_currentScene->AddRenderable(std::move(floor));
    m_currentScene->AddRenderable(std::move(cube));
}

void Sandbox::OnUpdate()
{
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT))
        m_currentCamera->speed = 3.0f;
    else
        m_currentCamera->speed = 1.0f;

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
}
