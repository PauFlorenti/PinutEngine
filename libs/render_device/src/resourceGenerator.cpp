#include "render_device/resourceGenerator.h"

namespace RED
{
BufferResource ResourceGenerator::GenerateBufferResource()
{
    return {m_buffer_id++, ResourceType::BUFFER};
}

TextureResource ResourceGenerator::GenerateTextureResource()
{
    return {m_texture_id++, ResourceType::TEXTURE};
}
} // namespace RED
