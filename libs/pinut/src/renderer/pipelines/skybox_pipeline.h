#pragma once

#include "render_device/renderPipeline.h"

#include "src/renderer/shaders/skybox_shader.h"

namespace Pinut
{
static const RED::RenderPipeline cSkyboxPipeline = {"SkyboxPipeline",
                                                    cSkyboxVertexShader,
                                                    cSkyboxFragShader,
                                                    "PosColorUvN",
                                                    {VK_FORMAT_R32G32B32A32_SFLOAT},
                                                    {}};

} // namespace Pinut
