#pragma once

#include "AppConfig.h"
#include "Models.h"

// App.cpp owns application state and non-visual features:
// - YouTube link parsing and thumbnail download
// - save-folder and cleanup settings
// - persistent Library/Favorites state
// - image loading and Direct2D resource helpers
//
// The project currently uses a unity build through main.cpp so the existing
// internal static helpers stay private and reliable. Shared models and layout
// settings are placed in headers for easy editing.
