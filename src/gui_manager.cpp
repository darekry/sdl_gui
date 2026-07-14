#include <SDL3/SDL.h>
#include "gui_manager.hpp"
#include "gui.hpp"
#include "timer_manager.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "constants.hpp"
#include "std.hpp"

static constexpr int DEFAULT_FONT_SIZE = 24;
static constexpr int TOOLTIP_FONT_SIZE = 14;
static constexpr int TOOLTIP_PADDING = 5;
static const SDL_Color TOOLTIP_BG_COLOR = {.r=255, .g=255, .b=225, .a=255};

static bool isDescendantOf(GUIElement* descendant, GUIElement* ancestor) {
    if (!descendant || !ancestor) return false;
    GUIElement* current = descendant->getParent();
    while (current) {
        if (current == ancestor) return true;
        current = current->getParent();
    }
    return false;
}

GUIManager::GUIManager(SDL_Renderer* renderer)
    : tooltipElement(nullptr), m_renderer(renderer), m_textureManager(renderer), m_theme(Theme::createDefaultTheme()) {
    timerManager = std::make_unique<TimerManager>();
    animation_manager = std::make_unique<AnimationManager>();

    Uint64 t0 = SDL_GetTicks();
    m_fontManager.loadDefaultFont(constants::kDefaultFontPath, DEFAULT_FONT_SIZE);
    LOG_INFO("GUIManager", "loadDefaultFont: {}ms", SDL_GetTicks() - t0);

    t0 = SDL_GetTicks();
    m_textureManager.createDefaultTexture(m_renderer, m_fontManager, "No Texture");
    LOG_INFO("GUIManager", "createDefaultTexture: {}ms", SDL_GetTicks() - t0);
}

GUIManager::~GUIManager() = default;


GUIElement* GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        auto* raw_ptr = element.get();
        
        for (const auto& e : m_elements) {
            if (e.get() == raw_ptr) {
                LOG_ERROR("GUIManager", "addElement() - element already exists in m_elements! ptr={}", static_cast<const void*>(raw_ptr));
                return nullptr;
            }
        }
        
        registerElement(raw_ptr);
        m_elements.push_back(std::move(element));
        return raw_ptr;
    }
    return nullptr;
}

std::unique_ptr<GUIElement> GUIManager::detachElement(GUIElement* element) {
    if (!element) return nullptr;
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (it->get() == element) {
            auto detached = std::move(*it);
            m_elements.erase(it);
            return detached;
        }
    }
    return nullptr;
}

bool GUIManager::processEvent(const SDL_Event& event) {
    // 1. Zdarzenia myszy
    if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (m_mouseCaptureElement) {
            // Jeśli element przechwycił mysz, wysyłaj zdarzenia tylko do niego
            return m_mouseCaptureElement->handleEvent(event);
        }
    }
    // 2. Zdarzenia klawiatury
    else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_TEXT_INPUT) {
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
            focusNextElement(!(event.key.mod & SDL_KMOD_SHIFT));
            return true;
        }
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
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && m_keyboardFocusElement) {
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
        if (element && !element->isOverlay() && !element->isMarkedForDeletion()) {
            element->render(m_renderer);
        }
    }

    if (tooltipElement && !tooltipElement->isMarkedForDeletion()) {
        tooltipElement->render(m_renderer);
    }

    for (const auto& element : m_elements) {
        if (element && element->isOverlay() && !element->isMarkedForDeletion()) {
            element->renderOverlay(m_renderer);
        }
    }

    // Render overlay for keyboard focus element (e.g., TextInput cursor/selection)
    // Only render if element is not hidden behind a modal overlay
    if (m_keyboardFocusElement && !m_keyboardFocusElement->isMarkedForDeletion()) {
        auto* overlay = getActiveOverlay();
        if (!overlay || isDescendantOf(m_keyboardFocusElement, overlay)) {
            m_keyboardFocusElement->renderOverlay(m_renderer);
        }
    }

    if (cursor) {
        cursor->renderOverlay(m_renderer);
    }
}

