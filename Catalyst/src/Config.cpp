#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Config.h"

#include <windows.h>
#include <d2d1.h>

#include <algorithm>
#include <cwctype>
#include <fstream>
#include <system_error>

namespace Config
{
    std::filesystem::path Directory()
    {
        wchar_t appData[MAX_PATH]{};
        const DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
        std::filesystem::path directory = length > 0
            ? std::filesystem::path(appData) / L"Visuals3D" / L"Configs"
            : std::filesystem::current_path() / L"Configs";

        std::error_code error;
        std::filesystem::create_directories(directory, error);
        return directory;
    }

    std::wstring CleanName(const std::wstring& name)
    {
        std::wstring clean;
        clean.reserve(name.size());
        for (wchar_t character : name)
        {
            if (std::iswalnum(character) || character == L' ' ||
                character == L'-' || character == L'_')
            {
                clean.push_back(character);
            }
        }

        while (!clean.empty() && clean.front() == L' ')
            clean.erase(clean.begin());
        while (!clean.empty() && clean.back() == L' ')
            clean.pop_back();
        if (clean.size() > 36)
            clean.resize(36);
        return clean;
    }

    std::filesystem::path Path(const std::wstring& name)
    {
        return Directory() / (CleanName(name) + L".cfg");
    }

    void WriteColour(std::wofstream& file, const wchar_t* prefix, const D2D1_COLOR_F& colour)
    {
        file << prefix << L"R=" << colour.r << L'\n';
        file << prefix << L"G=" << colour.g << L'\n';
        file << prefix << L"B=" << colour.b << L'\n';
    }

    bool Save(const std::wstring& name, const VisualSettings& settings, const RuntimeSettings& runtime)
    {
        const std::wstring clean = CleanName(name);
        if (clean.empty())
            return false;

        std::wofstream file(Path(clean), std::ios::trunc);
        if (!file)
            return false;

        auto writeBool = [&](const wchar_t* key, bool value) { file << key << L'=' << (value ? 1 : 0) << L'\n'; };
        auto writeInt = [&](const wchar_t* key, int value) { file << key << L'=' << value << L'\n'; };
        auto writeFloat = [&](const wchar_t* key, float value) { file << key << L'=' << value << L'\n'; };

        file << L"[Visuals3D]\n";
        writeBool(L"box", settings.box);
        writeInt(L"boxStyle", settings.boxStyle);
        writeFloat(L"boxThickness", settings.boxThickness);
        writeFloat(L"boxFillAlpha", settings.boxFillAlpha);
        writeBool(L"healthBar", settings.healthBar);
        writeInt(L"healthBarStyle", settings.healthBarStyle);
        writeFloat(L"healthBarThickness", settings.healthBarThickness);
        writeFloat(L"healthOutlineThickness", settings.healthOutlineThickness);
        writeBool(L"healthText", settings.healthText);
        writeBool(L"damageText", settings.damageText);
        writeBool(L"name", settings.name);
        writeBool(L"distance", settings.distance);
        writeBool(L"weapon", settings.weapon);
        writeBool(L"skeleton", settings.skeleton);
        writeFloat(L"skeletonThickness", settings.skeletonThickness);
        writeBool(L"snapline", settings.snapline);
        writeInt(L"snaplineOrigin", settings.snaplineOrigin);
        writeFloat(L"snaplineThickness", settings.snaplineThickness);
        writeBool(L"soundWalk", settings.soundWalk);
        writeInt(L"soundWalkAnimationStyle", settings.soundWalkAnimationStyle);
        writeFloat(L"soundWalkSpeed", settings.soundWalkSpeed);
        writeFloat(L"soundWalkExpansion", settings.soundWalkExpansion);
        writeFloat(L"soundWalkThickness", settings.soundWalkThickness);
        writeBool(L"soundMarker", settings.soundMarker);
        writeBool(L"modelOutline", settings.visualGlow);
        writeFloat(L"modelOutlineThickness", settings.visualGlowThickness);
        writeBool(L"rotateModel", settings.rotateModel);
        writeFloat(L"modelScale", settings.modelScale);
        writeBool(L"runtime.particlesEnabled", runtime.particlesEnabled);
        writeFloat(L"runtime.particleSpeed", runtime.particleSpeed);
        writeFloat(L"runtime.particleAmount", runtime.particleAmount);

        WriteColour(file, L"box", settings.boxColour);
        WriteColour(file, L"boxFill", settings.boxFillColour);
        WriteColour(file, L"health", settings.healthColour);
        WriteColour(file, L"healthBackground", settings.healthBackgroundColour);
        WriteColour(file, L"healthOutline", settings.healthOutlineColour);
        WriteColour(file, L"damageText", settings.damageTextColour);
        WriteColour(file, L"skeleton", settings.skeletonColour);
        WriteColour(file, L"snapline", settings.snaplineColour);
        WriteColour(file, L"soundWalk", settings.soundWalkColour);
        WriteColour(file, L"soundMarker", settings.soundMarkerColour);
        WriteColour(file, L"modelOutline", settings.visualGlowColour);
        WriteColour(file, L"model", settings.modelColour);
        WriteColour(file, L"accent", settings.accentColour);

        file.flush();
        return file.good();
    }

