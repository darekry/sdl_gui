#include "theme.hpp"


void Theme::setStyle(const std::string& componentType, ElementState state, Style style) {
    styles[componentType][state] = std::move(style);
}

Style Theme::getStyle(const std::string& componentType, ElementState state) const {
    auto componentStylesIt = styles.find(componentType);
    if (componentStylesIt != styles.end()) {
        auto stateStyleIt = componentStylesIt->second.find(state);
        if (stateStyleIt != componentStylesIt->second.end()) {
            return stateStyleIt->second;
        }
        // Fallback to Normal state if the specific state is not defined
        auto normalStyleIt = componentStylesIt->second.find(ElementState::Normal);
        if (normalStyleIt != componentStylesIt->second.end()) {
            return normalStyleIt->second;
        }
    }
    return defaultStyle;
}

Theme Theme::createDefaultTheme() {
    Theme theme;

    // --- Kolory motywu "Windows 95/98" ---
    const SDL_Color bg_color = {212, 208, 200, 255}; // Jasnoszary
    const SDL_Color text_color = {0, 0, 0, 255};    // Czarny
    const SDL_Color border_dark = {128, 128, 128, 255};  // Ciemnoszary

    // --- Domyślny styl bazowy ---
    theme.defaultStyle.backgroundColor = bg_color;
    theme.defaultStyle.textColor = text_color;
    theme.defaultStyle.borderColor = border_dark;
    theme.defaultStyle.borderWidth = 0;
    theme.defaultStyle.fontSize = 16;
    theme.defaultStyle.fontName = "assets/fonts/font.ttf";

    // --- Style dla przycisku (Button) ---
    Style button_normal;
    button_normal.backgroundColor = bg_color;
    button_normal.textColor = text_color;
    button_normal.borderColor = border_dark;
    button_normal.borderWidth = 2;

    Style button_hover = button_normal;
    // (Można dodać subtelne podświetlenie, jeśli zajdzie potrzeba)

    Style button_pressed = button_normal;
    button_pressed.borderColor = {0, 0, 0, 255}; // Ciemniejsza ramka po wciśnięciu

    Style button_disabled = button_normal;
    button_disabled.textColor = border_dark; // Szary tekst

    theme.setStyle("Button", ElementState::Normal, button_normal);
    theme.setStyle("Button", ElementState::Hover, button_hover);
    theme.setStyle("Button", ElementState::Pressed, button_pressed);
    theme.setStyle("Button", ElementState::Disabled, button_disabled);

    // --- Style dla Panelu ---
    Style panel_style;
    panel_style.backgroundColor = bg_color;
    panel_style.borderColor = border_dark;
    panel_style.borderWidth = 1;
    theme.setStyle("Panel", ElementState::Normal, panel_style);
    
    // Slider dziedziczy po Panelu, więc otrzyma jego styl
    // Możemy zdefiniować dodatkowe style dla "Slider", jeśli potrzebujemy
    Style slider_style = panel_style;
    theme.setStyle("Slider", ElementState::Normal, slider_style);

    // --- Style dla Etykiety (Label) ---
    Style label_style;
    label_style.textColor = text_color;
    theme.setStyle("Label", ElementState::Normal, label_style);
    theme.setStyle("Label", ElementState::Hover, label_style);
    theme.setStyle("Label", ElementState::Pressed, label_style);
    Style label_disabled = label_style;
    label_disabled.textColor = border_dark;
    theme.setStyle("Label", ElementState::Disabled, label_disabled);

    return theme;
}

const Style& Theme::getDefaultStyle() const {
    return defaultStyle;
}