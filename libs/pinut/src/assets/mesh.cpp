#include "stdafx.h"

#include "mesh.h"
#include "src/renderer/device.h"
#include "src/renderer/renderable.h"

namespace Pinut
{

void Mesh::DrawCall::Draw(VkCommandBuffer cmd) const
{
    VkDeviceSize offset = 0;
    if (m_indexBuffer->m_buffer != VK_NULL_HANDLE && m_indexCount > 0)
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer->m_buffer, &offset);
        vkCmdBindIndexBuffer(cmd, m_indexBuffer->m_buffer, m_indexOffset, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, m_owner->InstanceIndex());
    }
    else
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer->m_buffer, &m_vertexOffset);
        vkCmdDraw(cmd, m_vertexCount, 1, 0, m_owner->InstanceIndex());
    }
}

std::shared_ptr<Mesh> Mesh::Create(Device*             device,
                                   std::vector<Vertex> vertices,
                                   std::vector<u16>    indices)
{
    auto m = std::make_shared<Mesh>();

    DrawCall dc;
    dc.m_vertexCount  = static_cast<u32>(vertices.size());
    dc.m_indexCount   = static_cast<u32>(indices.size());
    dc.m_vertexOffset = 0;
    dc.m_indexOffset  = 0;

    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize  = indices.size() * sizeof(u16);

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
               indices.size() * sizeof(u16));

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

        vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m->m_vertexBuffer.m_buffer, 1, &vertexRegion);
        vkCmdCopyBuffer(cmd, stagingBuffer.m_buffer, m->m_indexBuffer.m_buffer, 1, &indexRegion);

        device->FlushCommandBuffer(cmd);

        stagingBuffer.Destroy();
    }

    dc.m_vertexBuffer = std::make_shared<GPUBuffer>(m->m_vertexBuffer);
    dc.m_indexBuffer  = std::make_shared<GPUBuffer>(m->m_indexBuffer);
    dc.m_material     = nullptr; // TODO
    m->m_drawCalls.push_back(dc);

    return m;
}

void Mesh::Destroy()
{
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
    m_drawCalls.clear();
}

const std::vector<std::shared_ptr<MaterialInstance>> Mesh::Materials() const
{
    std::vector<std::shared_ptr<MaterialInstance>> materials;
    for (const auto& dc : m_drawCalls)
    {
        if (dc.m_material)
            materials.push_back(dc.m_material);
    }

    return materials;
}

void Mesh::SetMaterial(std::shared_ptr<MaterialInstance> material, u32 slot)
{
    assert(m_drawCalls.size() > slot);
    m_drawCalls.at(slot).m_material = std::move(material);
}
} // namespace Pinut