void GUIManager::cleanup() {
  //  LOG_DEBUG("GUIManager::cleanup() - ENTER, m_elements.size = %zu", m_elements.size());
    
    if (tooltipElement && tooltipElement->isMarkedForDeletion()) {
        LOG_DEBUG("GUIManager::cleanup() - clearing tooltipElement");
        tooltipElement.reset();
    }

    auto hasAncestorMarkedForDeletion = [](GUIElement* element) {
        if (!element) return false;
        GUIElement* current = element;
        while (current) {
            if (current->isMarkedForDeletion()) return true;
            current = current->getParent();
        }
        return false;
    };

    // Check keyboard/mouse focus BEFORE removing elements
    // Focus element might be a child of an element marked for deletion
    if (m_keyboardFocusElement && 
        (m_keyboardFocusElement->isMarkedForDeletion() || hasAncestorMarkedForDeletion(m_keyboardFocusElement))) {
        LOG_DEBUG("GUIManager::cleanup() - clearing keyboardFocus (marked for deletion)");
        setKeyboardFocus(nullptr);
    }
    if (m_mouseCaptureElement && 
        (m_mouseCaptureElement->isMarkedForDeletion() || hasAncestorMarkedForDeletion(m_mouseCaptureElement))) {
        LOG_DEBUG("GUIManager::cleanup() - releasing mouse (marked for deletion)");
        releaseMouse();
    }
    
  //  LOG_DEBUG("GUIManager::cleanup() - calling cleanup on elements");
    for (const auto& element : m_elements) {
        if (element) {
            element->cleanup();
        }
    }
    
    size_t total_removed_count = 0;
    for (const auto& element : m_elements) {
        if (element && element->isMarkedForDeletion()) {
            total_removed_count += 1 + element->countDescendants();
        }
    }
    (void)total_removed_count;
    
  //  LOG_DEBUG("GUIManager::cleanup() - total elements to remove: %zu", total_removed_count);

    // Elements to be removed (and their children via unique_ptr destructor)
    // After erase, all children will be destroyed too
    // So we need to clear focus/capture if they point to any descendant of removed elements
    
    auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element && element->isMarkedForDeletion();
    });

    std::size_t prefix_distance = static_cast<std::size_t>(std::distance(m_elements.begin(), new_end));
    if (prefix_distance < m_elements.size())
    {
        LOG_DEBUG("GUIManager::cleanup() - erasing {} elements from vector", m_elements.size() - prefix_distance);
        m_elements.erase(new_end, m_elements.end());
    }
    
    // After erase, focus/capture pointers might be invalid (pointing to destroyed children)
    // We already checked hasAncestorMarkedForDeletion before, which should have cleared them
    // But double-check: if focus/capture still exists, it should not be null
    // SAFELY check if focus/capture is null (don't access the pointer)
    // Note: We cannot safely access m_keyboardFocusElement here if it was destroyed
    // The hasAncestorMarkedForDeletion check above should have handled it
    
 //   LOG_DEBUG("GUIManager::cleanup() - EXIT, m_elements.size = %zu", m_elements.size());
}

