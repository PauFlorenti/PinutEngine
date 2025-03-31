#include "pch.hpp"

#include "common/file.h"

namespace Common
{
    bool ExistFile(const std::string& InFilePath)
    {
        std::ifstream file(InFilePath);
        return file.good();
    }
}
