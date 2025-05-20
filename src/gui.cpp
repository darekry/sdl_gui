#include "gui.hpp"
#include "SDL2/SDL.h"

// Implementacja klasy GUIElement
GUIElement::GUIElement(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_parent(nullptr) {}

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

void GUIElement::addChild(GUIElement* child) {
    if (child && child->m_parent != this) {
        if (child->m_parent) {
            child->m_parent->removeChild(child);
        }
        m_children.push_back(child);
        child->m_parent = this;
    }
}

void GUIElement::removeChild(GUIElement* child) {
    for (size_t i = 0; i < m_children.size(); ++i) {
        if (m_children[i] == child) {
            m_children.erase(m_children.begin() + i);
            child->m_parent = nullptr;
            break;
        }
    }
}

// Implementacja klasy Button
Button::Button(int x, int y, int width, int height, SharedTexture texture)
    : GUIElement(x, y, width, height), m_texture(texture) {}

Button::~Button() {
    // SharedTexture automatycznie zarządza pamięcią tekstury, więc nie ma potrzeby ręcznego zwalniania.
}

void Button::setTexture(SharedTexture texture) {
    m_texture = texture;
}

void GUIElement::handleEvent(SDL_Event& e) {
    // Przekaż zdarzenie do dzieci, jeśli zdarzenie myszy miało miejsce w obrębie elementu
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONUP) {
        int mouseX, mouseY;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            mouseX = e.button.x;
            mouseY = e.button.y;
        } else if (e.type == SDL_MOUSEBUTTONUP) {
             mouseX = e.button.x;
             mouseY = e.button.y;
        }
        else { // SDL_MOUSEMOTION
            mouseX = e.motion.x;
            mouseY = e.motion.y;
        }

        // Sprawdź, czy zdarzenie myszy miało miejsce w obrębie tego elementu przed przekazaniem do dzieci
        if (contains(mouseX, mouseY)) {
            for (GUIElement* child : m_children) {
                if (child) {
                    child->handleEvent(e);
                }
            }
        }
    } else {
        // Dla innych typów zdarzeń, po prostu przekaż je do dzieci (opcjonalnie, w zależności od potrzeb)
        for (GUIElement* child : m_children) {
            if (child) {
                child->handleEvent(e);
            }
        }
    }
}

void Button::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONUP) {
        // Sprawdź, czy zdarzenie puszczenia przycisku myszy miało miejsce w obrębie przycisku
        // Pozycja myszy w zdarzeniu jest już absolutna
        if (contains(e.button.x, e.button.y)) {
            triggerOnRelease(); // triggerOnRelease już przekazuje 'this'
        }
    }
    // Można dodać obsługę innych zdarzeń, np. SDL_MOUSEMOTION dla efektu najechania
    // Przekaż zdarzenie dalej do dzieci (jeśli przycisk ma dzieci, np. tekst)
    GUIElement::handleEvent(e);
}

void GUIElement::render(SDL_Renderer* renderer) {
    // Domyślna implementacja renderowania: renderuj tylko dzieci
    for (GUIElement* child : m_children) {
        if (child) {
            child->render(renderer);
        }
    }
}

void Button::render(SDL_Renderer* renderer) {
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect renderQuad = { absPos.x, absPos.y, m_width, m_height };

    if (m_texture) { // Sprawdź, czy shared_ptr nie jest pusty
        // Renderuj teksturę, jeśli istnieje
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad); // Użyj .get() aby uzyskać surowy wskaźnik
    } else {
        // Renderuj prostokąt w jednolitym kolorze, jeśli brak tekstury
        // Można dodać pole koloru do klasy Button, na razie używamy domyślnego szarego
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF); // Szary
        SDL_RenderFillRect(renderer, &renderQuad);
        // Opcjonalnie: przywróć poprzedni kolor rysowania, jeśli jest to konieczne
        // SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);
        // SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);
    }

    // Renderuj dzieci (przyciski mogą mieć dzieci, np. tekst na przycisku)
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

void Panel::render(SDL_Renderer* renderer) {
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
