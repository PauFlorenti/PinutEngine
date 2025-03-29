#pragma once

#include "render_device/renderPipeline.h"
#include "render_device/textureFormat.h"

#include "pinut/renderer/shaders/draw_image_shader.h"
#include "pinut/renderer/shaders/quad_shader.h"

namespace Pinut
{
static const RED::RenderPipeline cDrawTexturePipeline = {"DrawTexturePipeline",
                                                         cQuadShader,
                                                         cDrawTextureShader,
                                                         "PosUv",
                                                         {RED::TextureFormat::R8G8B8A8_UNORM},
                                                         {}};

} // namespace Pinut
