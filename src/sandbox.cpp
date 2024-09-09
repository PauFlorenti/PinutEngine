#include "stdafx.h"

#include <glfw3.h>

#include "sandbox.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
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

Sandbox::Sandbox(const std::string& name) : Pinut::Application(name) {};

void Sandbox::OnCreate()
{
    m_currentCamera = new Pinut::Camera();
    m_currentCamera->LookAt(glm::vec3(18.0f, 15.0f, 32.0f), glm::vec3(0.0f));
    m_currentCamera->SetProjection(glm::radians(45.0f), (f32)m_width / m_height, 0.01f, 1000.0f);

    // ------------------
    // Creating materials
    // ------------------
    Pinut::MaterialData whiteMaterialData;
    whiteMaterialData.diffuse        = 0xFFFFFFFF;
    whiteMaterialData.diffuseTexture = GetAsset<Pinut::Texture>("DefaultWhiteTexture");

    const auto whiteMaterial =
      GetMaterialInstance("WhiteMAT", Pinut::MaterialType::OPAQUE, std::move(whiteMaterialData));

    Pinut::MaterialData redMaterialData;
    redMaterialData.diffuse        = 0xFFFFFFFF;
    redMaterialData.diffuseTexture = GetAsset<Pinut::Texture>("DefaultRedTexture");
    const auto redMaterial =
      GetMaterialInstance("RedMat", Pinut::MaterialType::OPAQUE, std::move(redMaterialData));

    Pinut::MaterialData glassMaterialData;
    glassMaterialData.diffuse        = 0x1FFFFFFF;
    glassMaterialData.diffuseTexture = GetAsset<Pinut::Texture>("DefaultRedTexture");

    const auto glassMaterial = GetMaterialInstance("GlassMAT",
                                                   Pinut::MaterialType::TRANSPARENT,
                                                   std::move(glassMaterialData));

    // ------------------
    // Loading assets
    // ------------------
    auto floor = std::make_shared<Pinut::Renderable>("Floor");
    auto m     = GetAsset<Pinut::Mesh>("UnitCube");
    m->SetMaterial(whiteMaterial);
    floor->SetMesh(std::move(m));
    floor->SetModel(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)),
                               glm::vec3(100.0f, 0.5f, 100.0f)));

    auto glassPlane = std::make_shared<Pinut::Renderable>("Glass plane");
    auto planeMesh  = GetAsset<Pinut::Mesh>("UnitPlane");
    planeMesh->SetMaterial(glassMaterial);
    glassPlane->SetMesh(std::move(planeMesh));
    glassPlane->SetModel(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 15.0f)),
                                    glm::vec3(10.0f, 10.0f, 1.0f)));

    auto monkey = GetRenderable("suzanne.obj", "Suzanne");
    monkey->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 5.0f, 10.0f)));

    auto vikingRoom = GetRenderable("viking_room.obj", "VikingRoom");
    vikingRoom->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));

    auto cornellBox = GetRenderable("cornell_box.obj", "CornellBox");
    cornellBox->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 0.0f)) *
                         glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));

    // ------------------
    // Creating lights
    // ------------------
    Pinut::DirectionalLight directionalLight;
    directionalLight.SetTransform(
      glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)));

    auto l = std::make_shared<Pinut::SpotLight>();
    l->SetPosition(glm::vec3(0.0f, 8.0f, 20.0f));
    l->m_color     = glm::vec3(0.8f, 0.5f, 0.2f);
    l->m_radius    = 50.0f;
    l->m_intensity = 50.0f;

    // ------------------
    // Creating scene
    // ------------------

    m_currentScene = new Pinut::Scene();
    m_currentScene->SetDirectionalLight(std::move(directionalLight));
    m_currentScene->AddRenderable(std::move(floor));
    // m_currentScene->AddRenderable(std::move(glassPlane));
    m_currentScene->AddRenderable(std::move(monkey));
    m_currentScene->AddRenderable(std::move(vikingRoom));
    // m_currentScene->AddRenderable(std::move(cornellBox));
    m_currentScene->AddLight(std::move(l));
    // m_currentScene->AddLight(l2);
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

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_2))
    {
        // glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        m_currentCamera->UpdateRotation(dt, -m_mouse.mouseOffset.x, -m_mouse.mouseOffset.y);
    }
}
