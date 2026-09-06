#include <SDL3/SDL.h>
#include "gui_manager.hpp"
#include "gui.hpp"
#include "timer_manager.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "context_menu.hpp"

#include "constants.hpp"
#include "std.hpp"

static constexpr int DEFAULT_FONT_SIZE = 24;
static constexpr size_t kRenderCachePruneThreshold = 256;
static const SDL_Color TOOLTIP_BG_COLOR = {.r=255, .g=255, .b=225, .a=255};

static bool isDescendantOf(GUIElement* descendant, GUIElement* ancestor) {
    GUIElement* current = descendant->getParent();
    while (current) {
        if (current == ancestor) return true;
        current = current->getParent();
    }
    return false;
}

GUIManager::GUIManager(SDL_Renderer* renderer, Viewport viewport)
    : m_renderer(renderer), m_textureManager(renderer), m_theme(Theme::createDefaultTheme()) {
    if (!viewport.valid()) {
        throw std::invalid_argument("GUIManager requires a NonZero Viewport (width and height must be > 0)");
    }
    m_windowWidth = viewport.width;
    m_windowHeight = viewport.height;
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
        registerElement(raw_ptr);
        m_elements.push_back(std::move(element));
        // Viewport jest zawsze NonZero — kotwice aplikowane od razu, bez
        // czekania na pierwszy resize (koniec elementów na (0,0) po dodaniu).
        raw_ptr->updateLayout(m_windowWidth, m_windowHeight);
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
    // 1. Mouse events
    if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (GUIElement* capture = getMouseCapture()) {
            // If an element captured the mouse, send events only to it.
            // A destroyed capture target resolves to null, so this branch
            // is simply skipped — no dangling call.
            if (cursor) cursor->handleEvent(event); /* cursor overlay tracks position even during capture */
            return capture->handleEvent(event);
        }
    }
    // 2. Keyboard events
    else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_TEXT_INPUT) {
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
            focusNextElement(!(event.key.mod & SDL_KMOD_SHIFT));
            return true;
        }
        if (GUIElement* focus = getKeyboardFocus()) {
            // If an element has keyboard focus, send events only to it
            return focus->handleEvent(event);
        }
        // If no element has focus, keyboard events are ignored by the GUI
        return false;
    }

    // 3. Standard propagation for unhandled events
    if (tooltipElement && tooltipElement->isVisible() && tooltipElement->handleEvent(event)) {
        return true;
    }

    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if ((*it)->handleEvent(event)) {
            return true;
        }
    }

    // Special handling for clicks outside focused elements
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && getKeyboardFocus()) {
        bool click_on_focusable = false;
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            if ((*it)->contains(event.button.x, event.button.y)) {
                // This is a simplification; ideally we would check whether the clicked element
                // can actually receive focus.
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
    // Update timers and animations
    timerManager->update();
    animation_manager->update();

    if (m_textureManager.getRenderCacheSize() > kRenderCachePruneThreshold) {
        m_textureManager.pruneUnused();
    }
}

void GUIManager::render() {
    for (const auto& element : m_elements) {
        if (!element->isOverlay() && !element->isMarkedForDeletion()) {
            element->render(m_renderer);
        }
    }

    if (tooltipElement && !tooltipElement->isMarkedForDeletion()) {
        tooltipElement->render(m_renderer);
    }

    for (const auto& element : m_elements) {
        if (element->isOverlay() && !element->isMarkedForDeletion()) {
            element->renderOverlay(m_renderer);
        }
    }

    // Render overlay for keyboard focus element (e.g., TextInput cursor/selection)
    // Only render if element is not hidden behind a modal overlay.
    // getKeyboardFocus() resolves the handle — a destroyed target yields
    // null instead of a dangling pointer, so no isMarkedForDeletion dance.
    if (GUIElement* focus = getKeyboardFocus()) {
        if (!focus->isMarkedForDeletion()) {
            auto* overlay = getActiveOverlay();
            if (!overlay || isDescendantOf(focus, overlay)) {
                focus->renderOverlay(m_renderer);
            }
        }
    }

    if (cursor) {
        cursor->renderOverlay(m_renderer);
    }
}

void GUIManager::cleanup() {
    auto subtreeDead = [](GUIElement* element) {
        for (GUIElement* current = element; current; current = current->getParent()) {
            if (current->isMarkedForDeletion()) return true;
        }
        return false;
    };

    // Focus/capture are handles: if the target (or any ancestor) is marked,
    // drop the handle after the standard lost-notification. The target object
    // itself is still alive here, so notifying is safe. If the target was
    // already destroyed, resolve() returns null and we just reset.
    if (m_keyboardFocusHandle.valid()) {
        if (GUIElement* focus = resolve(m_keyboardFocusHandle)) {
            if (subtreeDead(focus)) {
                m_keyboardFocusHandle.reset();
                focus->onFocusLost();
            }
        } else {
            m_keyboardFocusHandle.reset();
        }
    }
    if (m_mouseCaptureHandle.valid()) {
        if (GUIElement* capture = resolve(m_mouseCaptureHandle)) {
            if (subtreeDead(capture)) {
                m_mouseCaptureHandle.reset();
                capture->onMouseCaptureLost();
            }
        } else {
            m_mouseCaptureHandle.reset();
        }
    }

    if (tooltipElement && tooltipElement->isMarkedForDeletion()) {
        hideTooltip();
    }

    for (const auto& element : m_elements) {
        element->cleanup();
    }

    // Erasing destroys subtrees; each element's destructor unregisters its
    // slot (bumping the generation), so focus/capture/menu handles pointing
    // into removed subtrees go stale automatically.
    auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });

    std::size_t prefix_distance = static_cast<std::size_t>(std::distance(m_elements.begin(), new_end));
    if (prefix_distance < m_elements.size())
    {
        LOG_DEBUG("GUIManager::cleanup() - erasing {} elements from vector", m_elements.size() - prefix_distance);
        m_elements.erase(new_end, m_elements.end());
    }

    // Post-erase: never notify — the object may be gone. Just reset stale
    // handles and refresh the context-menu cache.
    if (m_keyboardFocusHandle.valid() && !resolve(m_keyboardFocusHandle)) {
        m_keyboardFocusHandle.reset();
    }
    if (m_mouseCaptureHandle.valid() && !resolve(m_mouseCaptureHandle)) {
        m_mouseCaptureHandle.reset();
    }
    m_contextMenuCache = static_cast<ContextMenu*>(resolve(m_contextMenuHandle));
    if (!m_contextMenuCache) m_contextMenuHandle.reset();
}

