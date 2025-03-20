#pragma once

namespace tinyobj
{
struct attrib_t;
struct shape_t;
} // namespace tinyobj
namespace Pinut
{
class Material;
class Mesh;
class OBJLoader final
{
  public:
    OBJLoader();
    ~OBJLoader();

    std::shared_ptr<Mesh> ParseObj(const std::filesystem::path& InFilepath);

  private:
    std::shared_ptr<Mesh> ParseMesh(const std::string&                     InFilename,
                                    const tinyobj::attrib_t&               InAttrib,
                                    const std::vector<tinyobj::shape_t>&   InShapes,
                                    std::vector<std::shared_ptr<Material>> InMaterials);
};
} // namespace Pinut
