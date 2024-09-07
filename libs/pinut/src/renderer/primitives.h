#pragma once

namespace Pinut
{
class AssetManager;
class Mesh;
class Device;
namespace Primitives
{
static std::shared_ptr<Mesh> CreateUnitLine();
static std::shared_ptr<Mesh> CreateUnitPlane();
static std::shared_ptr<Mesh> CreateUnitCube();
static std::shared_ptr<Mesh> CreateUnitWiredCircle();

void InitializeDefaultPrimitives(Device* device, AssetManager& assetManager);

void DrawLine(VkCommandBuffer  cmd,
              VkPipelineLayout layout,
              const glm::vec3& origin,
              const glm::vec3& dest,
              const glm::vec3& color = glm::vec3(1.0f));
void DrawWiredCircle(VkCommandBuffer  cmd,
                     VkPipelineLayout layout,
                     const glm::mat4& transform,
                     const f32        radius = 1.0f,
                     const glm::vec3  color  = glm::vec3(1.0f));
void DrawWiredSphere(VkCommandBuffer  cmd,
                     VkPipelineLayout layout,
                     const glm::mat4  transform,
                     const f32        radius = 1.0f,
                     const glm::vec3  color  = glm::vec3(1.0f));
void DrawWiredCone(VkCommandBuffer  cmd,
                   VkPipelineLayout layout,
                   const glm::vec3& origin,
                   const glm::vec3& dest,
                   const f32        radius = 1.0f,
                   const glm::vec3& color  = glm::vec3(1.0f));
} // namespace Primitives
} // namespace Pinut
