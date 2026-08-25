#pragma once
#include <SDL3/SDL.h>

namespace constants {
    constexpr const char* kDefaultFontPath = "assets/fonts/font.ttf";
    constexpr SDL_Color kDefaultTextColor  {0,   0,   0,   255};
    constexpr SDL_Color kSelectionColor    {100, 150, 255, 180};
    constexpr SDL_Color kFocusOutlineColor {0,   120, 215, 255};
    constexpr SDL_Color kTitleBarColor     {200, 200, 200, 255};
    constexpr SDL_Color kTitleBarLineColor {150, 150, 150, 255};
    constexpr int      kTooltipDelayMs     = 500;
    constexpr int      kTooltipFontSize    = 14;
    constexpr int      kTooltipPadding     = 5;

    // Systemowe kolory 3D Windows 95/98
    constexpr SDL_Color kWin95Face       {192, 192, 192, 255};
    constexpr SDL_Color kWin95Light      {223, 223, 223, 255};
    constexpr SDL_Color kWin95Highlight  {255, 255, 255, 255};
    constexpr SDL_Color kWin95Shadow     {128, 128, 128, 255};
    constexpr SDL_Color kWin95DarkShadow {0,   0,   0,   255};
}
