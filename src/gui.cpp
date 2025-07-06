#include "gui.hpp"

#include "SDL2/SDL.h"
#include "gui_manager.hpp" // Dodano, aby mieć definicję GUIManager

// Implementacja klasy GUIElement
GUIElement::GUIElement(GUIManager& manager, int x, int y, int width, int height)
    : m_manager(manager), m_x(x), m_y(y), m_width(width), m_height(height), m_parent(nullptr) {
    m_texture = m_manager.getTextureManager().getDefaultTexture();
}

void GUIElement::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

void GUIElement::setSize(int width, int height) {
    m_width = width;
    m_height = height;
}

// Metoda zwracająca absolutną pozycję elementu
SDL_Point GUIElement::getAbsolutePosition() const {
    SDL_Point pos = {m_x, m_y};
    if (m_parent) {
        SDL_Point parentPos = m_parent->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    return pos;
}

// Metoda sprawdzająca, czy punkt (x, y) znajduje się w obrębie elementu (uwzględniając pozycję rodzica)
bool GUIElement::contains(int x, int y) const {
    SDL_Point absPos = getAbsolutePosition();
    return (x >= absPos.x && x < absPos.x + m_width &&
            y >= absPos.y && y < absPos.y + m_height);
}


void GUIElement::addChild(std::unique_ptr<GUIElement> child) {
    if (child && child->m_parent != this) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }
}

void GUIElement::clearChildren() {
    m_children.clear();
}

void GUIElement::setTexture(SharedTexture texture) {
    m_texture = texture;
}

SharedTexture GUIElement::getLabelTexture() const {
    return m_texture;
}

void GUIElement::setLabel(const std::string& text, int fontSize, SDL_Color color) {
    FontManager& fontManager = m_manager.getFontManager();
    // Używamy domyślnej czcionki - załóżmy, że jest w assets
    SharedFont font = fontManager.loadFont("assets/fonts/font.ttf", fontSize);
    if (font) {
        TextureManager& textureManager = m_manager.getTextureManager();
        m_texture = textureManager.createTextureFromText(text, font, color);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "setLabel failed: Font could not be loaded.");
    }
}

// removeChild nie jest już potrzebne w tej formie przy użyciu unique_ptr,
// ponieważ usunięcie elementu z wektora m_children automatycznie zwalnia pamięć.
// Jeśli potrzebne jest usunięcie dziecka bez niszczenia go (np. przeniesienie do innego rodzica),
// należy zaimplementować inną metodę, która zwraca unique_ptr.


bool GUIElement::handleEvent(SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }
 
    // Przekaż zdarzenie do dzieci w odwrotnej kolejności (od góry do dołu)
    for (auto&& it : m_children) {
        if (it->handleEvent(e)) {
            return true; // Jeśli dziecko obsłużyło zdarzenie, nie propaguj dalej
        }
    }
 
    return false; // Żadne dziecko nie obsłużyło zdarzenia
}

void GUIElement::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }

    if (m_texture) {
        SDL_Point absPos = getAbsolutePosition();
        SDL_Rect renderQuad = { absPos.x, absPos.y, m_width, m_height };
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad);
    }

    // Domyślna implementacja renderowania: renderuj dzieci
    for (auto&& child : m_children) {
        if (child && child->isVisible()) {
            child->render(renderer);
        }
    }
}




