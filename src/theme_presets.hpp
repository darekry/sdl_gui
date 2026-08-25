#pragma once

#include "theme.hpp"
#include "constants.hpp"
#include "gui.hpp" // applyBevelToStyle

namespace ThemePresets {

// Kolorystyka Win95 z fazą 3D wpiętą w Style.
inline Style withBevel(Style style, BevelType type) {
    applyBevelToStyle(style, type);
    return style;
}

// ═══════════════════════════════════════════════════════════════════
// Windows 9x (klasyczny wygląd Win95/98)
// ═══════════════════════════════════════════════════════════════════

inline Theme createWin9xTheme() {
    Theme theme;

    constexpr SDL_Color kBtnShadow    {128, 128, 128, 255};
    constexpr SDL_Color kBtnDarkShadow {64, 64, 64, 255};
    constexpr SDL_Color kWindowBg     {192, 192, 192, 255};
    constexpr SDL_Color kWindowText   {0, 0, 0, 255};
    constexpr SDL_Color kWhite        {255, 255, 255, 255};
    constexpr SDL_Color kHighlight    {0, 0, 128, 255};
    constexpr SDL_Color kHighlightText {255, 255, 255, 255};
    constexpr SDL_Color k3dShadow     {128, 128, 128, 255};
    constexpr SDL_Color kDisabledText {128, 128, 128, 255};

    Style defaultStyle;
    defaultStyle.backgroundColor = kWindowBg;
    defaultStyle.textColor = kWindowText;
    defaultStyle.borderColor = k3dShadow;
    defaultStyle.borderWidth = 0;
    defaultStyle.borderRadius = 0;
    defaultStyle.fontSize = 14;
    defaultStyle.fontName = constants::kDefaultFontPath;
    theme.setDefaultStyle(defaultStyle);

    // Button
    {
        Style s;
        s.borderWidth = 2;
        s.borderRadius = 0;
        theme.setStyle("Button", ElementState::Normal, s);

        s.backgroundColor = {223, 223, 223, 255};
        theme.setStyle("Button", ElementState::Hover, s);

        s.backgroundColor = {160, 160, 160, 255};
        s.borderColor = kBtnDarkShadow;
        theme.setStyle("Button", ElementState::Pressed, s);

        s = Style{};
        s.backgroundColor = {192, 192, 192, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {160, 160, 160, 255};
        theme.setStyle("Button", ElementState::Disabled, s);
    }

    // Panel
    {
        Style s;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.backgroundColor = kWindowBg;
        theme.setStyle("Panel", ElementState::Normal, s);
    }

    // TextInput - sunken 3D look
    {
        Style s;
        s.backgroundColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kBtnShadow;
        s.textColor = kWindowText;
        theme.setStyle("TextInput", ElementState::Normal, s);

        s.borderColor = kHighlight;
        theme.setStyle("TextInput", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {212, 208, 200, 255};
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {160, 160, 160, 255};
        s.textColor = kDisabledText;
        theme.setStyle("TextInput", ElementState::Disabled, s);
    }

    // TextArea
    {
        Style s;
        s.backgroundColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kBtnShadow;
        s.textColor = kWindowText;
        theme.setStyle("TextArea", ElementState::Normal, s);

        s.borderColor = kHighlight;
        theme.setStyle("TextArea", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {212, 208, 200, 255};
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {160, 160, 160, 255};
        s.textColor = kDisabledText;
        theme.setStyle("TextArea", ElementState::Disabled, s);
    }

    // Checkbox
    {
        Style s;
        s.textColor = kWindowText;
        s.fontSize = 14;
        theme.setStyle("Checkbox", ElementState::Normal, s);
    }

    // RadioButton
    {
        Style s;
        s.textColor = kWindowText;
        s.fontSize = 14;
        theme.setStyle("RadioButton", ElementState::Normal, s);
    }

    // Label
    {
        Style s;
        s.textColor = kWindowText;
        s.backgroundColor = {0, 0, 0, 0}; // transparent
        s.fontSize = 14;
        theme.setStyle("Label", ElementState::Normal, s);
    }

    // Slider
    {
        Style s;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.backgroundColor = kWindowBg;
        theme.setStyle("Slider", ElementState::Normal, s);
    }

    // RangeSlider
    {
        Style s;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.backgroundColor = kWindowBg;
        theme.setStyle("RangeSlider", ElementState::Normal, s);
    }

    // ProgressBar
    {
        Style s;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.backgroundColor = kWhite;
        s.borderColor = {0, 0, 128, 255};
        theme.setStyle("ProgressBar", ElementState::Normal, s);
    }

    // StringGrid
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kWindowText;
        s.borderColor = kBtnShadow;
        s.borderWidth = 1;
        s.borderRadius = 0;
        theme.setStyle("StringGrid", ElementState::Normal, s);
    }

    // ListView
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kWindowText;
        s.borderColor = kBtnShadow;
        s.borderWidth = 2;
        s.borderRadius = 0;
        theme.setStyle("ListView", ElementState::Normal, s);
    }

    // ComboBox
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kWindowText;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kBtnShadow;
        theme.setStyle("ComboBox", ElementState::Normal, s);

        s.borderColor = kHighlight;
        theme.setStyle("ComboBox", ElementState::Hover, s);
    }

    // TabControl
    {
        Style s;
        s.backgroundColor = kWindowBg;
        s.borderWidth = 1;
        s.borderRadius = 0;
        s.textColor = kWindowText;
        theme.setStyle("TabControl", ElementState::Normal, s);
    }

    // ContextMenu
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kWindowText;
        s.borderWidth = 1;
        s.borderRadius = 0;
        s.borderColor = kBtnShadow;
        theme.setStyle("ContextMenu", ElementState::Normal, s);

        s.backgroundColor = kHighlight;
        s.textColor = kHighlightText;
        theme.setStyle("ContextMenu", ElementState::Hover, s);
    }

