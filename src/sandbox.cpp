#include "stdafx.h"

#include "sandbox.h"

int main()
{
    Run(new Sandbox("Sandbox application"));
    return 0;
}

Sandbox::Sandbox(const std::string& name) : Pinut::Application(std::move(name)){};
