// some comment
#include "stdafx.h"

#include "mesh.h"

#include "src/core/device.h"

namespace Pinut
{
	Mesh* Mesh::CreateMesh(std::vector<Vertex> inVertices, std::vector<uint16_t> inIndices)
	{
		Mesh* m = new Mesh();
		m->vertices = std::move(inVertices);
		m->indices = std::move(inIndices);

		return m;
	}

	void Mesh::Upload(Device* inDevice)
	{
		device = inDevice;
		assert(device);

		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint16_t);
		// Vertex buffer
		{
			auto bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			vertexBuffer.Create(device, vertexBufferSize, bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);
		}

		// Index buffer
		{
			auto bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			indexBuffer.Create(device, indexBufferSize, bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);
		}

		// Staging buffer
		{
			// Staging buffer
			Buffer stagingBuffer;
			stagingBuffer.Create(device, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

			// Copy data
			auto data = stagingBuffer.allocationInfo.pMappedData;
			memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
			memcpy((char*)data + vertices.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(uint16_t));

			VkBufferCopy vertexRegion{
				.srcOffset = 0,
				.dstOffset = 0,
				.size = vertexBufferSize,
			};

			VkBufferCopy indexRegion{
				.srcOffset = vertexBufferSize,
				.dstOffset = 0,
				.size = indexBufferSize,
			};

			auto cmd = device->GetSingleUseCommandBuffer();

			vkCmdCopyBuffer(cmd, stagingBuffer.buffer, vertexBuffer.buffer, 1, &vertexRegion);
			vkCmdCopyBuffer(cmd, stagingBuffer.buffer, indexBuffer.buffer, 1, &indexRegion);

			device->EndSingleUseCommandBuffer(cmd);

			vmaDestroyBuffer(device->GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
		}
	}

	void Mesh::Destroy()
	{
		assert(device);
		vmaDestroyBuffer(device->GetAllocator(), vertexBuffer.buffer, vertexBuffer.allocation);
		vmaDestroyBuffer(device->GetAllocator(), indexBuffer.buffer, indexBuffer.allocation);
		vertices.clear();
		indices.clear();
	}
}
