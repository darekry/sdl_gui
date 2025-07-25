#include "gui.hpp"

#include "gui_manager.hpp" // Dodano, aby mieć definicję GUIManager
#include "SDL2/SDL.h"
import std.compat;

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

void GUIElement::setParent(GUIElement* parent) {
    m_parent = parent;
}

void GUIElement::getSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void GUIElement::setClipChildren(bool clip) {
    m_clip_children = clip;
}

// Metoda zwracająca absolutną pozycję elementu
SDL_Point GUIElement::getAbsolutePosition() const {
    auto pos = SDL_Point{m_x, m_y};
    if (m_parent) {
        auto parentPos = m_parent->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    return pos;
}

// Metoda sprawdzająca, czy punkt (x, y) znajduje się w obrębie elementu (uwzględniając pozycję rodzica)
bool GUIElement::contains(int x, int y) const {
    auto absPos = getAbsolutePosition();
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

void GUIElement::setTexture(const SharedTexture& texture) {
    m_texture = texture;
}

SharedTexture GUIElement::getLabelTexture() const {
    return m_texture;
}

void GUIElement::setLabel(std::string_view text, int fontSize, const SDL_Color& color) {
    auto& fontManager = m_manager.getFontManager();
    // Używamy domyślnej czcionki - załóżmy, że jest w assets
    auto font = fontManager.loadFont("assets/fonts/font.ttf", fontSize);
    if (font) {
        auto& textureManager = m_manager.getTextureManager();
        m_texture = textureManager.createTextureFromText(text, font, color);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "setLabel failed: Font could not be loaded.");
    }
}

void GUIElement::setTooltip(const std::string& text) {
    this->tooltip = text;
}

// removeChild nie jest już potrzebne w tej formie przy użyciu unique_ptr,
// ponieważ usunięcie elementu z wektora m_children automatycznie zwalnia pamięć.
// Jeśli potrzebne jest usunięcie dziecka bez niszczenia go (np. przeniesienie do innego rodzica),
// należy zaimplementować inną metodę, która zwraca unique_ptr.

bool GUIElement::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    // Przekaż zdarzenie do dzieci w odwrotnej kolejności (od góry do dołu)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true; // Jeśli dziecko obsłużyło zdarzenie, nie propaguj dalej
        }
    }
    
    if (e.type == SDL_MOUSEMOTION) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool currentlyHovered = contains(mouseX, mouseY);

        if (currentlyHovered && !m_isHovered) { // Kursor wjechał na element
            m_isHovered = true;
            if (!tooltip.empty()) {
                tooltipTimerId = startTimer(500, true, [this](GUIElement* self) {
                    if (self) {
                       self->m_manager.showTooltip(self, self->tooltip);
                    }
                });
            }
        } else if (!currentlyHovered && m_isHovered) { // Kursor opuścił element
            m_isHovered = false;
            if (tooltipTimerId != 0) {
                stopTimer(tooltipTimerId);
                tooltipTimerId = 0;
            }
            m_manager.hideTooltip();
        }
    }
 
    return false; // Żadne dziecko nie obsłużyło zdarzenia
}


void GUIElement::render() {
    if (!m_visible) {
        return;
    }
    auto* renderer = m_manager.getRenderer();
    auto old_clip_rect = SDL_Rect{};
    bool clipping_was_active = false;

    if (m_clip_children) {
        clipping_was_active = true;
        SDL_RenderGetClipRect(renderer, &old_clip_rect);
        auto abs_pos = getAbsolutePosition();
        auto new_clip_rect = SDL_Rect{abs_pos.x, abs_pos.y, m_width, m_height};

        // Jeżeli istnieje już jakiś obszar przycinania, nowy musi być jego częścią
        if (old_clip_rect.w != 0 || old_clip_rect.h != 0) {
            SDL_IntersectRect(&old_clip_rect, &new_clip_rect, &new_clip_rect);
        }
        SDL_RenderSetClipRect(renderer, &new_clip_rect);
    }

    // Wywołanie specyficznego rysowania dla danego elementu
    draw();

    // Renderowanie dzieci z już ustawionym (i być może zawężonym) obszarem przycinania
    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render();
        }
    }

    // Przywróć poprzedni obszar przycinania_
    if (clipping_was_active) {
        SDL_RenderSetClipRect(renderer, &old_clip_rect);
    }
}

void GUIElement::draw() {
    // Domyślna implementacja rysowania: narysuj tło (teksturę), jeśli istnieje.
    // Klasy pochodne mogą to rozszerzyć lub zastąpić.
    if (m_texture) {
        auto* renderer = m_manager.getRenderer();
        auto absPos = getAbsolutePosition();
        auto renderQuad = SDL_Rect{absPos.x, absPos.y, m_width, m_height};
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad);
    }
}

void GUIElement::markForDeletion() {
    m_isMarkedForDeletion = true;
}

bool GUIElement::isMarkedForDeletion() const {
    return m_isMarkedForDeletion;
}

void GUIElement::cleanup() {
    // Najpierw rekurencyjnie wywołaj cleanup dla wszystkich dzieci
    for (const auto& child : m_children) {
        if (child) {
            child->cleanup();
        }
    }

    // Następnie usuń oznaczone dzieci z tego kontenera
    auto new_end = std::remove_if(m_children.begin(), m_children.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });

    m_children.erase(new_end, m_children.end());
}
uint32_t GUIElement::startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback) {
    if (m_manager.getTimerManager()) {
        // Poprawiona kolejność argumentów: target, delay, singleShot, callback
        return m_manager.getTimerManager()->addTimer(this, delay, singleShot, [callback](GUIElement* target)
        {
            callback(target);
        });
    }
    return 0;
}


void GUIElement::stopTimer(uint32_t timerId) {
    if (m_manager.getTimerManager()) {
        m_manager.getTimerManager()->removeTimer(timerId);
    }
}