void GUIManager::showTooltip(GUIElement* target, const std::string& text) {
    if (!target) return;

    const int fontSize = TOOLTIP_FONT_SIZE;
    const int padding = TOOLTIP_PADDING;

    int textWidth = 0;
    int textHeight = 0;
    m_fontManager.getTextSize(text, constants::kDefaultFontPath, fontSize, &textWidth, &textHeight);
    
    auto targetPos = target->getAbsolutePosition();
    int posX = targetPos.x;
    int posY = targetPos.y + target->getHeight();
    int panelWidth = textWidth + (2 * padding);
    int panelHeight = textHeight + (2 * padding);

    if (!m_tooltipPanel) {
        m_tooltipPanel = std::make_unique<Panel>(*this, posX, posY, panelWidth, panelHeight);
        m_tooltipPanel->setBackgroundColor(ElementState::Normal, TOOLTIP_BG_COLOR);
        m_tooltipPanel->setBorder(ElementState::Normal, {0, 0, 0, 255}, 1);
        
        auto label = std::make_unique<Label>(*this, padding, padding, "", fontSize);
        m_tooltipLabel = label.get();
        m_tooltipPanel->addChild(std::move(label));
    }
    
    m_tooltipPanel->setPosition(posX, posY);
    m_tooltipPanel->setSize(panelWidth, panelHeight);
    
    if (m_tooltipLabel) {
        m_tooltipLabel->setText(text);
    }
    
    m_tooltipPanel->setVisible(true);
    tooltipElement = std::move(m_tooltipPanel);
}

void GUIManager::hideTooltip() {
    if (tooltipElement) {
        GUIElement* raw = tooltipElement.release();
        m_tooltipPanel.reset(static_cast<Panel*>(raw));
        m_tooltipPanel->setVisible(false);
    }
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
    if (cursor) {
        registerElement(cursor.get());
    }
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

bool GUIManager::isElementAlive(GUIElement* element) const {
    if (!element) return false;
    return m_liveElements.contains(element);
}

void GUIManager::registerElement(GUIElement* element) {
    if (element) {
        m_liveElements.insert(element);
        LOG_DEBUG("GUIManager::registerElement() - registered {}, total = {}", static_cast<const void*>(element), m_liveElements.size());
    }
}

void GUIManager::unregisterElement(GUIElement* element) {
    if (element) {
        m_liveElements.erase(element);
        LOG_DEBUG("GUIManager::unregisterElement() - unregistered {}, total = {}", static_cast<const void*>(element), m_liveElements.size());
    }
}

// === Resize handling ===

void GUIManager::handleResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    // Update all top-level elements with anchors
    for (const auto& element : m_elements) {
        if (element && element->hasAnchor()) {
            element->updateLayout(width, height);
        }
    }
    
    // Call custom resize callback if set
    if (m_resizeCallback) {
        m_resizeCallback(width, height);
    }
}

void GUIManager::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = callback;
}

void GUIManager::getWindowSize(int& width, int& height) const {
    width = m_windowWidth;
    height = m_windowHeight;
}

void GUIManager::setWindowSize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
}

void GUIManager::collectFocusableElements(std::vector<GUIElement*>& out) const {
    std::function<void(GUIElement*)> collect = [&](GUIElement* element) {
        if (!element || !element->isVisible()) return;
        if (element->canGetKeyboardFocus()) {
            out.push_back(element);
        }
        for (const auto& child : element->getChildren()) {
            collect(child.get());
        }
    };

    auto activeOverlay = getActiveOverlay();
    if (activeOverlay) {
        collect(activeOverlay);
    } else {
        for (const auto& element : m_elements) {
            if (!element->isOverlay()) {
                collect(element.get());
            }
        }
    }
}

GUIElement* GUIManager::getActiveOverlay() const {
    for (const auto& element : m_elements) {
        if (element && element->isOverlay() && element->isVisible() && !element->isMarkedForDeletion()) {
            return element.get();
        }
    }
    return nullptr;
}

void GUIManager::focusNextElement(bool forward) {
    std::vector<GUIElement*> focusable;
    collectFocusableElements(focusable);
    if (focusable.empty()) return;

    auto it = std::find(focusable.begin(), focusable.end(), m_keyboardFocusElement);
    size_t index;
    if (it == focusable.end()) {
        index = forward ? 0 : focusable.size() - 1;
    } else {
        size_t pos = static_cast<size_t>(std::distance(focusable.begin(), it));
        if (forward) {
            index = (pos + 1) % focusable.size();
        } else {
            index = (pos == 0) ? focusable.size() - 1 : pos - 1;
        }
    }
    setKeyboardFocus(focusable[index]);
}