void GUIManager::showTooltip(GUIElement* target, const std::string& text) {
    if (!target) return;

    const int fontSize = constants::kTooltipFontSize;
    const int padding = constants::kTooltipPadding;

    auto targetPos = target->getAbsolutePosition();
    int posX = targetPos.x;
    int posY = targetPos.y + target->getHeight();

    if (!m_tooltipPanel) {
        // Persistent panel: created once, toggled with setVisible().
        // Registration happens in the Label ctor + addChild; the panel
        // itself is registered explicitly (never reparented, never moved).
        m_tooltipPanel = std::make_unique<Panel>(*this, posX, posY, 0, 0);
        m_tooltipPanel->setBackgroundColor(ElementState::Normal, TOOLTIP_BG_COLOR);
        m_tooltipPanel->setBorder(ElementState::Normal, {0, 0, 0, 255}, 1);

        auto label = std::make_unique<Label>(*this, padding, padding, "", fontSize);
        m_tooltipLabel = label.get();
        m_tooltipPanel->addChild(std::move(label));
        tooltipElement = m_tooltipPanel.get();
    }

    m_tooltipLabel->setText(text);
    int panelWidth = m_tooltipLabel->getWidth() + (2 * padding);
    int panelHeight = m_tooltipLabel->getHeight() + (2 * padding);

    m_tooltipPanel->setPosition(posX, posY);
    m_tooltipPanel->setSize(panelWidth, panelHeight);

    m_tooltipPanel->setVisible(true);
}

