#include "button.hpp"
#include "gui_manager.hpp"

// Implementacja klasy Button
Button::Button(int x, int y, int width, int height, SharedTexture texture)
    : GUIElement(x, y, width, height) {
    m_texture = texture;
}

bool Button::handleEvent(SDL_Event& e) {
    if (!m_enabled || !m_visible) return false;

    // Najpierw pozwól dzieciom obsłużyć zdarzenie
    if (GUIElement::handleEvent(e)) {
        // Jeśli dziecko obsłużyło zdarzenie, zresetuj stan hover tego przycisku,
        // ponieważ mysz jest nad dzieckiem, a nie bezpośrednio nad przyciskiem.
        m_isHovered = false;
        return true;
    }

    // Następnie obsłuż zdarzenia dla samego przycisku
    bool previousHoverState = m_isHovered;
    
    if (e.type == SDL_MOUSEMOTION) {
        m_isHovered = contains(e.motion.x, e.motion.y);
        if (m_isHovered && !previousHoverState) {
            triggerOnMouseOver();
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_isHovered) {
            triggerOnRelease(); // Używamy m_onClick dla zdarzenia puszczenia przycisku
            return true; // Przycisk obsłużył zdarzenie
        }
    }
    
    // Jeśli zdarzenie to ruch myszy, ale nie zostało obsłużone przez kliknięcie,
    // to wciąż jest to zdarzenie obsłużone na poziomie detekcji hover.
    // Zwracamy false, aby inne elementy mogły również reagować na ruch myszy,
    // ale stan `m_isHovered` jest już zaktualizowany.
    return false;
}

void Button::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }

    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect renderQuad = { absPos.x, absPos.y, m_width, m_height };
 
    // Zmień kolor tła w zależności od stanu najechania
    if (m_isHovered) {
        SDL_SetRenderDrawColor(renderer, 0xC0, 0xC0, 0xC0, 0xFF); // Jasnoszary dla hover
    } else {
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF); // Domyślny szary
    }
    SDL_RenderFillRect(renderer, &renderQuad);
 
    if (m_texture) {
        int texW, texH;
        SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &texW, &texH);
        SDL_Rect textQuad = {
            absPos.x + (m_width - texW) / 2,
            absPos.y + (m_height - texH) / 2,
            texW,
            texH
        };
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &textQuad);
    }
}

void Button::setLabel(const std::string& text, GUIManager& guiManager) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel: Setting label to \"%s\"", text.c_str());
    m_labelText = text;
    FontManager* fontManager = guiManager.getFontManager();
    if (!fontManager) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel ERROR: FontManager is null.");
        return;
    }

    SharedFont font = fontManager->loadFont("assets/fonts/font.ttf", 16);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel ERROR: Failed to load font.");
        return;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel: Font loaded.");

    SDL_Color textColor = { 0, 0, 0, 255 };
    SharedTexture textTexture = guiManager.getTextureManager()->createTextureFromText(
        text,
        font,
        textColor
    );

    if (!textTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel ERROR: Failed to create text texture for \"%s\".", text.c_str());
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Button::setLabel: Text texture created successfully for \"%s\".", text.c_str());
    }
    setTexture(textTexture);
}

void Button::setLabelText(const std::string& text) {
    m_labelText = text;
}