    // ScrollArea
    {
        Style s;
        s.backgroundColor = kWindowBg;
        s.borderWidth = 1;
        s.borderRadius = 0;
        theme.setStyle("ScrollArea", ElementState::Normal, s);
    }

    // AnimatedImage
    {
        Style s;
        s.backgroundColor = kWindowBg;
        theme.setStyle("AnimatedImage", ElementState::Normal, s);
    }

    // Canvas
    {
        Style s;
        s.backgroundColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kBtnShadow;
        theme.setStyle("Canvas", ElementState::Normal, s);
    }

    return theme;
}


// ═══════════════════════════════════════════════════════════════════
// Windows 95/98 — autentyczny look z fazami 3D (bevel).
// Przyciski wypukłe (Raised), pola edycji wklęśnięte (Sunken).
// ═══════════════════════════════════════════════════════════════════

inline Theme createWindows95Theme() {
    Theme theme;

    constexpr SDL_Color kWindowText    {0, 0, 0, 255};
    constexpr SDL_Color kWhite         {255, 255, 255, 255};
    constexpr SDL_Color kHighlightText {255, 255, 255, 255};
    constexpr SDL_Color kNavy          {0, 0, 128, 255};
    constexpr SDL_Color kDisabledText  {128, 128, 128, 255};

    Style defaultStyle;
    defaultStyle.backgroundColor = constants::kWin95Face;
    defaultStyle.textColor = kWindowText;
    defaultStyle.borderRadius = 0;
    defaultStyle.borderWidth = 0;
    defaultStyle.fontSize = 14;
    defaultStyle.fontName = constants::kDefaultFontPath;
    theme.setDefaultStyle(defaultStyle);

    // Button: Raised w spoczynku, Sunken po wciśnięciu
    {
        Style s;
        s.backgroundColor = constants::kWin95Face;
        s.borderRadius = 0;
        theme.setStyle("Button", ElementState::Normal, withBevel(s, BevelType::Raised));

        Style h = s;
        h.backgroundColor = constants::kWin95Light;
        theme.setStyle("Button", ElementState::Hover, withBevel(h, BevelType::Raised));

        Style p = s;
        theme.setStyle("Button", ElementState::Pressed, withBevel(p, BevelType::Sunken));

        Style d = s;
        d.textColor = kDisabledText;
        theme.setStyle("Button", ElementState::Disabled, withBevel(d, BevelType::Raised));
    }

    // Panel: płaskie tło okna (ramki okien ustawia użytkownik)
    {
        Style s;
        s.backgroundColor = constants::kWin95Face;
        s.borderRadius = 0;
        theme.setStyle("Panel", ElementState::Normal, s);
    }

    // Pola edycji i listy: białe tło + Sunken
    auto sunkenField = [](SDL_Color bg) {
        Style s;
        s.backgroundColor = bg;
        s.borderRadius = 0;
        return withBevel(s, BevelType::Sunken);
    };

    theme.setStyle("TextInput", ElementState::Normal, sunkenField(kWhite));
    theme.setStyle("TextInput", ElementState::Hover, sunkenField(kWhite));
    {
        Style d = sunkenField(constants::kWin95Face);
        d.borderColorOuterTopLeft.reset();
        d.borderColorOuterBottomRight.reset();
        d.borderColorInnerTopLeft.reset();
        d.borderColorInnerBottomRight.reset();
        d.textColor = kDisabledText;
        theme.setStyle("TextInput", ElementState::Disabled, d);
    }

    theme.setStyle("TextArea", ElementState::Normal, sunkenField(kWhite));
    theme.setStyle("TextArea", ElementState::Hover, sunkenField(kWhite));
    {
        Style d = sunkenField(constants::kWin95Face);
        d.borderColorOuterTopLeft.reset();
        d.borderColorOuterBottomRight.reset();
        d.borderColorInnerTopLeft.reset();
        d.borderColorInnerBottomRight.reset();
        d.textColor = kDisabledText;
        theme.setStyle("TextArea", ElementState::Disabled, d);
    }

    // ListView / ComboBox / Canvas: białe + Sunken
    theme.setStyle("ListView", ElementState::Normal, sunkenField(kWhite));
    theme.setStyle("ComboBox", ElementState::Normal, sunkenField(kWhite));
    theme.setStyle("Canvas", ElementState::Normal, sunkenField(kWhite));

    // StringGrid: biały + Sunken; borderColor zostaje jako kolor linii siatki
    {
        Style s = sunkenField(kWhite);
        s.borderColor = constants::kWin95Shadow;
        theme.setStyle("StringGrid", ElementState::Normal, s);
    }

    // ProgressBar: białe tło + Sunken; borderColor = kolor wypełnienia (navy)
    {
        Style s = sunkenField(kWhite);
        s.borderColor = kNavy;
        theme.setStyle("ProgressBar", ElementState::Normal, s);
    }

    // Slider: track rysowany przez widget; borderColor = kolor suwaka
    {
        Style s;
        s.backgroundColor = constants::kWin95Face;
        s.borderColor = constants::kWin95Shadow;
        s.borderRadius = 0;
        theme.setStyle("Slider", ElementState::Normal, s);
    }

    {
        Style s;
        s.backgroundColor = constants::kWin95Face;
        s.borderColor = constants::kWin95Shadow;
        s.borderRadius = 0;
        theme.setStyle("RangeSlider", ElementState::Normal, s);
    }

    // Checkbox / RadioButton: tylko tekst (boxy rysuje widget)
    {
        Style s;
        s.textColor = kWindowText;
        s.fontSize = 14;
        theme.setStyle("Checkbox", ElementState::Normal, s);

        Style r = s;
        theme.setStyle("RadioButton", ElementState::Normal, r);
    }

    // Label: przezroczysty
    {
        Style s;
        s.textColor = kWindowText;
        s.backgroundColor = SDL_Color{0, 0, 0, 0};
        s.fontSize = 14;
        theme.setStyle("Label", ElementState::Normal, s);
    }

    // TabControl / ScrollArea / AnimatedImage: płaskie tło
    {
        Style s;
        s.backgroundColor = constants::kWin95Face;
        s.borderRadius = 0;
        theme.setStyle("TabControl", ElementState::Normal, s);
        theme.setStyle("ScrollArea", ElementState::Normal, s);
        theme.setStyle("AnimatedImage", ElementState::Normal, s);
    }

    // ContextMenu: białe z cieniem, zaznaczenie navy
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kWindowText;
        s.borderColor = constants::kWin95Shadow;
        s.borderWidth = 1;
        s.borderRadius = 0;
        theme.setStyle("ContextMenu", ElementState::Normal, s);

        Style h = s;
        h.backgroundColor = kNavy;
        h.textColor = kHighlightText;
        theme.setStyle("ContextMenu", ElementState::Hover, h);
    }

    return theme;
}