    bool Load(const std::wstring& name, VisualSettings& settings, bool& particlesEnabled, float& particleSpeed, float& particleAmount)
    {
        std::wifstream file(Path(name));
        if (!file)
            return false;

        std::wstring line;
        while (std::getline(file, line))
        {
            if (line.empty() || line.front() == L'[')
                continue;
            const std::size_t separator = line.find(L'=');
            if (separator == std::wstring::npos)
                continue;

            const std::wstring key = line.substr(0, separator);
            const std::wstring value = line.substr(separator + 1);
            try
            {
                const int intValue = std::stoi(value);
                const bool boolValue = intValue != 0;
                const float floatValue = std::stof(value);

                if (key == L"box") settings.box = boolValue;
                else if (key == L"boxStyle") settings.boxStyle = intValue;
                else if (key == L"boxThickness") settings.boxThickness = floatValue;
                else if (key == L"boxFillAlpha") settings.boxFillAlpha = floatValue;
                else if (key == L"healthBar") settings.healthBar = boolValue;
                else if (key == L"healthBarStyle") settings.healthBarStyle = intValue;
                else if (key == L"healthBarThickness") settings.healthBarThickness = floatValue;
                else if (key == L"healthOutlineThickness") settings.healthOutlineThickness = floatValue;
                else if (key == L"healthText") settings.healthText = boolValue;
                else if (key == L"damageText") settings.damageText = boolValue;
                else if (key == L"name") settings.name = boolValue;
                else if (key == L"distance") settings.distance = boolValue;
                else if (key == L"weapon") settings.weapon = boolValue;
                else if (key == L"skeleton") settings.skeleton = boolValue;
                else if (key == L"skeletonThickness") settings.skeletonThickness = floatValue;
                else if (key == L"snapline") settings.snapline = boolValue;
                else if (key == L"snaplineOrigin") settings.snaplineOrigin = intValue;
                else if (key == L"snaplineThickness") settings.snaplineThickness = floatValue;
                else if (key == L"soundWalk") settings.soundWalk = boolValue;
                else if (key == L"soundWalkAnimationStyle") settings.soundWalkAnimationStyle = intValue;
                else if (key == L"soundWalkSpeed") settings.soundWalkSpeed = floatValue;
                else if (key == L"soundWalkExpansion") settings.soundWalkExpansion = floatValue;
                else if (key == L"soundWalkThickness") settings.soundWalkThickness = floatValue;
                else if (key == L"soundMarker") settings.soundMarker = boolValue;
                else if (key == L"modelOutline") settings.visualGlow = boolValue;
                else if (key == L"modelOutlineThickness") settings.visualGlowThickness = floatValue;
                else if (key == L"rotateModel") settings.rotateModel = boolValue;
                else if (key == L"modelScale") settings.modelScale = floatValue;
                else if (key == L"particlesEnabled") particlesEnabled = boolValue;
                else if (key == L"particleSpeed") particleSpeed = floatValue;
                else if (key == L"particleAmount") particleAmount = floatValue;

                else if (key == L"boxR") settings.boxColour.r = floatValue;
                else if (key == L"boxG") settings.boxColour.g = floatValue;
                else if (key == L"boxB") settings.boxColour.b = floatValue;
                else if (key == L"boxFillR") settings.boxFillColour.r = floatValue;
                else if (key == L"boxFillG") settings.boxFillColour.g = floatValue;
                else if (key == L"boxFillB") settings.boxFillColour.b = floatValue;
                else if (key == L"healthR") settings.healthColour.r = floatValue;
                else if (key == L"healthG") settings.healthColour.g = floatValue;
                else if (key == L"healthB") settings.healthColour.b = floatValue;
                else if (key == L"healthBackgroundR") settings.healthBackgroundColour.r = floatValue;
                else if (key == L"healthBackgroundG") settings.healthBackgroundColour.g = floatValue;
                else if (key == L"healthBackgroundB") settings.healthBackgroundColour.b = floatValue;
                else if (key == L"healthOutlineR") settings.healthOutlineColour.r = floatValue;
                else if (key == L"healthOutlineG") settings.healthOutlineColour.g = floatValue;
                else if (key == L"healthOutlineB") settings.healthOutlineColour.b = floatValue;
                else if (key == L"damageTextR") settings.damageTextColour.r = floatValue;
                else if (key == L"damageTextG") settings.damageTextColour.g = floatValue;
                else if (key == L"damageTextB") settings.damageTextColour.b = floatValue;
                else if (key == L"skeletonR") settings.skeletonColour.r = floatValue;
                else if (key == L"skeletonG") settings.skeletonColour.g = floatValue;
                else if (key == L"skeletonB") settings.skeletonColour.b = floatValue;
                else if (key == L"snaplineR") settings.snaplineColour.r = floatValue;
                else if (key == L"snaplineG") settings.snaplineColour.g = floatValue;
                else if (key == L"snaplineB") settings.snaplineColour.b = floatValue;
                else if (key == L"soundWalkR") settings.soundWalkColour.r = floatValue;
                else if (key == L"soundWalkG") settings.soundWalkColour.g = floatValue;
                else if (key == L"soundWalkB") settings.soundWalkColour.b = floatValue;
                else if (key == L"soundMarkerR") settings.soundMarkerColour.r = floatValue;
                else if (key == L"soundMarkerG") settings.soundMarkerColour.g = floatValue;
                else if (key == L"soundMarkerB") settings.soundMarkerColour.b = floatValue;
                else if (key == L"modelOutlineR") settings.visualGlowColour.r = floatValue;
                else if (key == L"modelOutlineG") settings.visualGlowColour.g = floatValue;
                else if (key == L"modelOutlineB") settings.visualGlowColour.b = floatValue;
                else if (key == L"modelR") settings.modelColour.r = floatValue;
                else if (key == L"modelG") settings.modelColour.g = floatValue;
                else if (key == L"modelB") settings.modelColour.b = floatValue;
                else if (key == L"accentR") settings.accentColour.r = floatValue;
                else if (key == L"accentG") settings.accentColour.g = floatValue;
                else if (key == L"accentB") settings.accentColour.b = floatValue;
            }
            catch (...)
            {
            }
        }

        settings.boxStyle = std::clamp(settings.boxStyle, 0, 4);
        settings.healthBarStyle = std::clamp(settings.healthBarStyle, 0, 5);
        settings.snaplineOrigin = std::clamp(settings.snaplineOrigin, 0, 2);
        settings.soundWalkAnimationStyle = std::clamp(settings.soundWalkAnimationStyle, 0, 3);
        settings.boxThickness = std::clamp(settings.boxThickness, 0.6f, 6.0f);
        settings.boxFillAlpha = std::clamp(settings.boxFillAlpha, 0.0f, 1.0f);
        settings.healthBarThickness = std::clamp(settings.healthBarThickness, 2.0f, 16.0f);
        settings.healthOutlineThickness = std::clamp(settings.healthOutlineThickness, 0.0f, 4.0f);
        settings.snaplineThickness = std::clamp(settings.snaplineThickness, 0.6f, 6.0f);
        settings.skeletonThickness = std::clamp(settings.skeletonThickness, 0.6f, 5.0f);
        settings.visualGlowThickness = std::clamp(settings.visualGlowThickness, 0.5f, 6.0f);
        settings.soundWalkSpeed = std::clamp(settings.soundWalkSpeed, 0.35f, 1.80f);
        settings.soundWalkExpansion = std::clamp(settings.soundWalkExpansion, 0.70f, 2.30f);
        settings.soundWalkThickness = std::clamp(settings.soundWalkThickness, 0.60f, 4.00f);
        settings.modelScale = std::clamp(settings.modelScale, 0.55f, 1.05f);
        particleSpeed = std::clamp(particleSpeed, 4.0f, 60.0f);
        particleAmount = std::clamp(particleAmount, 8.0f, 70.0f);

        D2D1_COLOR_F* colours[] = {
            &settings.boxColour, &settings.boxFillColour,
            &settings.healthColour, &settings.healthBackgroundColour,
            &settings.healthOutlineColour, &settings.damageTextColour,
            &settings.skeletonColour,
            &settings.snaplineColour, &settings.soundWalkColour,
            &settings.soundMarkerColour, &settings.visualGlowColour,
            &settings.modelColour, &settings.accentColour
        };
        for (D2D1_COLOR_F* colour : colours)
        {
            colour->r = std::clamp(colour->r, 0.0f, 1.0f);
            colour->g = std::clamp(colour->g, 0.0f, 1.0f);
            colour->b = std::clamp(colour->b, 0.0f, 1.0f);
            colour->a = 1.0f;
        }
        return true;
    }

    bool Delete(const std::wstring& name)
    {
        std::error_code error;
        return std::filesystem::remove(Path(name), error);
    }
}
