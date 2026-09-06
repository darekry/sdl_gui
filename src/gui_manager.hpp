#pragma once


#include <SDL3/SDL.h>
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "timer_manager.hpp"
#include "theme.hpp"
#include "animation_manager.hpp"
#include "cursor.hpp"
#include <SDL3/SDL_gpu.h>

#include "std.hpp"

class TimerManager;
class AnimationManager;
class GUIElement;
class Theme;
class Panel;
class Label;
class ContextMenu;
struct ContextMenuItem;

template<typename T>
class ElementRef;

/**
 * @brief Callback type for window resize events
 * Parameters: new window width, new window height
 */
using ResizeCallback = std::function<void(int, int)>;

class GUIManager {
public:
    /**
     * @brief Viewport (rozmiar okna) wstrzykiwany w konstruktorze — nigdy 0x0.
     * Niezmiennik NonZero: getWindowSize() zawsze zwraca wymiary > 0, więc
     * żaden widget nie potrzebuje fallbacku rozmiaru.
     */
    explicit GUIManager(SDL_Renderer* renderer, Viewport viewport);
    ~GUIManager();

    GUIElement* addElement(std::unique_ptr<GUIElement> element);

    template<typename T, typename... Args>
    T* create(Args&&... args) {
        auto widget = std::make_unique<T>(*this, std::forward<Args>(args)...);
        T* raw = widget.get();
        addElement(std::move(widget));
        return raw;
    }

    template<typename T, typename... Args>
    T* create(GUIElement* parent, Args&&... args) {
        auto widget = std::make_unique<T>(*this, std::forward<Args>(args)...);
        T* raw = widget.get();
        parent->addChild(std::move(widget));
        return raw;
    }

    /*
     * Detach a top-level element without deleting it.
     * Returns the unique_ptr, or nullptr if the element is not a top-level element.
     * The element remains registered in m_liveElements (still valid).
     */
    std::unique_ptr<GUIElement> detachElement(GUIElement* element);

    bool processEvent(const SDL_Event& e);
    void update();
    void render();
    void cleanup();
    
    // === Resize handling ===
    
    /**
     * @brief Handle window resize - updates all anchored elements
     * Call this when window size changes (SDL_EVENT_WINDOW_RESIZED)
     * @param width New window width
     * @param height New window height
     */
    void handleResize(int width, int height);
    
    /**
     * @brief Set custom resize callback for additional handling
     * Callback is called after anchored elements are updated
     */
    void setResizeCallback(ResizeCallback callback);
    
    /**
     * @brief Get current stored window size (always > 0, see Viewport).
     */
    void getWindowSize(int& width, int& height) const;
    [[nodiscard]] Viewport getViewport() const { return Viewport{m_windowWidth, m_windowHeight}; }

    SDL_Renderer* getRenderer() const { return m_renderer; }
    SDL_GPUDevice* getGPUDevice() const { return SDL_GetGPURendererDevice(m_renderer); }
    FontManager& getFontManager() { return m_fontManager; }
    const FontManager& getFontManager() const { return m_fontManager; }
    TextureManager& getTextureManager() { return m_textureManager; }
    const TextureManager& getTextureManager() const { return m_textureManager; }
    TimerManager* getTimerManager();
    AnimationManager* getAnimationManager();
    
    void showTooltip(GUIElement* target, const std::string& text);
    void hideTooltip();
    [[nodiscard]] GUIElement* getActiveTooltip() const { return tooltipElement.get(); }

    // Shared right-click context menu (one instance, reused; items rebuilt on each show).
    // Used for the default Cut/Copy/Paste/Select All menu in text fields and for custom menus.
    // Item actions must guard against widget destruction (e.g. via isElementAlive).
    void showContextMenu(const std::vector<ContextMenuItem>& items, float x, float y);
    void closeContextMenu();
    [[nodiscard]] bool isContextMenuVisible() const;
    [[nodiscard]] ContextMenu* getContextMenu() const { return m_contextMenu; }

    void setTheme(Theme theme);
    Theme& getTheme();

    GUIElement* findElementAt(int x, int y);

    void captureMouse(GUIElement* element);
    void releaseMouse();
    void setKeyboardFocus(GUIElement* element);
    [[nodiscard]] GUIElement* getKeyboardFocus() const;
    void focusNextElement(bool forward);

    void setCursor(std::unique_ptr<Cursor> new_cursor);
    [[nodiscard]] Cursor* getCursor() const { return cursor.get(); }
    
    bool isElementAlive(GUIElement* element) const;
    void registerElement(GUIElement* element);
    void unregisterElement(GUIElement* element);
    
    template<typename T = GUIElement>
    ElementRef<T> makeRef(T* element);

private:
    std::unordered_set<GUIElement*> m_liveElements;
    
    std::vector<std::unique_ptr<GUIElement>> m_elements;
    std::unique_ptr<GUIElement> tooltipElement;
    std::unique_ptr<Cursor> cursor;
    ContextMenu* m_contextMenu = nullptr;  // owned by m_elements, created lazily
    
    // Cached tooltip components (reuse to avoid allocations)
    std::unique_ptr<Panel> m_tooltipPanel;
    Label* m_tooltipLabel = nullptr;

    GUIElement* m_mouseCaptureElement = nullptr;
    GUIElement* m_keyboardFocusElement = nullptr;

    SDL_Renderer* m_renderer;
    FontManager m_fontManager;
    TextureManager m_textureManager;
    std::unique_ptr<TimerManager> timerManager;
    std::unique_ptr<AnimationManager> animation_manager;

    Theme m_theme;
    
    void collectFocusableElements(std::vector<GUIElement*>& out) const;
    GUIElement* getActiveOverlay() const;
    
    // === Resize handling ===
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    ResizeCallback m_resizeCallback;
};

template<typename T>
class ElementRef {
public:
    ElementRef() : m_manager(nullptr), m_ptr(nullptr) {}
    ElementRef(GUIManager& manager, T* ptr) : m_manager(&manager), m_ptr(ptr) {}

    T* get() const {
        if (m_ptr && m_manager && m_manager->isElementAlive(m_ptr)) {
            return m_ptr;
        }
        return nullptr;
    }

    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    explicit operator bool() const { return get() != nullptr; }

    bool operator==(std::nullptr_t) const { return get() == nullptr; }

private:
    GUIManager* m_manager;
    T* m_ptr;
};

template<typename T>
ElementRef<T> GUIManager::makeRef(T* element) {
    return ElementRef<T>(*this, element);
}
