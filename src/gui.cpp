#include "gui.hpp"
#include "SDL2/SDL.h"


// Implementacja funkcji pomocniczej do tworzenia tekstury z tekstu
SharedTexture createTextTexture(SDL_Renderer* renderer, SharedFont font, const std::string& text, SDL_Color color) {
    if (!renderer || !font || text.empty()) {
        return nullptr;
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font.get(), text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!newTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return {newTexture, SDLTextureDeleter()};
}

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


void GUIElement::addChild(std::unique_ptr<GUIElement> child) {
    if (child && child->m_parent != this) {
        // Jeśli dziecko ma już rodzica, usuń je z listy dzieci tego rodzica
        // (Ta logika może wymagać dostosowania w zależności od tego, czy chcemy pozwalać na przenoszenie dzieci między rodzicami)
        // Na razie zakładamy, że dziecko jest dodawane tylko raz lub przenoszone z nullptr parent
        if (child->m_parent) {
             // W przypadku unique_ptr, usunięcie dziecka z listy rodzica oznacza jego zniszczenie.
             // Ta operacja jest bardziej złożona i może wymagać innej strategii zarządzania dziećmi.
             // Na potrzeby tej refaktoryzacji, upraszczamy i zakładamy, że dziecko nie ma rodzica przy dodawaniu.
             // Jeśli potrzebne jest przenoszenie, należy zaimplementować odpowiednią logikę.
        }
        child->m_parent = this;
        m_children.push_back(std::move(child)); // Przenieś własność do wektora
    }
}

// removeChild nie jest już potrzebne w tej formie przy użyciu unique_ptr,
// ponieważ usunięcie elementu z wektora m_children automatycznie zwalnia pamięć.
// Jeśli potrzebne jest usunięcie dziecka bez niszczenia go (np. przeniesienie do innego rodzica),
// należy zaimplementować inną metodę, która zwraca unique_ptr.

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
            for (auto& child : m_children) {
                if (child) {
                    child->handleEvent(e);
                }
            }
        }
    } else {
        // Dla innych typów zdarzeń, po prostu przekaż je do dzieci (opcjonalnie, w zależności od potrzeb)
        for (auto& child : m_children) {
            if (child) {
                child->handleEvent(e);
            }
        }
    }
}


void Button::handleEvent(SDL_Event& e) {
    // Obsługa zdarzenia puszczenia przycisku myszy w obrębie przycisku
    if (e.type == SDL_MOUSEBUTTONUP) {
        // Pozycja myszy w zdarzeniu jest już absolutna
        if (contains(e.button.x, e.button.y)) {
            triggerOnRelease(); // triggerOnRelease już przekazuje 'this'
        }
    }
    // Przekaż zdarzenie dalej do dzieci (jeśli przycisk ma dzieci, np. tekst)
    GUIElement::handleEvent(e);
}

void GUIElement::render(SDL_Renderer* renderer) {
    // Domyślna implementacja renderowania: renderuj tylko dzieci
    for (auto&& child : m_children) {
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

void Panel::render(SDL_Renderer* renderer)  {
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
