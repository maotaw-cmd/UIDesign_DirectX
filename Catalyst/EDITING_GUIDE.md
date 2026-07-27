# Beginner Editing Guide

## Change default options

Open `src/Core/ProjectCore.hpp` and find `struct VisualSettings`.

Examples:

- `bool box = true;` controls whether the box starts enabled.
- `int boxStyle = 1;` selects the default box style.
- `float modelScale = 0.94f;` changes the default model size.
- `D2D1_COLOR_F visualGlowColour = MakeColour(0xFFFFFF);` changes the default outline colour.

## Add or move a Visuals control

Open `src/UI/VisualsUI.hpp` and find `DrawVisualControls`.

Rows use either:

- `ToggleRow(...)` for a normal toggle; or
- `ToggleColourRow(...)` for a toggle with paint and optional gear icons.

The Sound Walk row passes `VisualPopup::SoundWalk`, which opens its own settings card.

## Edit the Sound Walk animation

Open `src/UI/VisualsUI.hpp` and search for `animationStyle` inside `DrawPreviewOverlay`.

The four presets control duration, maximum radius, opacity, and ring count. The user sliders are applied after the preset.

## Edit shaders or 3D glow

Open `src/Renderer/DirectXRenderer.hpp` and search for `shaderSource` or `DrawMannequin`.

The embedded character data is in `EmbeddedCharacterMesh.inl`; do not edit that large file for ordinary UI changes.

## Edit configuration behaviour

Open `src/Config/ConfigManager.hpp` for file paths, cleaning names, saving, loading, deleting, default-config creation, and list refresh.

Open `src/Config/ConfigUI.hpp` only for the configuration page layout and its save/load/delete buttons.

`src/UI/VisualsUI.hpp` no longer contains config persistence or the config page.

## Add a config value

1. Add the field to `VisualSettings` in `ProjectCore.hpp`.
2. Save it in `SaveConfig` in `Config/ConfigManager.hpp`.
3. Load and clamp it in `LoadConfig` in `Config/ConfigManager.hpp`.
4. Add a UI control in `VisualsUI.hpp` only when the value needs an editable control.

## Sound Walk animation controls

Sound Walk animation is edited only from the gear icon beside **Sound walk** on the Visuals page. The main Settings page intentionally contains no Sound Walk animation controls.


## Sound Walk UI location

The Sound Walk row and its animation popup are implemented only in `src/UI/SoundWalkSettings.hpp`. The main Settings page has no Sound Walk animation controls.
