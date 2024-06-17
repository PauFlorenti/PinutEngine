#pragma once

namespace Pinut
{
    class Texture;

    class Material
    {
    public:
        void Destroy();

        void SetAlbedo(Texture *t) { albedo = t; }
        void SetNormal(Texture *t) { normal = t; }
        void SetRoughness(Texture *t) { metallicRoughness = t; }
        Texture *GetAlbedo() const { return albedo; }
        Texture *GetNormal() const { return normal; }
        Texture *GetRoughness() const { return metallicRoughness; }

    private:
        glm::vec4 color = glm::vec4(1.0f);
        Texture *albedo{nullptr};
        Texture *normal{nullptr};
        Texture *metallicRoughness{nullptr};
        Texture *ambientOcclusion{nullptr};
    };
}
