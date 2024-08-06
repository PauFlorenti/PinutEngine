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

void InitializeDefaultPrimitives(Device* device, AssetManager& assetManager);
} // namespace Primitives
} // namespace Pinut
