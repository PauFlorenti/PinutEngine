#include "stdafx.h"

#include "primitives.h"
#include "src/assets/mesh.h"
#include "src/core/assetManager.h"

namespace Pinut
{
namespace Primitives
{
std::shared_ptr<Mesh> CreateUnitPlane(Device* device)
{
    assert(device);

    std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 1.0f}},
      {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, glm::vec4(1.0f), {0.0f, 0.0f}},
    };

    std::vector<uint16_t> indices = {0, 1, 3, 3, 1, 2};

    return Mesh::Create(device, std::move(vertices), std::move(indices));
}

std::shared_ptr<Mesh> CreateUnitCube(Device* device)
{
    assert(device);

    // clang-format off
    std::vector<Vertex> vertices = {
        //Top
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f)},  //0
        {glm::vec3( 0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f)},  //1
        {glm::vec3(-0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f)},  //2
        {glm::vec3( 0.5f, 0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f)},  //3

        //Bottom
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //4
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //5
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //6
        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //7

        //Front
        { glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //8
        { glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //9
        { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //10
        { glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //11

        //Back
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //12
        { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //13
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //14
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //15

        //Left
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //16
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //17
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //18
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.5f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }, //19

        //Right
        { glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 0.0f) }, //20
        { glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(0.0f, 1.0f) }, //21
        { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 0.0f) }, //22
        { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec4(1.0f), glm::vec2(1.0f, 1.0f) }  //23
    };

    std::vector<u16> indices = {
        //Top
        0, 1, 2,
        2, 3, 1,

        //Bottom
        4, 5, 6,
        6, 7, 5,

        //Front
        8, 9, 10,
        10, 11, 9,

        //Back
        12, 13, 14,
        14, 15, 13,

        //Left
        16, 17, 18,
        18, 19, 17,

        //Right
        20, 21, 22,
        22, 23, 21 };
    // clang-format on

    return Mesh::Create(device, std::move(vertices), std::move(indices));
}

void InitializeDefaultPrimitives(Device* device, AssetManager& assetManager)
{
    assert(device);

    assetManager.RegisterAsset("UnitPlane", CreateUnitPlane(device));
    assetManager.RegisterAsset("UnitCube", CreateUnitCube(device));
}
} // namespace Primitives
} // namespace Pinut
