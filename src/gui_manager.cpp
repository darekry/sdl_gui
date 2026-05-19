#include "SDL2/SDL.h"
#include "gui_manager.hpp"
#include "gui.hpp"
#include "timer_manager.hpp"
#include "panel.hpp"
#include "label.hpp"

import std.compat;

static constexpr int DEFAULT_FONT_SIZE = 24;
static constexpr int TOOLTIP_FONT_SIZE = 14;
static constexpr int TOOLTIP_PADDING = 5;
static const SDL_Color TOOLTIP_BG_COLOR = {.r=255, .g=255, .b=225, .a=255};

GUIManager::GUIManager(SDL_Renderer* renderer)
    : tooltipElement(nullptr), m_renderer(renderer), m_textureManager(renderer), m_theme(Theme::createDefaultTheme()) {
    timerManager = std::make_unique<TimerManager>();
    animation_manager = std::make_unique<AnimationManager>();

    // Załaduj domyślną czcionkę
    m_fontManager.loadDefaultFont("assets/fonts/font.ttf", DEFAULT_FONT_SIZE);

    // Utwórz domyślną teksturę zastępczą
    m_textureManager.createDefaultTexture(m_renderer, m_fontManager, "No Texture");
}

GUIManager::~GUIManager() = default;


GUIElement* GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        auto* raw_ptr = element.get();
        m_elements.push_back(std::move(element)); // Przenieś własność do wektora
        return raw_ptr;
    }
    return nullptr;
}

bool GUIManager::processEvent(const SDL_Event& event) {
    // 1. Zdarzenia myszy
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        if (m_mouseCaptureElement) {
            // Jeśli element przechwycił mysz, wysyłaj zdarzenia tylko do niego
            return m_mouseCaptureElement->handleEvent(event);
        }
    }
    // 2. Zdarzenia klawiatury
    else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || event.type == SDL_TEXTINPUT) {
        if (m_keyboardFocusElement) {
            // Jeśli element ma fokus klawiatury, wysyłaj zdarzenia tylko do niego
            return m_keyboardFocusElement->handleEvent(event);
        }
        // Jeśli żaden element nie ma fokusu, zdarzenia klawiatury są ignorowane przez GUI
        return false;
    }

    // 3. Standardowa propagacja dla zdarzeń nieprzechwyconych
    if (tooltipElement && tooltipElement->isVisible() && tooltipElement->handleEvent(event)) {
        return true;
    }

    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if ((*it)->handleEvent(event)) {
            return true;
        }
    }

    // Specjalna obsługa kliknięcia poza elementami z focusem
    if (event.type == SDL_MOUSEBUTTONDOWN && m_keyboardFocusElement) {
        bool click_on_focusable = false;
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            if ((*it)->contains(event.button.x, event.button.y)) {
                // To jest uproszczenie, idealnie byłoby sprawdzić, czy kliknięty element
                // faktycznie może otrzymać fokus.
                click_on_focusable = true;
                break;
            }
        }
        if (!click_on_focusable) {
            setKeyboardFocus(nullptr);
        }
    }

    if (cursor) {
        cursor->handleEvent(event);
    }

    return false;
}

void GUIManager::update() {
    // Zaktualizuj timery i animacje
    timerManager->update();
    animation_manager->update();

}

void GUIManager::render() {
    for (const auto& element : m_elements) {
        if (element && !element->isOverlay()) {
            element->render(m_renderer);
        }
    }
    
    if (tooltipElement) {
        tooltipElement->render(m_renderer);
    }
    
    for (const auto& element : m_elements) {
        if (element && element->isOverlay()) {
            element->renderOverlay(m_renderer);
        }
    }

    if (m_keyboardFocusElement) {
        m_keyboardFocusElement->renderOverlay(m_renderer);
    }

    if (cursor) {
        cursor->renderOverlay(m_renderer);
    }
}

