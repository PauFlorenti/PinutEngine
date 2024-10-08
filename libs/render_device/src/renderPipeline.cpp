#include "pch.hpp"

#include "src/renderPipeline.h"
#include "src/states.h"

bool RenderPipeline::operator==(const RenderPipeline& other) const noexcept
{
    return &vertexShader == &other.vertexShader && &fragmentShader == &other.fragmentShader;
}
