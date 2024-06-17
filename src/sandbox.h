#pragma once

#include "src/core/application.h"

class Renderer;

namespace Pinut
{
    class Camera;
    class Scene;
}

class Sandbox : public Pinut::Application
{
public:
    Sandbox(const char *name);

    void OnCreate() override;
    void OnDestroy() override;
    void OnRender() override;
    void OnResize() override;
    void OnUpdateDisplay() override;

    void OnUpdate() override;

private:
    Renderer *renderer{nullptr};
    Pinut::Scene *currentScene{nullptr};
    Pinut::Camera *camera{nullptr};
};
