#include "stdafx.h"

#include "src/assets/material.h"
#include "src/assets/mesh.h"
#include "src/core/renderable.h"
#include "src/renderer/device.h"

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
    m->Upload(device);

    return m;
}

void Mesh::Destroy()
{
    m_primitives.clear();
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
}

void Mesh::Upload(Device* device)
{
    assert(device != nullptr);
    assert(!m_vertices.empty());

    const u64 vertexBufferSize = m_vertices.size() * sizeof(Vertex);
    const u64 indexBufferSize  = m_indices.size() * sizeof(u16);

    m_vertexBuffer.Create(device,
                          vertexBufferSize,
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    m_indexBuffer.Create(device,
                         indexBufferSize,
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    GPUBuffer stagingBuffer;
    stagingBuffer.Create(device,
                         vertexBufferSize + indexBufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

    auto stagingData = static_cast<u8*>(stagingBuffer.AllocationInfo().pMappedData);
    memset(stagingData, 0, vertexBufferSize + indexBufferSize);
    memcpy(stagingData, m_vertices.data(), vertexBufferSize);
    memcpy(stagingData + vertexBufferSize, m_indices.data(), indexBufferSize);

    VkBufferCopy vertexRegion{
      .srcOffset = 0,
      .dstOffset = 0,
      .size      = vertexBufferSize,
    };

    VkBufferCopy indexRegion{
      .srcOffset = vertexBufferSize,
      .dstOffset = 0,
      .size      = indexBufferSize,
    };

    auto cmd = device->CreateImmediateCommandBuffer();
    vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_vertexBuffer.m_buffer, 1, &vertexRegion);
    vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m_indexBuffer.m_buffer, 1, &indexRegion);
    device->FlushCommandBuffer(cmd);

    stagingBuffer.Destroy();
}

void Mesh::Draw(VkCommandBuffer cmd, VkPipelineLayout layout) const
{
    for (const auto& primitive : m_primitives)
    {
        VkDeviceSize vertexOffset = primitive.m_firstVertex;
        VkDeviceSize indexOffset  = primitive.m_firstIndex;

        if (primitive.m_material)
            primitive.m_material->Bind(cmd, layout);

        if (m_indexBuffer.m_buffer != VK_NULL_HANDLE && primitive.m_indexCount > 0)
        {
            vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer.m_buffer, &vertexOffset);
            vkCmdBindIndexBuffer(cmd, m_indexBuffer.m_buffer, indexOffset, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(cmd, primitive.m_indexCount, 1, 0, 0, 0);
        }
        else
        {
            vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer.m_buffer, &vertexOffset);
            vkCmdDraw(cmd, primitive.m_vertexCount, 1, 0, 0);
        }
    }
}
} // namespace Pinut
