#pragma once

#include "render_device/shader.h"

namespace Pinut
{
static const auto cDrawTextureShader =
  RED::Shader("draw_texture.frag",
              RED::ShaderType::FRAGMENT,
              {RED::UniformData::Create(RED::ShaderType::FRAGMENT,
                                        RED::UniformType::TEXTURE,
                                        "offscreen_texture",
                                        0)},
              {0});
} // namespace Pinut
