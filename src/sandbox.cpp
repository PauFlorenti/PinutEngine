// Some comment

#include "stdafx.h"

#include "sandbox.h"

#include "glm/glm.hpp"
#include "renderer.h"
#include "src/core/texture.h"
#include "src/misc/camera.h"
#include "src/scene/material.h"
#include "src/scene/mesh.h"
#include "src/scene/scene.h"

// Feed a path to a json file ...
static Pinut::Scene *LoadScene()
{
    Pinut::Scene *s = new Pinut::Scene();

    return s;
}

Sandbox::Sandbox(const char *name) : Application(std::move(name))
{
}

void Sandbox::OnCreate()
{
    renderer = new Renderer();
    renderer->OnCreate(&device, &swapchain);

    OnResize();
    OnUpdateDisplay();

    camera = new Pinut::Camera();
    camera->LookAt(glm::vec3(0.0f, 2.0f, -5.0f), glm::vec3(0.0f));
    camera->SetProjection(glm::radians(45.0f), (float)width / height, 0.01f, 1000.0f);

    Texture *albedo = new Texture();
    albedo->CreateFromFile(&device, "../assets/wall/wall_albedo.jpg", "albedo");
    Texture *normal = new Texture();
    normal->CreateFromFile(&device, "../assets/wall/wall_normal.jpg", "normal");
    Texture *roughness = new Texture();
    roughness->CreateFromFile(&device, "../assets/wall/wall_roughness.jpg", "roughness");

    auto m = new Material();
    m->SetAlbedo(std::move(albedo));
    m->SetNormal(std::move(normal));
    m->SetRoughness(std::move(roughness));

    auto plane = new Scene::Renderable();
    plane->SetMesh(CreateUnitPlane());
    plane->SetMaterial(std::move(m));
    plane->Upload(&device);
    plane->model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

    auto white_texture = new Texture();
    auto blue_texture = new Texture();
    auto grey_texture = new Texture();
    uint32_t white_data = 0xFFFFFFFF;
    uint32_t black_data = 0x00000000;
    uint32_t blue_data = 0xFFFF0000;
    uint32_t grey_data = 0xFFCCCCCC;
    white_texture->CreateFromData(&device, 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &white_data, "White texture");
    blue_texture->CreateFromData(&device, 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &blue_data, "Blue texture");
    grey_texture->CreateFromData(&device, 1, 1, VK_FORMAT_R8G8B8A8_SRGB, &grey_data, "Grey texture");

    auto white_material = new Material();
    white_material->SetAlbedo(white_texture);
    white_material->SetNormal(blue_texture);
    white_material->SetRoughness(grey_texture);

    Scene::Renderable *floor = new Scene::Renderable();
    floor->SetMesh(CreateUnitPlane());
    floor->SetMaterial(std::move(white_material));
    floor->Upload(&device);
    floor->model = glm::translate(glm::mat4(1), glm::vec3(0.0f, -2.0f, 0.0f)) * glm::rotate(glm::mat4(1), glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1), glm::vec3(10));

    Light l{};
    l.color = glm::vec3(0.77f);
    l.position = glm::vec3(0.f, 1.0f, 1.f);
    l.intensity = 1.0f;
    l.radius = 10.0f;
    l.type = 0;

    Light l1{};
    l1.color = glm::vec3(1.0f, 0.0f, 0.0f);
    l1.position = glm::vec3(0.0f, 3.0f, 0.0f);
    l1.intensity = 10.0f;
    l1.type = 1;
    l1.radius = 5.0f;

    currentScene = LoadScene();
    currentScene->SetCamera(camera);
    currentScene->AddRenderable(std::move(plane));
    currentScene->AddRenderable(std::move(floor));
    currentScene->SetDirectionalLight(std::move(l));
    currentScene->AddLight(std::move(l1));
}

void Sandbox::OnDestroy()
{
    currentScene->OnDestroy();

    renderer->OnDestroy();
    renderer = nullptr;
}

void Sandbox::OnRender()
{
    renderer->OnRender(&swapchain, currentScene);
}

void Sandbox::OnResize()
{
    if (renderer && width && height)
    {
        renderer->OnDestroyDisplay();
        renderer->OnCreateDisplay(&swapchain, width, height);
    }
}

void Sandbox::OnUpdateDisplay()
{
    if (renderer)
    {
        renderer->OnUpdateDisplay(&swapchain);
    }
}

void Sandbox::OnUpdate()
{
    // Compute delta time
    Application::OnUpdate();

    currentScene->OnUpdate(static_cast<float>(deltaTime));

    // Input

    if (glfwGetKey(windowHandle, GLFW_KEY_LEFT_SHIFT))
        camera->speed = 3.0f;
    else
        camera->speed = 1.0f;

    if (glfwGetKey(windowHandle, GLFW_KEY_W))
    {
        camera->UpdateCameraWASD(camera->GetForward() * deltaTime);
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_A))
    {
        camera->UpdateCameraWASD(camera->GetRight() * deltaTime);
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_S))
    {
        camera->UpdateCameraWASD(-camera->GetForward() * deltaTime);
    }
    if (glfwGetKey(windowHandle, GLFW_KEY_D))
    {
        camera->UpdateCameraWASD(-camera->GetRight() * deltaTime);
    }
}

/*
 * MAIN
 */

int main()
{
    const char *gameName = "Pinut Sandbox";
    return Run(new Sandbox(std::move(gameName)));
}
