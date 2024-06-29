#include "stdafx.h"

#include "device.h"
#include "mesh.h"

namespace Pinut
{
Mesh* Mesh::Create(Device* device, std::vector<Vertex> vertices, std::vector<u16> indices)
{
    assert(device);
    auto m = new Mesh();

    m->m_vertexCount = static_cast<u32>(vertices.size());
    m->m_indexCount  = static_cast<u32>(indices.size());

    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize  = indices.size() * sizeof(uint32_t);

    // Vertex buffer
    {
        auto bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m->m_vertexBuffer.Create(device,
                                 vertexBufferSize,
                                 std::move(bufferUsage),
                                 VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    }

    // Index buffer
    {
        auto bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        m->m_indexBuffer.Create(device,
                                indexBufferSize,
                                std::move(bufferUsage),
                                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    }

    // Staging buffer
    {
        GPUBuffer stagingBuffer;
        stagingBuffer.Create(device,
                             vertexBufferSize + indexBufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

        // Copy data
        auto data = stagingBuffer.AllocationInfo().pMappedData;
        memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
        memcpy((char*)data + vertices.size() * sizeof(Vertex),
               indices.data(),
               indices.size() * sizeof(uint32_t));

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

        auto cmd = device->CreateCommandBuffer();

        vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m->m_vertexBuffer.m_buffer, 1, &vertexRegion);
        vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m->m_indexBuffer.m_buffer, 1, &indexRegion);

        device->FlushCommandBuffer(cmd);

        stagingBuffer.Destroy();
    }

    return m;
}

void Mesh::Destroy()
{
    m_vertexBuffer.Destroy();
    if (m_indexCount > 0)
        m_indexBuffer.Destroy();
}

const u32& Mesh::GetVertexCount() const { return m_vertexCount; }
const u32& Mesh::GetIndexCount() const { return m_indexCount; }
} // namespace Pinut