// ═══════════════════════════════════════════════════════════════════
// Jasny (nowoczesny, czysty)
// ═══════════════════════════════════════════════════════════════════

inline Theme createLightTheme() {
    Theme theme;

    constexpr SDL_Color kBg        {240, 240, 240, 255};
    constexpr SDL_Color kText      {20, 20, 20, 255};
    constexpr SDL_Color kBorder    {200, 200, 200, 255};
    constexpr SDL_Color kAccent    {0, 120, 215, 255};
    constexpr SDL_Color kWhite     {255, 255, 255, 255};
    constexpr SDL_Color kPanelBg   {230, 230, 230, 255};
    constexpr SDL_Color kDisabledText {160, 160, 160, 255};
    constexpr SDL_Color kInputBg   {255, 255, 255, 255};

    Style defaultStyle;
    defaultStyle.backgroundColor = kBg;
    defaultStyle.textColor = kText;
    defaultStyle.borderColor = kBorder;
    defaultStyle.borderWidth = 1;
    defaultStyle.borderRadius = 6;
    defaultStyle.fontSize = 16;
    defaultStyle.fontName = constants::kDefaultFontPath;
    theme.setDefaultStyle(defaultStyle);

    // Button
    {
        Style s;
        s.backgroundColor = kAccent;
        s.textColor = kWhite;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kAccent;
        theme.setStyle("Button", ElementState::Normal, s);

        s.backgroundColor = {0, 100, 190, 255};
        s.borderColor = {0, 100, 190, 255};
        theme.setStyle("Button", ElementState::Hover, s);

        s.backgroundColor = {0, 80, 160, 255};
        s.borderColor = {0, 80, 160, 255};
        theme.setStyle("Button", ElementState::Pressed, s);

        s = Style{};
        s.backgroundColor = {200, 200, 200, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {210, 210, 210, 255};
        theme.setStyle("Button", ElementState::Disabled, s);
    }

    // Panel
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 8;
        theme.setStyle("Panel", ElementState::Normal, s);
    }

    // TextInput
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("TextInput", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("TextInput", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {248, 248, 248, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {225, 225, 225, 255};
        theme.setStyle("TextInput", ElementState::Disabled, s);
    }

    // TextArea
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("TextArea", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("TextArea", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {248, 248, 248, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {225, 225, 225, 255};
        theme.setStyle("TextArea", ElementState::Disabled, s);
    }

    // Checkbox
    {
        Style s;
        s.textColor = kAccent;
        s.fontSize = 16;
        theme.setStyle("Checkbox", ElementState::Normal, s);
    }

    // RadioButton
    {
        Style s;
        s.textColor = kAccent;
        s.fontSize = 16;
        theme.setStyle("RadioButton", ElementState::Normal, s);
    }

    // Label
    {
        Style s;
        s.textColor = kText;
        s.fontSize = 16;
        theme.setStyle("Label", ElementState::Normal, s);
    }

    // Slider
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("Slider", ElementState::Normal, s);
    }

    // RangeSlider
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("RangeSlider", ElementState::Normal, s);
    }

    // ProgressBar
    {
        Style s;
        s.backgroundColor = {225, 225, 225, 255};
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kAccent;
        theme.setStyle("ProgressBar", ElementState::Normal, s);
    }

    // StringGrid
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kText;
        s.borderColor = kBorder;
        s.borderWidth = 1;
        s.borderRadius = 4;
        theme.setStyle("StringGrid", ElementState::Normal, s);
    }

    // ListView
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kText;
        s.borderColor = kBorder;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("ListView", ElementState::Normal, s);
    }

    // ComboBox
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("ComboBox", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("ComboBox", ElementState::Hover, s);
    }

    // TabControl
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.textColor = kText;
        theme.setStyle("TabControl", ElementState::Normal, s);
    }

    // ContextMenu
    {
        Style s;
        s.backgroundColor = kWhite;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("ContextMenu", ElementState::Normal, s);

        s.backgroundColor = kAccent;
        s.textColor = kWhite;
        theme.setStyle("ContextMenu", ElementState::Hover, s);
    }

    // ScrollArea
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("ScrollArea", ElementState::Normal, s);
    }

    // AnimatedImage
    {
        Style s;
        s.backgroundColor = kBg;
        theme.setStyle("AnimatedImage", ElementState::Normal, s);
    }

    // Canvas
    {
        Style s;
        s.backgroundColor = kWhite;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("Canvas", ElementState::Normal, s);
    }

    return theme;
}


