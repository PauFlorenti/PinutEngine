#pragma once

namespace tinyobj
{
struct attrib_t;
struct shape_t;
} // namespace tinyobj
namespace Pinut
{
struct RawData;
class OBJLoader final
{
  public:
    OBJLoader();
    ~OBJLoader();

    RawData ParseObjFile(const std::filesystem::path& InFilepath);

  private:
    void ParseMesh(const std::string&                   InFilename,
                   const tinyobj::attrib_t&             InAttrib,
                   const std::vector<tinyobj::shape_t>& InShapes,
                   RawData&                             OutRawData);
};
} // namespace Pinut
