#include "stdafx.h"

#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/assetManager.h"
#include "src/core/node.h"
#include "src/core/renderable.h"
#include "src/loaders/gltfLoader.h"

namespace Pinut
{
glm::mat4 GetLocalMatrix(const tinygltf::Node& inputNode)
{
    glm::mat4 matrix = glm::mat4(1);
    if (inputNode.translation.size() == 3)
        matrix = glm::translate(matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    if (inputNode.rotation.size() == 4)
    {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        matrix *= glm::mat4(q);
    }
    if (inputNode.scale.size() == 3)
        matrix = glm::scale(matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    if (inputNode.matrix.size() == 16)
        matrix = glm::make_mat4x4(inputNode.matrix.data());

    return matrix;
}

GLTFLoader::GLTFLoader() = default;

void GLTFLoader::Init(Device* device, VkDescriptorSetLayout layout)
{
    assert(device);
    m_device = device;
    m_layout = layout;
}

std::shared_ptr<Renderable> GLTFLoader::LoadFromFile(const std::filesystem::path& filepath,
                                                     AssetManager&                assetManager)
{
    assert(!filepath.empty());
    assert(filepath.has_extension());

    const auto extension = filepath.extension();
    if (extension != ".glb" && extension != ".gltf")
    {
        printf("[ERROR]: No valid extension for a .gltf file type.");
        return nullptr;
    }

    bool binary = extension == ".glb" ? true : false;

    tinygltf::Model    gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string        warn, err;
    bool               fileLoaded{false};

    fileLoaded = binary ?
                   gltfContext.LoadBinaryFromFile(&gltfModel, &err, &warn, filepath.string()) :
                   gltfContext.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath.string());

    if (!fileLoaded)
    {
        printf("[ERROR]: Error loading GLTF file.");
        return nullptr;
    }

    if (!err.empty())
    {
        printf("[ERROR]: Error loading gltf. '%s'", err.c_str());
        return nullptr;
    }

    if (!warn.empty())
    {
        printf("[WARN]: Warning loading gltf file. '%s'", warn.c_str());
    }

    auto root = LoadNode(gltfModel, gltfModel.nodes.at(0), nullptr, assetManager);
    return std::make_shared<Renderable>(std::move(root));
}

std::shared_ptr<Node> GLTFLoader::LoadNode(const tinygltf::Model& tmodel,
                                           const tinygltf::Node&  tnode,
                                           std::shared_ptr<Node>  parent,
                                           AssetManager&          assetManager,
                                           const bool             invertNormals)
{
    auto node = std::make_shared<Node>();
    node->SetMatrix(GetLocalMatrix(tnode));

    if (!tnode.children.empty())
    {
        for (const auto& childIndex : tnode.children)
        {
            LoadNode(tmodel, tmodel.nodes.at(childIndex), node, assetManager);
        }
    }

    if (tnode.mesh > -1)
    {
        const tinygltf::Mesh mesh = tmodel.meshes[tnode.mesh];

        if (const auto foundMesh = assetManager.GetAsset<Mesh>(mesh.name))
        {
            node->SetMesh(std::move(foundMesh));
        }
        else
        {
            auto m = std::make_shared<Mesh>();
            // Iterate through all primitives in mesh
            for (size_t i = 0; i < mesh.primitives.size(); i++)
            {
                const tinygltf::Primitive& tprimitive  = mesh.primitives[i];
                u32                        firstIndex  = static_cast<u32>(m->m_indices.size());
                u32                        firstVertex = static_cast<u32>(m->m_vertices.size());
                u32                        indexCount  = 0;
                u32                        vertexCount = 0;

                //Vertices
                {
                    const float* positionBuffer  = nullptr;
                    const float* normalsBuffer   = nullptr;
                    const float* texCoordsBuffer = nullptr;

                    // Get buffer data for vertex position
                    if (tprimitive.attributes.find("POSITION") != tprimitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor =
                          tmodel.accessors[tprimitive.attributes.find("POSITION")->second];
                        const tinygltf::BufferView& view = tmodel.bufferViews[accessor.bufferView];
                        positionBuffer                   = reinterpret_cast<const float*>(&(
                          tmodel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        vertexCount                      = static_cast<u32>(accessor.count);
                    }
                    // Get buffer data for vertex normals
                    if (tprimitive.attributes.find("NORMAL") != tprimitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor =
                          tmodel.accessors[tprimitive.attributes.find("NORMAL")->second];
                        const tinygltf::BufferView& view = tmodel.bufferViews[accessor.bufferView];
                        normalsBuffer                    = reinterpret_cast<const float*>(&(
                          tmodel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                    // Get buffer data for vertex texture coordinates
                    // glTF supports multiple sets, we only load the first one
                    if (tprimitive.attributes.find("TEXCOORD_0") != tprimitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor =
                          tmodel.accessors[tprimitive.attributes.find("TEXCOORD_0")->second];
                        const tinygltf::BufferView& view = tmodel.bufferViews[accessor.bufferView];
                        texCoordsBuffer                  = reinterpret_cast<const float*>(&(
                          tmodel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }

                    // Append data to model's vertex buffer
                    for (size_t v = 0; v < vertexCount; v++)
                    {
                        glm::vec3 normal = glm::normalize(glm::vec3(
                          normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                        Vertex    vert{};
                        vert.position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                        vert.normal   = invertNormals ? normal * glm::vec3(-1) : normal;
                        vert.uv       = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) :
                                                          glm::vec3(0.0f);
                        vert.color    = glm::vec4(1.0f);
                        m->m_vertices.push_back(vert);
                    }
                }
                //Indices
                {
                    const tinygltf::Accessor&   accessor = tmodel.accessors[tprimitive.indices];
                    const tinygltf::BufferView& bufferView =
                      tmodel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = tmodel.buffers[bufferView.buffer];

                    indexCount += static_cast<uint32_t>(accessor.count);

                    // glTF supports different component types of indices
                    switch (accessor.componentType)
                    {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            uint32_t* buf = new uint32_t[accessor.count];
                            memcpy(buf,
                                   &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint32_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                m->m_indices.push_back(buf[index] + firstVertex);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            uint16_t* buf = new uint16_t[accessor.count];
                            memcpy(buf,
                                   &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint16_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                m->m_indices.push_back(buf[index] + firstVertex);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            uint8_t* buf = new uint8_t[accessor.count];
                            memcpy(buf,
                                   &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                                   accessor.count * sizeof(uint8_t));
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                m->m_indices.push_back(buf[index] + firstVertex);
                            }
                            break;
                        }
                        default:
                            printf("Index component type '%i' not supported!",
                                   accessor.componentType);
                            return nullptr;
                    }
                }

                if (tprimitive.material > -1)
                {
                    const auto& tmat = tmodel.materials[tprimitive.material];
                    const auto& tpbr = tmat.pbrMetallicRoughness;

                    MaterialData data{};
                    data.diffuse = (u8)tpbr.baseColorFactor[0] * 255 << 16 |
                                   (u8)tpbr.baseColorFactor[1] * 255 << 8 |
                                   (u8)tpbr.baseColorFactor[2] * 255 << 0 |
                                   (u8)tpbr.baseColorFactor[3] * 255 << 24;
                    data.diffuseTexture = tpbr.baseColorTexture.index > -1 ?
                                            assetManager.GetAsset<Texture>(
                                              tmodel.images.at(tpbr.baseColorTexture.index).uri) :
                                            assetManager.GetAsset<Texture>("");

                    assetManager.CreateMaterial(tmat.name, m_layout, std::move(data));
                }

                Primitive prim;
                prim.m_firstIndex  = firstIndex;
                prim.m_indexCount  = indexCount;
                prim.m_firstVertex = firstVertex;
                prim.m_vertexCount = vertexCount;
                m->m_primitives.push_back(prim);
            }

            m->Upload(m_device);
            assetManager.RegisterAsset(mesh.name, m);
            node->SetMesh(std::move(m));
        }
    }

    return node;
}
} // namespace Pinut