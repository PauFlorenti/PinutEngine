#pragma once

namespace Pinut
{
class AssetManager;
class Mesh;
class Device;
namespace Primitives
{
static std::shared_ptr<Mesh> CreateUnitPlane();
static std::shared_ptr<Mesh> CreateUnitCube();
static std::shared_ptr<Mesh> CreateUnitWiredCircle();

void InitializeDefaultPrimitives(Device* device, AssetManager& assetManager);

void DrawWiredCircle(VkCommandBuffer  cmd,
                     VkPipelineLayout layout,
                     const glm::mat4& transform,
                     const f32        radius,
                     const glm::vec3  color);
void DrawWiredSphere(VkCommandBuffer  cmd,
                     VkPipelineLayout layout,
                     const glm::mat4  transform,
                     const f32        radius = 1.0f,
                     const glm::vec3  color  = glm::vec3(1.0f));
} // namespace Primitives
} // namespace Pinut
