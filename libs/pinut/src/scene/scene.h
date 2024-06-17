#pragma once

#include "mesh.h"
#include "src/core/common.h"

namespace Pinut
{
    class Camera;
    class Device;
    class Mesh;
    class Material;

    Mesh *CreateUnitPlane();
    Mesh *CreateUnitCube();

    class Scene
    {
    public:
        class Renderable
        {
        public:
            inline bool operator==(const Renderable &other) { return *mesh == *other.mesh; }
            inline bool operator!=(const Renderable &other) { return !(*this == other); }

            void OnDestroy();
            void Draw(VkCommandBuffer cmd);
            void Update(float deltaTime);
            void Upload(Device *device);

            const Mesh *GetMesh() const { return mesh; }
            const Material *GetMaterial() const { return material; }
            const Buffer &GetBuffer() const { return objectBuffer; }
            VkDescriptorSet GetDescriptorSet() { return descriptorSet; }
            const glm::mat4 &GetModelMatrix() const { return model; }

            void SetMesh(Mesh *m)
            {
                assert(m);
                mesh = m;
            }

            void SetMaterial(Material *m)
            {
                assert(m);
                material = m;
            }

            glm::mat4 model = glm::mat4(1.0f);

        private:
            Mesh *mesh{nullptr};
            Material *material{nullptr};
            Buffer objectBuffer;
            VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
            float time{0};
        };

        void OnUpdate(float deltaTime);
        void OnDestroy();

        void AddRenderable(Renderable *r);
        void RemoveRenderable(Renderable *r);

        const std::vector<Renderable *> &GetRenderables() const { return renderables; }
        const Light &GetDirectionalLight() const { return directionalLight; }
        const Light *GetLights() const { return lights; }
        const Camera* GetCamera() const { return camera; }

        void SetDirectionalLight(Light l) { directionalLight = std::move(l); }
        void SetCamera(Camera *inCamera);

        void AddLight(Light l)
        {
            if (lightsCounter < MaxLights)
                lights[lightsCounter++] = std::move(l);
        }

    private:
        std::vector<Renderable *> renderables;
        Camera *camera{nullptr};
        Light directionalLight;
        Light lights[MaxLights];
        uint32_t lightsCounter{0};
        VkDescriptorPool descriptorSetPool{VK_NULL_HANDLE};
    };
}
