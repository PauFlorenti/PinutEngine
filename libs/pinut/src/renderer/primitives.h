#pragma once

namespace Pinut
{
class Device;
class Mesh;

namespace Primitives
{
static Mesh* CreateUnitPlane(Device* device);
static Mesh* CreateUnitCube(Device* device);

Mesh* GetUnitPlane();
Mesh* GetUnitCube();

void InitializeDefaultPrimitives(Device* device);
void DestroyDefaultPrimitives();

} // namespace Primitives
} // namespace Pinut