void GUIManager::cleanup() {
    if (tooltipElement && tooltipElement->isMarkedForDeletion()) {
        tooltipElement.reset();
    }

    // Sprawdź, czy elementy z fokusem/przechwyceniem nie są usuwane
    if (m_keyboardFocusElement && m_keyboardFocusElement->isMarkedForDeletion()) {
        setKeyboardFocus(nullptr);
    }
    if (m_mouseCaptureElement && m_mouseCaptureElement->isMarkedForDeletion()) {
        releaseMouse();
    }
    
    // Najpierw rekurencyjnie wywołaj cleanup dla wszystkich elementów
    for (const auto& element : m_elements) {
        if (element) {
            element->cleanup();
        }
    }
// Następnie usuń oznaczone elementy z głównego kontenera
size_t total_removed_count = 0;
for (const auto& element : m_elements) {
    if (element && element->isMarkedForDeletion()) {
        total_removed_count += 1 + element->countDescendants();
    }
}

auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                              [](const std::unique_ptr<GUIElement>& element) {
    return element->isMarkedForDeletion();
});

std::size_t prefix_distance = static_cast<std::size_t>(std::distance(m_elements.begin(), new_end));
if (prefix_distance < m_elements.size())
{
    m_elements.erase(new_end, m_elements.end());
}

if (total_removed_count > 0) {
    LOG_DEBUG("GUIManager::cleanup(): Removed %zu elements in total.", total_removed_count);
}
}

void GUIManager::showTooltip(GUIElement* target, const std::string& text) {
    if (!target) return;

    const int fontSize = TOOLTIP_FONT_SIZE;
    const int padding = TOOLTIP_PADDING;

    // Precyzyjne obliczanie rozmiaru tekstu
    int textWidth = 0;
    int textHeight = 0;
    m_fontManager.getTextSize(text, "assets/fonts/font.ttf", fontSize, &textWidth, &textHeight);
    
    auto targetPos = target->getAbsolutePosition();
    int posX = targetPos.x;
    int posY = targetPos.y + target->getHeight();

    // Utwórz panel
    auto panel = std::make_unique<Panel>(*this, posX, posY, (textWidth + (2 * padding)), (textHeight + (2 * padding)));
    panel->setBackgroundColor(ElementState::Normal, TOOLTIP_BG_COLOR);
    panel->setBorder(ElementState::Normal, {0, 0, 0, 255}, 1);

    // Utwórz etykietę i dodaj ją do panelu
    auto label = std::make_unique<Label>(*this, padding, padding, text, fontSize);
    panel->addChild(std::move(label));

    tooltipElement = std::move(panel);
}

void GUIManager::hideTooltip() {
    tooltipElement.reset();
}


TimerManager* GUIManager::getTimerManager() {
    return timerManager.get();
}

void GUIManager::setTheme(Theme theme) {
    m_theme = std::move(theme);
    for (const auto& element : m_elements) {
        if (element) {
            element->markDirtyRecursively();
        }
    }
}

Theme& GUIManager::getTheme() {
    return m_theme;
}

void GUIManager::captureMouse(GUIElement* element) {
    if (m_mouseCaptureElement && m_mouseCaptureElement != element) {
        m_mouseCaptureElement->onMouseCaptureLost();
    }
    m_mouseCaptureElement = element;
    if (m_mouseCaptureElement) {
        m_mouseCaptureElement->onMouseCaptureGained();
    }
}

void GUIManager::releaseMouse() {
    if (m_mouseCaptureElement) {
        m_mouseCaptureElement->onMouseCaptureLost();
    }
    m_mouseCaptureElement = nullptr;
}

void GUIManager::setKeyboardFocus(GUIElement* element) {
    if (m_keyboardFocusElement == element) {
        return;
    }
    if (m_keyboardFocusElement) {
        m_keyboardFocusElement->onFocusLost();
    }
    m_keyboardFocusElement = element;
    if (m_keyboardFocusElement) {
        m_keyboardFocusElement->onFocusGained();
    }
}

GUIElement* GUIManager::getKeyboardFocus() const {
    return m_keyboardFocusElement;
}

AnimationManager* GUIManager::getAnimationManager() {
    return animation_manager.get();
}

void GUIManager::setCursor(std::unique_ptr<Cursor> new_cursor) {
    cursor = std::move(new_cursor);
}

GUIElement* GUIManager::findElementAt(int x, int y) {
    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if (*it) {
            if (auto* element = (*it)->findElementAt(x, y)) {
                return element;
            }
        }
    }
    return nullptr;
}
