#pragma once

#include "pinut/core/application.h"

class Sandbox final : public Pinut::Application
{
  public:
    explicit Sandbox(const std::string& name);

    void OnCreate() override;
    void OnDestroy() override {};
    void OnRender() override {};
    void OnUpdate() override;
};
