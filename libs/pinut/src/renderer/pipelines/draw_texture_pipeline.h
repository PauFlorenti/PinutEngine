#pragma once

#include "render_device/renderPipeline.h"

#include "src/renderer/shaders/draw_image_shader.h"
#include "src/renderer/shaders/quad_shader.h"

namespace Pinut
{
static const RED::RenderPipeline cDrawTexturePipeline =
  {"DrawTexturePipeline", cQuadShader, cDrawTextureShader, "PosUv", {VK_FORMAT_B8G8R8A8_UNORM}, {}};

} // namespace Pinut
