#include "gui.hpp"
#include "SDL2/SDL.h"
#include "gui_manager.hpp" // Dodano, aby mieć definicję GUIManager

// Implementacja klasy GUIElement
GUIElement::GUIElement(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_parent(nullptr), m_guiManager(nullptr), m_texture(nullptr) {}

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
        // Propaguj wskaźnik do GUIManager do nowego dziecka
        child->setGUIManager(this->m_guiManager);
        m_children.push_back(std::move(child));
    }
}

void GUIElement::clearChildren() {
    m_children.clear();
}
void GUIElement::setGUIManager(GUIManager* manager) {
    m_guiManager = manager;
    // Propaguj wskaźnik do wszystkich istniejących dzieci
    for (auto& child : m_children) {
        if (child) {
            child->setGUIManager(manager);
        }
    }
}

void GUIElement::setTexture(SharedTexture texture) {
    m_texture = texture;
}

GUIManager* GUIElement::getGUIManager() const {
    return m_guiManager;
}

// removeChild nie jest już potrzebne w tej formie przy użyciu unique_ptr,
// ponieważ usunięcie elementu z wektora m_children automatycznie zwalnia pamięć.
// Jeśli potrzebne jest usunięcie dziecka bez niszczenia go (np. przeniesienie do innego rodzica),
// należy zaimplementować inną metodę, która zwraca unique_ptr.

// Implementacja klasy Button
Button::Button(int x, int y, int width, int height, SharedTexture texture)
    : GUIElement(x, y, width, height) {
    m_texture = texture;
}

bool GUIElement::handleEvent(SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    // Przekaż zdarzenie do dzieci w odwrotnej kolejności (od góry do dołu)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true; // Jeśli dziecko obsłużyło zdarzenie, nie propaguj dalej
        }
    }

    return false; // Żadne dziecko nie obsłużyło zdarzenia
}


bool Button::handleEvent(SDL_Event& e) {
    if (!m_enabled) return false;

    // Najpierw sprawdź, czy dzieci obsłużyły zdarzenie
    if (GUIElement::handleEvent(e)) {
        return true;
    }

    // Jeśli nie, sprawdź, czy ten przycisk powinien obsłużyć zdarzenie
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            triggerOnRelease();
            return true; // Zdarzenie obsłużone
        }
    }
    return false;
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


void Button::render(SDL_Renderer* renderer) {
    if (!m_visible) return;

    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect renderQuad = { absPos.x, absPos.y, m_width, m_height };

    if (m_texture) {
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad);
    } else {
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF); // Szary
        SDL_RenderFillRect(renderer, &renderQuad);
    }

    // Renderuj dzieci
    GUIElement::render(renderer);
}

// Implementacja klasy Panel
Panel::Panel(int x, int y, int width, int height)
    : GUIElement(x, y, width, height) {
    // Dodatkowa inicjalizacja dla Panelu, jeśli potrzebna
}

void Panel::setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    m_borderColor = {r, g, b, a};
}

void Panel::setBorderThickness(int thickness) {
    m_borderThickness = thickness;
}
void Panel::render(SDL_Renderer* renderer)  {
    if (!m_visible) {
        return;
    }
    // Pobierz absolutną pozycję panelu
    SDL_Point absPos = getAbsolutePosition();

    // Ustaw kolor rysowania na kolor obramowania
    SDL_SetRenderDrawColor(renderer, m_borderColor.r, m_borderColor.g, m_borderColor.b, m_borderColor.a);

    // Narysuj obramowanie
    for (int i = 0; i < m_borderThickness; ++i) {
        SDL_Rect borderRect = {absPos.x + i, absPos.y + i, m_width - 2 * i, m_height - 2 * i};
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    // Przywróć domyślny kolor rysowania (opcjonalnie, jeśli jest to konieczne)
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Czarny

    // Renderuj dzieci panelu
    GUIElement::render(renderer);
}
