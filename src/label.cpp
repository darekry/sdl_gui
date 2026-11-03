#include "label.hpp"
#include "gui_manager.hpp"
#include "gui.hpp"
#include "constants.hpp"


namespace {

std::vector<std::string> splitLines(std::string_view text) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (true) {
        size_t end = text.find('\n', start);
        if (end == std::string_view::npos) {
            lines.emplace_back(text.substr(start));
            break;
        }
        lines.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
    for (auto& line : lines) {
        while (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
    }
    return lines;
}

} // namespace

void Label::updateLines() {
    m_lines.clear();
    if (!m_text.empty()) {
        m_lines = splitLines(m_text);
    }
}

void Label::recalculateSize() {
    if (m_text.empty()) {
        setSize(0, 0);
        return;
    }
    const auto& resolvedStyle = getComposedStyle(m_state);
    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or(constants::kDefaultFontPath), font_size);
    if (!font) {
        return;
    }

    if (m_lines.size() <= 1) {
        int textWidth = 0, textHeight = 0;
        if (!TTF_GetStringSize(font.get(), m_text.c_str(), m_text.length(), &textWidth, &textHeight)) {
            LOG_DEBUG("Label: TTF_GetStringSize failed: %s", SDL_GetError());
            textWidth = textHeight = 0;
        }
        setSize(textWidth, textHeight);
        return;
    }

    int lineHeight = TTF_GetFontHeight(font.get());
    int maxWidth = 0;
    for (const auto& line : m_lines) {
        if (line.empty()) {
            continue;
        }
        int textWidth = 0, textHeight = 0;
        if (TTF_GetStringSize(font.get(), line.c_str(), line.length(), &textWidth, &textHeight)) {
            maxWidth = std::max(maxWidth, textWidth);
        } else {
            LOG_DEBUG("Label: TTF_GetStringSize failed: %s", SDL_GetError());
        }
    }
    setSize(maxWidth, static_cast<int>(m_lines.size()) * lineHeight);
}

Label::Label(GUIManager& manager, int x, int y, std::string_view text, int font_size)
    : GUIElement(manager, x, y, 0, 0), m_text(text), m_font_size(font_size) {
    updateLines();
    recalculateSize();
    markDirty();
}

void Label::setText(std::string_view text) {
    if (m_text != text) {
        m_text = text;
        updateLines();
        recalculateSize();
        markDirty();
    }
}

void Label::setFontSize(int font_size) {
    if (m_font_size != font_size) {
        m_font_size = font_size;
        recalculateSize();
        markDirty();
    }
}

void Label::draw(SDL_Renderer* renderer) {
    if (m_text.empty()) {
        return;
    }

    const auto& resolvedStyle = getComposedStyle(m_state);
    if (!resolvedStyle.textColor.has_value()) {
        LOG_DEBUG("no text color");
        return;
    }

    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));

    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or(constants::kDefaultFontPath), font_size);
    if (!font) {
        LOG_DEBUG("no font");
        return;
    }

    auto& textureManager = m_manager.getTextureManager();

    auto alignX = [this](int contentWidth) {
        switch (m_h_align) {
            case TextHAlign::Center: return (m_width - contentWidth) / 2;
            case TextHAlign::Right:  return m_width - contentWidth;
            case TextHAlign::Left:
            default:                 return 0;
        }
    };

    if (m_lines.size() <= 1) {
        auto textTexture = textureManager.createTextureFromText(m_text, font, resolvedStyle.textColor.value());
        if (!textTexture) {
            LOG_DEBUG("no shared texture");
            return;
        }

        const int texW = TextureWidth(textTexture.get());
        const int texH = TextureHeight(textTexture.get());
        int y = 0;
        if (m_v_align == TextVAlign::Middle) {
            y = (m_height - texH) / 2;
        } else if (m_v_align == TextVAlign::Bottom) {
            y = m_height - texH;
        }
        SDL_Rect dstRect = {alignX(texW), y, texW, texH};
        RenderTexture(renderer, textTexture.get(), dstRect);
        return;
    }

    int lineHeight = TTF_GetFontHeight(font.get());
    // Multi-line block: vertical alignment applies to the whole block.
    const int blockHeight = static_cast<int>(m_lines.size()) * lineHeight;
    int blockY = 0;
    if (m_v_align == TextVAlign::Middle) {
        blockY = (m_height - blockHeight) / 2;
    } else if (m_v_align == TextVAlign::Bottom) {
        blockY = m_height - blockHeight;
    }

    SDL_Color textColor = resolvedStyle.textColor.value();
    int yOffset = blockY;
    for (const auto& line : m_lines) {
        if (!line.empty()) {
            auto lineTexture = textureManager.createTextureFromText(line, font, textColor);
            if (lineTexture) {
                const int texW = TextureWidth(lineTexture.get());
                SDL_Rect dstRect = {alignX(texW), yOffset, texW, TextureHeight(lineTexture.get())};
                RenderTexture(renderer, lineTexture.get(), dstRect);
            } else {
                LOG_DEBUG("no shared texture");
            }
        }
        yOffset += lineHeight;
    }
}