void GUIManager::hideTooltip() {
    if (m_tooltipPanel) {
        m_tooltipPanel->setVisible(false);
    }
}

void GUIManager::showContextMenu(const std::vector<ContextMenuItem>& items, float x, float y) {
    ContextMenu* menu = getContextMenu();
    if (!menu) {
        auto fresh = std::make_unique<ContextMenu>(*this);
        menu = fresh.get();
        if (addElement(std::move(fresh)) == nullptr) return;
        m_contextMenuHandle = getHandle(menu);
        m_contextMenuCache = menu;
    }

    menu->clearItems();
    for (const auto& item : items) {
        if (item.separator) {
            menu->addSeparator();
        } else {
            menu->addItem(item.text, item.action, item.enabled);
        }
    }
    menu->showAt(static_cast<int>(x), static_cast<int>(y));
}

void GUIManager::closeContextMenu() {
    if (ContextMenu* menu = getContextMenu()) {
        menu->hide();
    }
}

bool GUIManager::isContextMenuVisible() const {
    if (const GUIElement* menu = resolveSlot(m_contextMenuHandle)) {
        return menu->isVisible();
    }
    return false;
}

ContextMenu* GUIManager::getContextMenu() {
    m_contextMenuCache = static_cast<ContextMenu*>(resolve(m_contextMenuHandle));
    if (!m_contextMenuCache) m_contextMenuHandle.reset();
    return m_contextMenuCache;
}


TimerManager* GUIManager::getTimerManager() {
    return timerManager.get();
}

void GUIManager::setTheme(Theme theme) {
    m_theme = std::move(theme);
    for (const auto& element : m_elements) {
        element->markDirtyRecursively();
    }
}

Theme& GUIManager::getTheme() {
    return m_theme;
}

void GUIManager::captureMouse(GUIElement* element) {
    GUIElement* current = getMouseCapture();
    if (current == element) return;
    if (current) {
        current->onMouseCaptureLost();
    }
    m_mouseCaptureHandle = getHandle(element);
    if (element) {
        element->onMouseCaptureGained();
    }
}

void GUIManager::releaseMouse() {
    if (GUIElement* current = getMouseCapture()) {
        current->onMouseCaptureLost();
    }
    m_mouseCaptureHandle.reset();
}

GUIElement* GUIManager::getMouseCapture() const {
    return resolve(m_mouseCaptureHandle);
}

void GUIManager::setKeyboardFocus(GUIElement* element) {
    GUIElement* current = getKeyboardFocus();
    if (current == element) {
        return;
    }
    if (current) {
        current->onFocusLost();
    }
    m_keyboardFocusHandle = getHandle(element);
    if (element) {
        element->onFocusGained();
    }
}

GUIElement* GUIManager::getKeyboardFocus() const {
    return resolve(m_keyboardFocusHandle);
}

bool GUIManager::isFocusInside(const GUIElement* element) const {
    if (!element) return false;
    for (GUIElement* focused = getKeyboardFocus(); focused; focused = focused->getParent()) {
        if (focused == element) return true;
    }
    return false;
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
        if (auto* element = (*it)->findElementAt(x, y)) {
            return element;
        }
    }
    return nullptr;
}

bool GUIManager::isElementAlive(GUIElement* element) const {
    if (!element) return false;
    auto it = m_ptrToSlot.find(element);
    if (it == m_ptrToSlot.end()) return false;
    uint32_t idx = it->second;
    return idx < m_slots.size() && m_slots[idx].alive && m_slots[idx].ptr == element;
}

