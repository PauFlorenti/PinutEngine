// pch.h precompiled headers

#define WIN32_LEAN_AND_MEAN

#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <entt/entt.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

// Vulkan files
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include "pinut/defines.h"
#include "pinut/renderer/common.h"
