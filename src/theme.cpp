#include "theme.hpp"
#include "theme_presets.hpp"

void Theme::setStyle(std::string_view type, ElementState state, Style style) {
    auto& arr = m_typeStyles[std::string(type)];
    arr[stateIdx(state)] = std::move(style);
}

Style Theme::getStyle(std::string_view type, ElementState state) const {
    auto it = m_typeStyles.find(type);
    if (it != m_typeStyles.end()) {
        const auto& styles = it->second;
        size_t idx = stateIdx(state);
        if (styles[idx].has_value()) {
            Style result = *styles[idx];
            result.mergeWith(m_defaultStyle);
            return result;
        }

        size_t normalIdx = stateIdx(ElementState::Normal);
        if (styles[normalIdx].has_value()) {
            Style result = *styles[normalIdx];
            result.mergeWith(m_defaultStyle);
            return result;
        }
    }

    return m_defaultStyle;
}

void Theme::setStyle(std::string_view type, Style style) {
    setStyle(type, ElementState::Normal, std::move(style));
}

Style Theme::getStyle(std::string_view type) const {
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
    return ThemePresets::createWin9xTheme();
}