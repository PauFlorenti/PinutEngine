#pragma once

namespace Pinut
{
    static const uint32_t MaxLights = 10;
    static const uint32_t DirectionalLightType = 0;
    static const uint32_t PointLightType = 1;
    static const uint32_t SpotLightType = 2;

    struct Light
    {
        glm::vec3 color = glm::vec3(0.0f);
        float intensity{0.0f};
        glm::vec3 position = glm::vec3(0.0f);
        uint32_t type{0};
        float radius{0.0f};
    };

    struct PerFrameData
    {
        glm::mat4 view;
        glm::mat4 projection;
        //glm::vec3 cameraPosition;
        // float padding;

        // Light directionalLight;
        //Light lights[MaxLights];
    };

    struct PerObjectData
    {
        glm::mat4 model;
        glm::mat4 inverse_model;
    };
}
