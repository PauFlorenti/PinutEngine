#pragma once

#include "render_device/shader.h"

namespace Pinut
{
static const auto cSkyboxVertexShader = RED::Shader(
  "skybox.vert",
  RED::ShaderType::VERTEX,
  {RED::UniformData::Create(RED::ShaderType::VERTEX, RED::UniformType::BUFFER, "skybox_data", 0)},
  {0});

static const auto cSkyboxFragShader =
  RED::Shader("skybox.frag",
              RED::ShaderType::FRAGMENT,
              {RED::UniformData::Create(RED::ShaderType::FRAGMENT,
                                        RED::UniformType::TEXTURE,
                                        "skybox_texture",
                                        1,
                                        1,
                                        1)},
              {0});
} // namespace Pinut
