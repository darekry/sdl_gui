#include "theme.hpp"

void Theme::setStyle(const std::string& type, Style style) {
    m_styles[type] = std::move(style);
}

Style Theme::getStyle(const std::string& type) const {
    auto it = m_styles.find(type);
    if (it != m_styles.end()) {
        Style specificStyle = it->second; // Kopia
        specificStyle.mergeWith(m_defaultStyle);
        return specificStyle;
    }
    return m_defaultStyle;
}

void Theme::setDefaultStyle(Style style) {
    m_defaultStyle = std::move(style);
}

const Style& Theme::getDefaultStyle() const {
    return m_defaultStyle;
}

Theme Theme::createDefaultTheme() {
    Theme theme;

    // --- Kolory motywu "Windows 95/98" ---
    const SDL_Color bg_color = {212, 208, 200, 255}; // Jasnoszary
    const SDL_Color text_color = {0, 0, 0, 255};    // Czarny
    const SDL_Color border_dark = {128, 128, 128, 255};  // Ciemnoszary

    // --- Domyślny styl bazowy ---
    Style defaultStyle;
    defaultStyle.backgroundColor = bg_color;
    defaultStyle.textColor = text_color;
    defaultStyle.borderColor = border_dark;
    defaultStyle.borderWidth = 0;
    defaultStyle.fontSize = 16;
    defaultStyle.fontName = "assets/fonts/font.ttf";
    theme.setDefaultStyle(defaultStyle);

    // --- Style dla przycisku (Button) ---
    Style button_style;
    button_style.borderWidth = 2;
    button_style.borderRadius = 4;  // Zaokrąglone przyciski
    theme.setStyle("Button", button_style);

    // --- Style dla Panelu ---
    Style panel_style;
    panel_style.borderWidth = 1;
    theme.setStyle("Panel", panel_style);
    
    // Slider dziedziczy po Panelu, więc otrzyma jego styl
    // Możemy zdefiniować dodatkowe style dla "Slider", jeśli potrzebujemy
    theme.setStyle("Slider", panel_style);

    // --- Style dla pola tekstowego (TextInput) ---
    Style textInputStyle;
    textInputStyle.backgroundColor = {255, 255, 255, 255}; // Białe tło
    textInputStyle.borderRadius = 2;  // Delikatne zaokrąglenie
    theme.setStyle("TextInput", textInputStyle);

    // --- Style dla pola tekstowego wieloliniowego (TextArea) ---
    Style textAreaStyle;
    textAreaStyle.borderWidth=2;
    textAreaStyle.backgroundColor = {255, 255, 255, 255}; // Białe tło
    textAreaStyle.borderRadius = 2;  // Delikatne zaokrąglenie
    theme.setStyle("TextArea", textAreaStyle);

    // --- Style dla Etykiety (Label) ---
    // Etykieta nie ma żadnych specjalnych stylów, więc będzie w pełni dziedziczyć z domyślnego.

    // --- Style dla StringGrid ---
    Style stringGridStyle;
    stringGridStyle.backgroundColor = {255, 255, 255, 255};  // białe tło komórek
    stringGridStyle.textColor = {0, 0, 0, 255};              // czarny tekst
    stringGridStyle.borderColor = {200, 200, 200, 255};      // linie siatki
    stringGridStyle.borderWidth = 1;
    theme.setStyle("StringGrid", stringGridStyle);
    
    return theme;
}