// ═══════════════════════════════════════════════════════════════════
// Ciemny (dark mode)
// ═══════════════════════════════════════════════════════════════════

inline Theme createDarkTheme() {
    Theme theme;

    constexpr SDL_Color kBg        {30, 30, 30, 255};
    constexpr SDL_Color kPanelBg   {40, 40, 40, 255};
    constexpr SDL_Color kText      {220, 220, 220, 255};
    constexpr SDL_Color kBorder    {70, 70, 70, 255};
    constexpr SDL_Color kAccent    {60, 140, 240, 255};
    constexpr SDL_Color kWhite     {255, 255, 255, 255};
    constexpr SDL_Color kDisabledText {100, 100, 100, 255};
    constexpr SDL_Color kInputBg   {50, 50, 50, 255};

    Style defaultStyle;
    defaultStyle.backgroundColor = kBg;
    defaultStyle.textColor = kText;
    defaultStyle.borderColor = kBorder;
    defaultStyle.borderWidth = 1;
    defaultStyle.borderRadius = 6;
    defaultStyle.fontSize = 16;
    defaultStyle.fontName = constants::kDefaultFontPath;
    theme.setDefaultStyle(defaultStyle);

    // Button
    {
        Style s;
        s.backgroundColor = kAccent;
        s.textColor = kWhite;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kAccent;
        theme.setStyle("Button", ElementState::Normal, s);

        s.backgroundColor = {80, 155, 245, 255};
        s.borderColor = {80, 155, 245, 255};
        theme.setStyle("Button", ElementState::Hover, s);

        s.backgroundColor = {40, 120, 220, 255};
        s.borderColor = {40, 120, 220, 255};
        theme.setStyle("Button", ElementState::Pressed, s);

        s = Style{};
        s.backgroundColor = {60, 60, 60, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {55, 55, 55, 255};
        theme.setStyle("Button", ElementState::Disabled, s);
    }

    // Panel
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 8;
        theme.setStyle("Panel", ElementState::Normal, s);
    }

    // TextInput
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("TextInput", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("TextInput", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {40, 40, 40, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {50, 50, 50, 255};
        theme.setStyle("TextInput", ElementState::Disabled, s);
    }

    // TextArea
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("TextArea", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("TextArea", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = {40, 40, 40, 255};
        s.textColor = kDisabledText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = {50, 50, 50, 255};
        theme.setStyle("TextArea", ElementState::Disabled, s);
    }

    // Checkbox
    {
        Style s;
        s.textColor = kAccent;
        s.fontSize = 16;
        theme.setStyle("Checkbox", ElementState::Normal, s);
    }

    // RadioButton
    {
        Style s;
        s.textColor = kAccent;
        s.fontSize = 16;
        theme.setStyle("RadioButton", ElementState::Normal, s);
    }

    // Label
    {
        Style s;
        s.textColor = kText;
        s.fontSize = 16;
        theme.setStyle("Label", ElementState::Normal, s);
    }

    // Slider
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("Slider", ElementState::Normal, s);
    }

    // RangeSlider
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("RangeSlider", ElementState::Normal, s);
    }

    // ProgressBar
    {
        Style s;
        s.backgroundColor = {60, 60, 60, 255};
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kAccent;
        theme.setStyle("ProgressBar", ElementState::Normal, s);
    }

    // StringGrid
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderColor = kBorder;
        s.borderWidth = 1;
        s.borderRadius = 4;
        theme.setStyle("StringGrid", ElementState::Normal, s);
    }

    // ListView
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderColor = kBorder;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("ListView", ElementState::Normal, s);
    }

    // ComboBox
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("ComboBox", ElementState::Normal, s);

        s.borderColor = kAccent;
        theme.setStyle("ComboBox", ElementState::Hover, s);
    }

    // TabControl
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.textColor = kText;
        theme.setStyle("TabControl", ElementState::Normal, s);
    }

    // ContextMenu
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.textColor = kText;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("ContextMenu", ElementState::Normal, s);

        s.backgroundColor = kAccent;
        s.textColor = kWhite;
        theme.setStyle("ContextMenu", ElementState::Hover, s);
    }

    // ScrollArea
    {
        Style s;
        s.backgroundColor = kPanelBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        theme.setStyle("ScrollArea", ElementState::Normal, s);
    }

    // AnimatedImage
    {
        Style s;
        s.backgroundColor = kBg;
        theme.setStyle("AnimatedImage", ElementState::Normal, s);
    }

    // Canvas
    {
        Style s;
        s.backgroundColor = kInputBg;
        s.borderWidth = 1;
        s.borderRadius = 6;
        s.borderColor = kBorder;
        theme.setStyle("Canvas", ElementState::Normal, s);
    }

    return theme;
}


