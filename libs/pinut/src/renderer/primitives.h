#pragma once

namespace Pinut
{
class Mesh;

namespace Primitives
{
static void CreateUnitPlane();
static void CreateUnitCube();

std::shared_ptr<Mesh> GetUnitPlane();
std::shared_ptr<Mesh> GetUnitCube();

void InitializeDefaultPrimitives();
} // namespace Primitives
} // namespace Pinut
