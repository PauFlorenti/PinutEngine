#include "stdafx.h"

#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/core/renderable.h"

namespace Pinut
{
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
