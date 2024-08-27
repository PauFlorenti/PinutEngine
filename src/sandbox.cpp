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

    auto whiteMaterial =
      GetMaterialInstance("WhiteMAT", Pinut::MaterialType::OPAQUE, std::move(whiteMaterialData));

    Pinut::MaterialData redMaterialData;
    redMaterialData.diffuse        = 0xFFFFFFFF;
    redMaterialData.diffuseTexture = GetAsset<Pinut::Texture>("DefaultRedTexture");
    const auto redMaterial =
      GetMaterialInstance("RedMat", Pinut::MaterialType::OPAQUE, std::move(redMaterialData));

    Pinut::MaterialData glassMaterialData;
    glassMaterialData.diffuse        = 0xFFFFFFFF;
    glassMaterialData.diffuseTexture = GetAsset<Pinut::Texture>("DefaultRedTexture");

    auto glassMaterial = GetMaterialInstance("GlassMAT",
                                             Pinut::MaterialType::TRANSPARENT,
                                             std::move(glassMaterialData));

    // ------------------
    // Loading assets
    // ------------------
    auto floor = std::make_shared<Pinut::Renderable>("Floor");
    auto m     = GetAsset<Pinut::Mesh>("UnitCube");
    m->SetMaterial(whiteMaterial);
    floor->SetMesh(std::move(m));
    floor->SetModel(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)),
                               glm::vec3(100.0f, 0.5f, 100.0f)));

    auto glassPlane = std::make_shared<Pinut::Renderable>("Glass plane");
    auto planeMesh  = GetAsset<Pinut::Mesh>("UnitPlane");
    planeMesh->SetMaterial(glassMaterial);
    glassPlane->SetMesh(std::move(planeMesh));
    glassPlane->SetModel(glm::mat4(1.0f));

    auto cube    = std::make_shared<Pinut::Renderable>("Cube");
    auto redCube = GetAsset<Pinut::Mesh>("UnitCube");
    redCube->SetMaterial(redMaterial);
    cube->SetMesh(std::move(redCube));
    cube->SetModel(glm::mat4(1.0f));

    auto monkey = std::make_shared<Pinut::Renderable>("Suzanne");
    monkey->SetMesh(GetAsset<Pinut::Mesh>("suzanne.obj"));
    monkey->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 5.0f, 10.0f)));

    auto vikingRoom = std::make_shared<Pinut::Renderable>("VikingRoom");
    vikingRoom->SetMesh(GetAsset<Pinut::Mesh>("viking_room.obj"));
    vikingRoom->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));

    auto cornellBox = std::make_shared<Pinut::Renderable>("CornellBox");
    cornellBox->SetMesh(GetAsset<Pinut::Mesh>("cornell_box.obj"));
    cornellBox->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 0.0f)) *
                         glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));

    // ------------------
    // Creating lights
    // ------------------
    Pinut::Light l;
    l.color     = glm::vec3(1.0f, 1.0f, 1.0f);
    l.position  = glm::vec3(1.0f, 1.0f, 0.0f);
    l.radius    = 100.0f;
    l.intensity = 100.0f;

    Pinut::Light l2;
    l2.color          = glm::vec3(0.0f, 0.0f, 1.0f);
    l2.position       = glm::vec3(20.0f, 2.0f, 0.0f);
    l2.radius         = 50.0f;
    l2.intensity      = 50.0f;
    l2.cosine         = 30.0f;
    // l2.rotation       = glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));
    l2.cosineExponent = 10.0f;

    // ------------------
    // Creating scene
    // ------------------

    m_currentScene = new Pinut::Scene();
    m_currentScene->SetDirectionalLight(glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(1.0f), 0.1f);
    m_currentScene->AddRenderable(std::move(floor));
    // m_currentScene->AddRenderable(std::move(glassPlane));
    // m_currentScene->AddRenderable(std::move(cube));
    m_currentScene->AddRenderable(std::move(monkey));
    m_currentScene->AddRenderable(std::move(vikingRoom));
    m_currentScene->AddRenderable(std::move(cornellBox));
    // m_currentScene->AddLight(l);
    m_currentScene->AddLight(l2);
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
