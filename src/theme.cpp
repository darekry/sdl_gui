#include "theme.hpp"

// === Per-type, per-state API ===

void Theme::setStyle(const std::string& type, ElementState state, Style style) {
    m_typeStyles[type][state] = std::move(style);
}

Style Theme::getStyle(const std::string& type, ElementState state) const {
    // 1. Spróbuj znaleźć styl dla konkretnego typu i stanu
    auto typeIt = m_typeStyles.find(type);
    if (typeIt != m_typeStyles.end()) {
        auto stateIt = typeIt->second.find(state);
        if (stateIt != typeIt->second.end()) {
            Style result = stateIt->second;
            result.mergeWith(m_defaultStyle);
            return result;
        }
        
        // Fallback: jeśli nie ma stylu dla tego stanu, spróbuj Normal
        auto normalIt = typeIt->second.find(ElementState::Normal);
        if (normalIt != typeIt->second.end()) {
            Style result = normalIt->second;
            result.mergeWith(m_defaultStyle);
            return result;
        }
    }
    
    // 2. Jeśli brak stylu dla typu, zwróć default style
    return m_defaultStyle;
}

// === Legacy API (backward compatible) ===

void Theme::setStyle(const std::string& type, Style style) {
    setStyle(type, ElementState::Normal, std::move(style));
}

Style Theme::getStyle(const std::string& type) const {
    return getStyle(type, ElementState::Normal);
}

// === Default style ===

void Theme::setDefaultStyle(Style style) {
    m_defaultStyle = std::move(style);
}

const Style& Theme::getDefaultStyle() const {
    return m_defaultStyle;
}

// === Factory ===

Theme Theme::createDefaultTheme() {
    Theme theme;

    // === Kolory bazowe motywu "Windows 95/98" ===
    const SDL_Color bg_color = {212, 208, 200, 255};       // Jasnoszary (app background)
    const SDL_Color text_color = {0, 0, 0, 255};           // Czarny
    const SDL_Color border_dark = {128, 128, 128, 255};    // Ciemnoszary
    
    // === Domyślny styl bazowy ===
    Style defaultStyle;
    defaultStyle.backgroundColor = bg_color;
    defaultStyle.textColor = text_color;
    defaultStyle.borderColor = border_dark;
    defaultStyle.borderWidth = 0;
    defaultStyle.fontSize = 16;
    defaultStyle.fontName = "assets/fonts/font.ttf";
    theme.setDefaultStyle(defaultStyle);

    // === Button styles ===
    // Normal: standard button look
    Style buttonNormal;
    buttonNormal.borderWidth = 2;
    buttonNormal.borderRadius = 4;
    theme.setStyle("Button", ElementState::Normal, buttonNormal);
    
    // Hover: slightly lighter background
    Style buttonHover;
    buttonHover.backgroundColor = {230, 226, 218, 255};
    buttonHover.borderWidth = 2;
    buttonHover.borderRadius = 4;
    theme.setStyle("Button", ElementState::Hover, buttonHover);
    
    // Pressed: darker, pushed look
    Style buttonPressed;
    buttonPressed.backgroundColor = {180, 176, 168, 255};
    buttonPressed.borderWidth = 2;
    buttonPressed.borderRadius = 4;
    theme.setStyle("Button", ElementState::Pressed, buttonPressed);

    // === Panel style ===
    Style panelStyle;
    panelStyle.borderWidth = 1;
    theme.setStyle("Panel", ElementState::Normal, panelStyle);
    
    // === Slider style (inherits from Panel) ===
    Style sliderStyle;
    sliderStyle.borderWidth = 1;
    theme.setStyle("Slider", ElementState::Normal, sliderStyle);

    // === TextInput styles ===
    Style textInputNormal;
    textInputNormal.backgroundColor = {255, 255, 255, 255};  // Białe tło
    textInputNormal.borderWidth = 2;
    textInputNormal.borderRadius = 2;
    theme.setStyle("TextInput", ElementState::Normal, textInputNormal);
    
    Style textInputHover;
    textInputHover.backgroundColor = {255, 255, 255, 255};
    textInputHover.borderWidth = 2;
    textInputHover.borderRadius = 2;
    textInputHover.borderColor = {100, 100, 100, 255};  // Darker border on hover
    theme.setStyle("TextInput", ElementState::Hover, textInputHover);

    // === TextArea styles ===
    Style textAreaNormal;
    textAreaNormal.backgroundColor = {255, 255, 255, 255};
    textAreaNormal.borderWidth = 2;
    textAreaNormal.borderRadius = 2;
    theme.setStyle("TextArea", ElementState::Normal, textAreaNormal);
    
    Style textAreaHover;
    textAreaHover.backgroundColor = {255, 255, 255, 255};
    textAreaHover.borderWidth = 2;
    textAreaHover.borderRadius = 2;
    textAreaHover.borderColor = {100, 100, 100, 255};
    theme.setStyle("TextArea", ElementState::Hover, textAreaHover);

    // === StringGrid style ===
    Style stringGridStyle;
    stringGridStyle.backgroundColor = {255, 255, 255, 255};
    stringGridStyle.textColor = {0, 0, 0, 255};
    stringGridStyle.borderColor = {200, 200, 200, 255};
    stringGridStyle.borderWidth = 1;
    theme.setStyle("StringGrid", ElementState::Normal, stringGridStyle);
    
    return theme;
}