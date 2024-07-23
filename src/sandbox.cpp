#include "stdafx.h"

#include <glfw3.h>

#include "sandbox.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/assetManager.h"
#include "src/core/camera.h"
#include "src/core/scene.h"
#include "src/renderer/materials/opaqueMaterial.h"
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

    u32  whiteData    = 0xFFFFFFFF;
    auto whiteTexture = Pinut::Texture::CreateFromData(1,
                                                       1,
                                                       4,
                                                       VK_FORMAT_R8G8B8A8_SRGB,
                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                         VK_IMAGE_USAGE_SAMPLED_BIT,
                                                       &whiteData,
                                                       "WhiteTexture");

    u32  redColor   = 0xFF0000FF;
    auto redTexture = Pinut::Texture::CreateFromData(1,
                                                     1,
                                                     4,
                                                     VK_FORMAT_R8G8B8A8_SRGB,
                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                                                     &redColor,
                                                     "RedTexture");

    Pinut::MaterialData whiteMaterialData;
    whiteMaterialData.color   = whiteData;
    whiteMaterialData.diffuse = whiteTexture;

    auto whiteMaterial = m_materialManager.CreateMaterialInstance("WhiteMAT",
                                                                  Pinut::MaterialType::OPAQUE,
                                                                  std::move(whiteMaterialData));

    Pinut::MaterialData glassMaterialData;
    glassMaterialData.color   = 0x7FFF0000;
    glassMaterialData.diffuse = redTexture;

    auto glassMaterial = m_materialManager.CreateMaterialInstance("GlassMAT",
                                                                  Pinut::MaterialType::TRANSPARENT,
                                                                  std::move(glassMaterialData));

    floor->SetMaterial(whiteMaterial);
    cube->SetMaterial(glassMaterial);

    Pinut::Light l;
    l.color     = glm::vec3(1.0f, 1.0f, 1.0f);
    l.position  = glm::vec3(1.0f, 1.0f, 0.0f);
    l.radius    = 100.0f;
    l.intensity = 100.0f;

    Pinut::Light l2;
    l2.color     = glm::vec3(0.0f, 0.0f, 1.0f);
    l2.position  = glm::vec3(-1.0f, 2.0f, 0.0f);
    l2.radius    = 100.0f;
    l2.intensity = 100.0f;

    m_currentScene = new Pinut::Scene();
    m_currentScene->AddRenderable(std::move(floor));
    m_currentScene->AddRenderable(std::move(cube));
    m_currentScene->AddLight(l);
    // m_currentScene->AddLight(l2);
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