// ═══════════════════════════════════════════════════════════════════
// Wysoki kontrast (dostępność)
// ═══════════════════════════════════════════════════════════════════

inline Theme createHighContrastTheme() {
    Theme theme;

    constexpr SDL_Color kBlack    {0, 0, 0, 255};
    constexpr SDL_Color kWhite    {255, 255, 255, 255};
    constexpr SDL_Color kYellow   {255, 255, 0, 255};
    constexpr SDL_Color kCyan     {0, 255, 255, 255};

    Style defaultStyle;
    defaultStyle.backgroundColor = kBlack;
    defaultStyle.textColor = kWhite;
    defaultStyle.borderColor = kWhite;
    defaultStyle.borderWidth = 2;
    defaultStyle.borderRadius = 0;
    defaultStyle.fontSize = 18;
    defaultStyle.fontName = constants::kDefaultFontPath;
    theme.setDefaultStyle(defaultStyle);

    // Button
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kYellow;
        s.borderWidth = 3;
        s.borderRadius = 0;
        s.borderColor = kYellow;
        theme.setStyle("Button", ElementState::Normal, s);

        s.backgroundColor = kYellow;
        s.textColor = kBlack;
        theme.setStyle("Button", ElementState::Hover, s);

        s.backgroundColor = kCyan;
        s.textColor = kBlack;
        s.borderColor = kCyan;
        theme.setStyle("Button", ElementState::Pressed, s);

        s = Style{};
        s.backgroundColor = kBlack;
        s.textColor = {128, 128, 128, 255};
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {128, 128, 128, 255};
        theme.setStyle("Button", ElementState::Disabled, s);
    }

    // Panel
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("Panel", ElementState::Normal, s);
    }

    // TextInput
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("TextInput", ElementState::Normal, s);

        s.borderColor = kYellow;
        theme.setStyle("TextInput", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = kBlack;
        s.textColor = {128, 128, 128, 255};
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {128, 128, 128, 255};
        theme.setStyle("TextInput", ElementState::Disabled, s);
    }

    // TextArea
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("TextArea", ElementState::Normal, s);

        s.borderColor = kYellow;
        theme.setStyle("TextArea", ElementState::Hover, s);

        s = Style{};
        s.backgroundColor = kBlack;
        s.textColor = {128, 128, 128, 255};
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = {128, 128, 128, 255};
        theme.setStyle("TextArea", ElementState::Disabled, s);
    }

    // Checkbox
    {
        Style s;
        s.textColor = kCyan;
        s.fontSize = 18;
        theme.setStyle("Checkbox", ElementState::Normal, s);
    }

    // RadioButton
    {
        Style s;
        s.textColor = kCyan;
        s.fontSize = 18;
        theme.setStyle("RadioButton", ElementState::Normal, s);
    }

    // Label
    {
        Style s;
        s.textColor = kWhite;
        s.fontSize = 18;
        theme.setStyle("Label", ElementState::Normal, s);
    }

    // Slider
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("Slider", ElementState::Normal, s);
    }

    // RangeSlider
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("RangeSlider", ElementState::Normal, s);
    }

    // ProgressBar
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kYellow;
        theme.setStyle("ProgressBar", ElementState::Normal, s);
    }

    // StringGrid
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        theme.setStyle("StringGrid", ElementState::Normal, s);
    }

    // ListView
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        theme.setStyle("ListView", ElementState::Normal, s);
    }

    // ComboBox
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("ComboBox", ElementState::Normal, s);

        s.borderColor = kYellow;
        theme.setStyle("ComboBox", ElementState::Hover, s);
    }

    // TabControl
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.textColor = kWhite;
        theme.setStyle("TabControl", ElementState::Normal, s);
    }

    // ContextMenu
    {
        Style s;
        s.backgroundColor = kBlack;
        s.textColor = kWhite;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("ContextMenu", ElementState::Normal, s);

        s.backgroundColor = kYellow;
        s.textColor = kBlack;
        theme.setStyle("ContextMenu", ElementState::Hover, s);
    }

    // ScrollArea
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        theme.setStyle("ScrollArea", ElementState::Normal, s);
    }

    // AnimatedImage
    {
        Style s;
        s.backgroundColor = kBlack;
        theme.setStyle("AnimatedImage", ElementState::Normal, s);
    }

    // Canvas
    {
        Style s;
        s.backgroundColor = kBlack;
        s.borderWidth = 2;
        s.borderRadius = 0;
        s.borderColor = kWhite;
        theme.setStyle("Canvas", ElementState::Normal, s);
    }

    return theme;
}

} // namespace ThemePresets
