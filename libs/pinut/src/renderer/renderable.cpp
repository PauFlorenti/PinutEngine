#include "stdafx.h"

#include <imgui.h>

#include "renderable.h"
#include "src/assets/mesh.h"
#include "src/renderer/buffer.h"

namespace Pinut
{
Renderable::Renderable() {}

const glm::mat4 Renderable::Model() const
{
    return glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_rotation) *
           glm::scale(glm::mat4(1.0f), m_scale);
}

void Renderable::SetModel(glm::mat4 model)
{
    glm::vec3 skew{};
    glm::vec4 perspective{};
    auto      ok = glm::decompose(model, m_scale, m_rotation, m_position, skew, perspective);
    assert(ok == true);
}

void Renderable::Draw(VkCommandBuffer cmd) const
{
    VkDeviceSize offset{0};

    if (m_mesh->m_indexBuffer.m_buffer != VK_NULL_HANDLE && m_mesh->GetIndexCount() > 0)
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_mesh->m_vertexBuffer.m_buffer, &offset);
        vkCmdBindIndexBuffer(cmd, m_mesh->m_indexBuffer.m_buffer, offset, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(cmd, m_mesh->GetIndexCount(), 1, 0, 0, m_instanceIndex);
    }
    else
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &m_mesh->m_vertexBuffer.m_buffer, &offset);
        vkCmdDraw(cmd, m_mesh->GetVertexCount(), 1, 0, m_instanceIndex);
    }
}

void Renderable::DrawImGui()
{
    if (ImGui::TreeNode("Entity Name"))
    {
        ImGui::DragFloat3("Position", &m_position.x);
        ImGui::DragFloat3("Scale", &m_scale.x);
        ImGui::DragFloat4("Rotation", &m_rotation[0]);

        ImGui::TreePop();
    }
}

void Renderable::Destroy() {}

bool Renderable::IsValid() const { return m_mesh != nullptr; }

} // namespace Pinut
