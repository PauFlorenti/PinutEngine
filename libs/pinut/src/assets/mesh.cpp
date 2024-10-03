#include "stdafx.h"

#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/core/renderable.h"

namespace Pinut
{
std::shared_ptr<Mesh> Mesh::Create(Device*             device,
                                   std::vector<Vertex> vertices,
                                   std::vector<u16>    indices)
{
    auto m        = std::make_shared<Mesh>();
    m->m_vertices = std::move(vertices);
    m->m_indices  = std::move(indices);

    Primitive prim;
    prim.m_vertexCount = static_cast<u32>(m->m_vertices.size());
    prim.m_indexCount  = static_cast<u32>(m->m_indices.size());
    prim.m_firstVertex = 0;
    prim.m_firstIndex  = 0;
    prim.m_material    = nullptr; // TODO

    m->m_primitives.push_back(prim);

    return m;
}

void Mesh::Destroy() { m_primitives.clear(); }
} // namespace Pinut
