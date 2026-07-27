#pragma once

#include "Common.h"
#include "CustomizationUI.h"




namespace Config
{
    struct RuntimeSettings
    {
        bool particlesEnabled = true;
        float particleSpeed = 22.0f;
        float particleAmount = 34.0f;
    };

    std::filesystem::path Directory();
    std::wstring CleanName(const std::wstring& name);
    std::filesystem::path Path(const std::wstring& name);

    bool Save(const std::wstring& name,
              const VisualSettings& settings,
              const RuntimeSettings& runtime);

    bool Load(const std::wstring& name,
              VisualSettings& settings,
              bool& particlesEnabled,
              float& particleSpeed,
              float& particleAmount);

    bool Delete(const std::wstring& name);
}
