//#include "pch.hpp"
//
//#include "primitives.h"
//#include "pinut/assets/mesh.h"
//#include "pinut/core/assetManager.h"
//
//namespace Pinut
//{
//namespace Primitives
//{
//std::shared_ptr<Mesh> line        = nullptr;
//std::shared_ptr<Mesh> wiredCircle = nullptr;
//std::shared_ptr<Mesh> wiredSphere = nullptr;
//
//std::shared_ptr<Mesh> CreateUnitLine(Device* device)
//{
//    std::vector<Vertex> vertices = {
//      {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec4(1.0f), glm::vec2(0.0f)},
//      {{0.0f, 0.0f, -1.0f}, glm::vec3(0.0f), glm::vec4(1.0f), glm::vec2(0.0f)}};
//
//    std::vector<u16> indices = {0, 1};
//
//    line = Mesh::Create(device, std::move(vertices), std::move(indices));
//    return line;
//}
//
//std::shared_ptr<Mesh> CreateUnitPlane(Device* device)
//{
//    assert(device);
//
//    std::vector<Vertex> vertices = {
//      {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 1.0f}},
//      {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 1.0f}},
//      {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 0.0f}},
//      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 0.0f}},
//    };
//
//    std::vector<uint16_t> indices = {0, 1, 3, 3, 1, 2};
//
//    return Mesh::Create(device, std::move(vertices), std::move(indices));
//}
//
//std::shared_ptr<Mesh> CreateUnitCube(Device* device)
//{
//    assert(device);
//
//    // clang-format off
//    std::vector<Vertex> vertices = {
//        //Top
//        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f)},  //0
//        {glm::vec3( 0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f)},  //1
//        {glm::vec3(-0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f)},  //2
//        {glm::vec3( 0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f)},  //3
//
//        //Bottom
//        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //4
//        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //5
//        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //6
//        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //7
//
//        //Front
//        { glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //8
//        { glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //9
//        { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //10
//        { glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //11
//
//        //Back
//        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //12
//        { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //13
//        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //14
//        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //15
//
//        //Left
//        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //16
//        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //17
//        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //18
//        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //19
//
//        //Right
//        { glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //20
//        { glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //21
//        { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //22
//        { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }  //23
//    };
//
//    std::vector<u16> indices = {
//        //Top
//        0, 1, 2,
//        2, 3, 1,
//
//        //Bottom
//        4, 5, 6,
//        6, 7, 5,
//
//        //Front
//        8, 9, 10,
//        10, 11, 9,
//
//        //Back
//        12, 13, 14,
//        14, 15, 13,
//
//        //Left
//        16, 17, 18,
//        18, 19, 17,
//
//        //Right
//        20, 21, 22,
//        22, 23, 21 };
//    // clang-format on
//
//    return Mesh::Create(device, std::move(vertices), std::move(indices));
//}
//
//std::shared_ptr<Mesh> CreateUnitWiredCircle(Device* device)
//{
//    assert(device);
//    const i32           samples = 32;
//    std::vector<Vertex> vertices(samples);
//    std::vector<u16>    indices;
//
//    for (u16 i = 0; i < samples; i++)
//    {
//        f32    angle = 2.0f * glm::pi<f32>() * static_cast<f32>(i) / static_cast<f32>(samples);
//        Vertex v{};
//        v.position = glm::vec3(sinf(angle), 0.0f, cosf(angle));
//        v.color    = glm::vec4(1.0f);
//
//        vertices[i] = v;
//        indices.push_back(i);
//    }
//
//    indices.push_back(0);
//
//    wiredCircle = Mesh::Create(device, std::move(vertices), std::move(indices));
//    return wiredCircle;
//}
//
//void InitializeDefaultPrimitives(Device* device, AssetManager& assetManager)
//{
//    assert(device);
//
//    assetManager.RegisterAsset("UnitLine", CreateUnitLine(device));
//    assetManager.RegisterAsset("UnitPlane", CreateUnitPlane(device));
//    assetManager.RegisterAsset("UnitCube", CreateUnitCube(device));
//    assetManager.RegisterAsset("UnitWiredCircle", CreateUnitWiredCircle(device));
//}
//
//void DrawLine(VkCommandBuffer  cmd,
//              VkPipelineLayout layout,
//              const glm::vec3& origin,
//              const glm::vec3& dest,
//              const glm::vec3& color)
//{
//    auto       dst      = dest;
//    const auto dir      = dest - origin;
//    const f32  distance = glm::length(dir);
//
//    if (distance < 0.001f)
//        return;
//
//    // Make sure it does not align with up(0, 1, 0).
//    if (glm::abs(dest.x) < 0.001f || glm::abs(dest.z) < 0.001f)
//    {
//        dst.x += 0.001f;
//    }
//
//    const auto model = glm::scale(
//      glm::inverse(glm::lookAt(origin, origin + glm::normalize(dir), glm::vec3(0.0f, 1.0f, 0.0f))),
//      glm::vec3(distance));
//    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
//    vkCmdPushConstants(cmd,
//                       layout,
//                       VK_SHADER_STAGE_VERTEX_BIT,
//                       sizeof(glm::mat4),
//                       sizeof(glm::vec3),
//                       &color);
//
//    line->Draw(cmd, layout);
//}
//
//void DrawWiredCircle(VkCommandBuffer  cmd,
//                     VkPipelineLayout layout,
//                     const glm::mat4& transform,
//                     const f32        radius,
//                     const glm::vec3  color)
//{
//    const auto model = glm::scale(transform, glm::vec3(radius));
//    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
//    vkCmdPushConstants(cmd,
//                       layout,
//                       VK_SHADER_STAGE_VERTEX_BIT,
//                       sizeof(glm::mat4),
//                       sizeof(glm::vec3),
//                       &color);
//
//    wiredCircle->Draw(cmd, layout);
//}
//
//void DrawWiredSphere(VkCommandBuffer  cmd,
//                     VkPipelineLayout layout,
//                     const glm::mat4  transform,
//                     const f32        radius,
//                     const glm::vec3  color)
//{
//    const auto draw = [&](glm::mat4 rotation)
//    {
//        const auto model = glm::scale(transform * rotation, glm::vec3(radius));
//        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
//        vkCmdPushConstants(cmd,
//                           layout,
//                           VK_SHADER_STAGE_VERTEX_BIT,
//                           sizeof(glm::mat4),
//                           sizeof(glm::vec3),
//                           &color);
//
//        wiredCircle->Draw(cmd, layout);
//    };
//
//    draw(glm::mat4(1.0f));
//    draw(glm::rotate(glm::mat4(1.0f), glm::pi<f32>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)));
//    draw(glm::rotate(glm::mat4(1.0f), glm::pi<f32>() * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f)));
//}
//
//void DrawWiredCone(VkCommandBuffer  cmd,
//                   VkPipelineLayout layout,
//                   const glm::vec3& origin,
//                   const glm::vec3& dest,
//                   const f32        radius, // The radius of the base of the cone.
//                   const glm::vec3& color)
//{
//    const auto dir      = dest - origin;
//    const auto distance = glm::length(dir);
//
//    if (distance < 0.001f)
//        return;
//
//    const auto model = glm::scale(
//      glm::inverse(glm::lookAt(dest, dest + glm::normalize(dir), glm::vec3(0.0f, 1.0f, 0.0f))) *
//        glm::rotate(glm::pi<f32>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)),
//      glm::vec3(radius));
//    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
//    vkCmdPushConstants(cmd,
//                       layout,
//                       VK_SHADER_STAGE_VERTEX_BIT,
//                       sizeof(glm::mat4),
//                       sizeof(glm::vec3),
//                       &color);
//
//    wiredCircle->Draw(cmd, layout);
//
//    u16 samples = 16;
//    for (u16 i = 0; i < samples; i++)
//    {
//        f32        angle = 2.0f * glm::pi<f32>() * static_cast<f32>(i) / static_cast<f32>(samples);
//        const auto position = model * glm::vec4(sinf(angle), 0.0f, cosf(angle), 1.0f);
//        DrawLine(cmd, layout, origin, position, color);
//    }
//}
//} // namespace Primitives
//} // namespace Pinut
