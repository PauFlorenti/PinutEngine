#pragma once

#include "render_device/renderPipeline.h"
#include "render_device/textureFormat.h"

#include "pinut/renderer/shaders/skybox_shader.h"

namespace Pinut
{
static const RED::RenderPipeline cSkyboxPipeline = {"SkyboxPipeline",
                                                    cSkyboxVertexShader,
                                                    cSkyboxFragShader,
                                                    "PosColorUvN",
                                                    {RED::TextureFormat::R32G32B32A32_SFLOAT},
                                                    {}};

} // namespace Pinut
