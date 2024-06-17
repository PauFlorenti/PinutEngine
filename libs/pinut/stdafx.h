// stdafx.h precompiled headers

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <cassert>
#include <chrono>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "src/core/vulkan_utils.h"