#include "pch.hpp"

#include "pinut/assets/mesh.h"

namespace Pinut
{
Mesh::Mesh(const std::string& InName) : Asset(InName, AssetType::MESH) {};

Mesh::Mesh(std::vector<Vertex>    vertices,
           std::vector<u16>       indices,
           std::vector<Primitive> primitives,
           const std::string&     InName)
: Asset(InName, AssetType::MESH),
  m_vertices(std::move(vertices)),
  m_indices(std::move(indices)),
  m_primitives(std::move(primitives))
{
}

Mesh::~Mesh() = default;

bool Mesh::operator==(const Mesh& other) const
{
    // TODO Should we compare primitives? Probably if they hold the material ...
    return this->m_vertices == other.m_vertices && this->m_indices == other.m_indices;
}

bool Mesh::operator!=(const Mesh& other) const { return !(*this == other); }

void Mesh::Destroy()
{
    m_vertices.clear();
    m_indices.clear();
    m_primitives.clear();
}
} // namespace Pinut