ElementHandle GUIManager::registerSlot(GUIElement* element) const {
    if (!element) return {};
    if (auto it = m_ptrToSlot.find(element); it != m_ptrToSlot.end()) {
        uint32_t idx = it->second;
        if (idx < m_slots.size() && m_slots[idx].alive && m_slots[idx].ptr == element) {
            return {idx, m_slots[idx].generation};
        }
    }
    uint32_t idx;
    if (!m_freeSlots.empty()) {
        idx = m_freeSlots.back();
        m_freeSlots.pop_back();
    } else {
        idx = static_cast<uint32_t>(m_slots.size());
        m_slots.push_back({});
    }
    m_slots[idx].ptr = element;
    m_slots[idx].alive = true;
    // Generation was bumped on unregister; first use keeps the initial 1.
    m_ptrToSlot[element] = idx;
    return {idx, m_slots[idx].generation};
}

GUIElement* GUIManager::resolveSlot(ElementHandle handle) const {
    if (!handle.valid() || handle.index >= m_slots.size()) return nullptr;
    const LifetimeSlot& slot = m_slots[handle.index];
    if (!slot.alive || slot.generation != handle.generation) return nullptr;
    return slot.ptr;
}

void GUIManager::registerElement(GUIElement* element) {
    if (!element) return;
    ElementHandle h = registerSlot(element);
    element->setLifetimeHandle(h);
    LOG_DEBUG("GUIManager::registerElement() - registered {}, total = {}", static_cast<const void*>(element), m_ptrToSlot.size());
}

void GUIManager::unregisterElement(GUIElement* element) {
    if (!element) return;
    auto it = m_ptrToSlot.find(element);
    if (it == m_ptrToSlot.end()) return;
    uint32_t idx = it->second;
    m_ptrToSlot.erase(it);
    if (idx < m_slots.size()) {
        m_slots[idx].ptr = nullptr;
        m_slots[idx].alive = false;
        if (++m_slots[idx].generation == 0) ++m_slots[idx].generation;
        if (m_slots[idx].generation == ElementHandle::kInvalidIndex) ++m_slots[idx].generation;
        m_freeSlots.push_back(idx);
    }
    LOG_DEBUG("GUIManager::unregisterElement() - unregistered {}, total = {}", static_cast<const void*>(element), m_ptrToSlot.size());
}

ElementHandle GUIManager::getHandle(const GUIElement* element) const {
    if (!element) return {};
    auto it = m_ptrToSlot.find(element);
    if (it == m_ptrToSlot.end()) return {};
    uint32_t idx = it->second;
    if (idx >= m_slots.size() || !m_slots[idx].alive) return {};
    return {idx, m_slots[idx].generation};
}

GUIElement* GUIManager::resolve(ElementHandle handle) const {
    return resolveSlot(handle);
}

bool GUIManager::isHandleAlive(ElementHandle handle) const {
    return resolveSlot(handle) != nullptr;
}

// === Resize handling ===

void GUIManager::handleResize(int width, int height) {
    // Zachowaj niezmiennik NonZero (0 przychodzi np. przy minimalizacji okna).
    if (width <= 0 || height <= 0) {
        return;
    }
    m_windowWidth = width;
    m_windowHeight = height;

    // Propagacja do WSZYSTKICH top-level (nie tylko z anchorami): rodzic bez
    // anchora też musi przekazać resize dzieciom z anchorami.
    for (const auto& element : m_elements) {
        element->updateLayout(width, height);
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

void GUIManager::collectFocusableElements(std::vector<GUIElement*>& out) const {
    std::function<void(GUIElement*)> collect = [&](GUIElement* element) {
        if (!element->isVisible()) return;
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
        if (element->isOverlay() && element->isVisible() && !element->isMarkedForDeletion()) {
            return element.get();
        }
    }
    return nullptr;
}

void GUIManager::focusNextElement(bool forward) {
    std::vector<GUIElement*> focusable;
    collectFocusableElements(focusable);
    if (focusable.empty()) return;

    auto it = std::find(focusable.begin(), focusable.end(), getKeyboardFocus());
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
