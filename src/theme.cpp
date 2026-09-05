#include "theme.hpp"
#include "theme_presets.hpp"

void Theme::setStyle(ComponentType type, ElementState state, Style style) {
    m_styles[typeIdx(type)][stateIdx(state)] = std::move(style);
    ++m_epoch;
}

Style Theme::getStyle(ComponentType type, ElementState state) const {
    const auto& styles = m_styles[typeIdx(type)];
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
    return m_defaultStyle;
}

void Theme::setStyle(ComponentType type, Style style) {
    setStyle(type, ElementState::Normal, std::move(style));
}

Style Theme::getStyle(ComponentType type) const {
    return getStyle(type, ElementState::Normal);
}

// === Default style ===

void Theme::setDefaultStyle(Style style) {
    m_defaultStyle = std::move(style);
    ++m_epoch;
}

const Style& Theme::getDefaultStyle() const {
    return m_defaultStyle;
}

// === Factory ===

Theme Theme::createDefaultTheme() {
    return ThemePresets::createWin9xTheme();
}

Theme Theme::createWindows95Theme() {
    return ThemePresets::createWindows95Theme();
}
