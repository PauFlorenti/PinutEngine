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

    // ------------------
    // Creating materials
    // ------------------
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

    Pinut::MaterialData redMaterialData;
    redMaterialData.color   = redColor;
    redMaterialData.diffuse = redTexture;
    const auto redMaterial  = m_materialManager.CreateMaterialInstance("RedMat",
                                                                      Pinut::MaterialType::OPAQUE,
                                                                      std::move(redMaterialData));

    Pinut::MaterialData glassMaterialData;
    glassMaterialData.color   = 0x7FFF0000;
    glassMaterialData.diffuse = redTexture;

    auto glassMaterial = m_materialManager.CreateMaterialInstance("GlassMAT",
                                                                  Pinut::MaterialType::TRANSPARENT,
                                                                  std::move(glassMaterialData));

    auto vikingTexture =
      Pinut::Texture::CreateFromFile("../assets/viking_room/viking_room_diffuse.png",
                                     "VikingRoomDiffuse");

    Pinut::MaterialData vikingMaterialData;
    vikingMaterialData.color   = whiteData;
    vikingMaterialData.diffuse = vikingTexture;

    auto vikingMaterial = m_materialManager.CreateMaterialInstance("VikingRoomMaterial",
                                                                   Pinut::MaterialType::OPAQUE,
                                                                   std::move(vikingMaterialData));

    // ------------------
    // Loading assets
    // ------------------
    auto assetManager = Pinut::AssetManager::Get();

    assetManager->LoadAsset("../assets/monkey_smooth/suzanne.obj", "Suzanne");
    // assetManager->LoadAsset("../assets/viking_room/viking_room.obj", "VikingRoom");
    // assetManager->LoadAsset("../assets/cornell_box/cornell_box.obj", "CornellBox");

    auto floor = std::make_shared<Pinut::Renderable>("Floor");
    auto m     = assetManager->GetAsset<Pinut::Mesh>("UnitCube");
    m->SetMaterial(whiteMaterial);
    floor->SetMesh(std::move(m));
    floor->SetModel(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)),
                               glm::vec3(5.0f, 0.5f, 5.0f)));

    auto glassPlane = std::make_shared<Pinut::Renderable>("Glass plane");
    auto planeMesh  = assetManager->GetAsset<Pinut::Mesh>("UnitPlane");
    planeMesh->SetMaterial(glassMaterial);
    glassPlane->SetMesh(std::move(planeMesh));
    glassPlane->SetModel(glm::mat4(1.0f));

    auto cube    = std::make_shared<Pinut::Renderable>("Cube");
    auto redCube = assetManager->GetAsset<Pinut::Mesh>("UnitCube");
    redCube->SetMaterial(redMaterial);
    cube->SetMesh(std::move(redCube));
    cube->SetModel(glm::mat4(1.0f));

    //auto planeMesh = assetManager->GetAsset<Pinut::Mesh>("UnitCube");

    // auto monkey = std::make_shared<Pinut::Renderable>("Suzanne");
    // monkey->SetMesh(assetManager->GetAsset<Pinut::Mesh>("Suzanne"));
    // monkey->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)));

    // auto vikingRoom = std::make_shared<Pinut::Renderable>("VikingRoom");
    // vikingRoom->SetMesh(assetManager->GetAsset<Pinut::Mesh>("VikingRoom"));
    // vikingRoom->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));

    // auto cornellBox = std::make_shared<Pinut::Renderable>("CornellBox");
    // cornellBox->SetMesh(assetManager->GetAsset<Pinut::Mesh>("CornellBox"));
    // cornellBox->SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)));

    // ------------------
    // Creaint lights
    // ------------------
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

    // ------------------
    // Creating scene
    // ------------------

    m_currentScene = new Pinut::Scene();
    m_currentScene->AddRenderable(std::move(floor));
    m_currentScene->AddRenderable(std::move(glassPlane));
    m_currentScene->AddRenderable(std::move(cube));
    // m_currentScene->AddRenderable(std::move(monkey));
    // m_currentScene->AddRenderable(std::move(vikingRoom));
    // m_currentScene->AddRenderable(std::move(cornellBox));
    m_currentScene->AddLight(l);
    m_currentScene->AddLight(l2);
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

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_2))
    {
        m_currentCamera->UpdateRotation(dt, -m_mouse.mouseOffset.x, -m_mouse.mouseOffset.y);
    }
}
