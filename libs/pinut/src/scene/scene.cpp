// Some comment
#include "stdafx.h"

#include "scene.h"

#include "glm/gtc/matrix_transform.hpp"

#include "material.h"
#include "src/misc/camera.h"

namespace Pinut
{
    Mesh *CreateUnitPlane()
    {
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 0.0f}},
        };

        std::vector<uint16_t> indices = {
            0, 1, 3, 3, 1, 2};

        return Mesh::CreateMesh(std::move(vertices), std::move(indices));
    }

    Mesh *CreateUnitCube()
    {
        return nullptr;
    }

    void Scene::Renderable::OnDestroy()
    {
        objectBuffer.Destroy();

        if (mesh)
        {
            mesh->Destroy();
            mesh = nullptr;
        }

        if (material)
        {
            material->Destroy();
            material = nullptr;
        }
    }

    void Scene::Renderable::Draw(VkCommandBuffer cmd)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->GetVertexBuffer().buffer, &offset);
        vkCmdBindIndexBuffer(cmd, mesh->GetIndexBuffer().buffer, offset, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cmd, mesh->GetIndicesSize(), 1, 0, 0, 0);
    }

    void Scene::Renderable::Update(float deltaTime)
    {
        time += deltaTime;
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, glm::sin(time) * 0.01f));
        //model = glm::rotate(glm::mat4(1), 2 * time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void Scene::Renderable::Upload(Device *device)
    {
        objectBuffer.Create(device, sizeof(PerObjectData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        mesh->Upload(device);
    }

    void Scene::OnUpdate(float deltaTime)
    {
        for (auto r : renderables)
        {
            r->Update(deltaTime);
        }
    }

    void Scene::OnDestroy()
    {
        for (auto &r : renderables)
        {
            r->OnDestroy();
        }
    }

    void Scene::AddRenderable(Renderable *r)
    {
        renderables.push_back(r);
    }

    void Scene::RemoveRenderable(Renderable *r)
    {
        for (auto it = renderables.begin(); it != renderables.end(); ++it)
        {
            auto value = *it;
            if (*value == *r)
            {
                renderables.erase(it);
                return;
            }
        }
    }

    void Scene::SetCamera(Camera *inCamera)
    {
        assert(inCamera);
        camera = inCamera;
    }
}
