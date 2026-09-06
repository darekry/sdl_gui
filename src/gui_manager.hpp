#pragma once


#include <SDL3/SDL.h>
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "timer_manager.hpp"
#include "theme.hpp"
#include "animation_manager.hpp"
#include "cursor.hpp"
#include "element_handle.hpp"
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
     * The element keeps its lifetime slot (handle stays valid).
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
    [[nodiscard]] GUIElement* getActiveTooltip() const { return tooltipElement; }

    // Shared right-click context menu (one instance, reused; items rebuilt on each show).
    // Used for the default Cut/Copy/Paste/Select All menu in text fields and for custom menus.
    // Item actions no longer need isElementAlive guards when they capture an
    // ElementRef (handle-based, auto-null after destroy).
    void showContextMenu(const std::vector<ContextMenuItem>& items, float x, float y);
    void closeContextMenu();
    [[nodiscard]] bool isContextMenuVisible() const;
    [[nodiscard]] ContextMenu* getContextMenu();

    void setTheme(Theme theme);
    Theme& getTheme();

    GUIElement* findElementAt(int x, int y);

    void captureMouse(GUIElement* element);
    void releaseMouse();
    [[nodiscard]] GUIElement* getMouseCapture() const;
    void setKeyboardFocus(GUIElement* element);
    [[nodiscard]] GUIElement* getKeyboardFocus() const;
    // True when the focused element is this element or one of its descendants.
    // Used by overlays (e.g. ContextMenu::hide) instead of walking parents
    // with raw pointers that may dangle.
    [[nodiscard]] bool isFocusInside(const GUIElement* element) const;
    void focusNextElement(bool forward);

    void setCursor(std::unique_ptr<Cursor> new_cursor);
    [[nodiscard]] Cursor* getCursor() const { return cursor.get(); }

    // === Generational lifetime (point 5: SlotMap + ElementHandle) ===
    //
    // registerElement assigns (or reuses) a slot and stamps the element's
    // lifetime handle; unregisterElement bumps the generation so all existing
    // handle copies stop resolving. Safe against address reuse (ABA).
    bool isElementAlive(GUIElement* element) const;
    void registerElement(GUIElement* element);
    void unregisterElement(GUIElement* element);

    // O(1) handle API — the preferred way to hold elements across frames.
    [[nodiscard]] ElementHandle getHandle(const GUIElement* element) const;
    [[nodiscard]] GUIElement* resolve(ElementHandle handle) const;
    [[nodiscard]] bool isHandleAlive(ElementHandle handle) const;
    
    template<typename T = GUIElement>
    ElementRef<T> makeRef(T* element);

private:
    // One slot per registered element. Generation bumps on unregister;
    // resolve() requires generation match + alive + ptr set.
    struct LifetimeSlot {
        GUIElement* ptr = nullptr;
        uint32_t generation = 1;
        bool alive = false;
    };
    mutable std::vector<LifetimeSlot> m_slots;
    mutable std::vector<uint32_t> m_freeSlots;
    mutable std::unordered_map<const GUIElement*, uint32_t> m_ptrToSlot;

    [[nodiscard]] ElementHandle registerSlot(GUIElement* element) const;
    [[nodiscard]] GUIElement* resolveSlot(ElementHandle handle) const;

    std::vector<std::unique_ptr<GUIElement>> m_elements;
    std::unique_ptr<Cursor> cursor;
    // Lazily created context menu, owned by m_elements. Accessed only via
    // m_contextMenuHandle (auto-null after destroy) — never a stale raw*.
    ElementHandle m_contextMenuHandle;
    ContextMenu* m_contextMenuCache = nullptr;

    // Tooltip: single persistent panel, toggled with setVisible().
    // (Old code ping-ponged ownership between two unique_ptrs via move().)
    std::unique_ptr<Panel> m_tooltipPanel;
    Label* m_tooltipLabel = nullptr;
    GUIElement* tooltipElement = nullptr;  // non-owning view of m_tooltipPanel

    // Focus/capture as handles: they self-null when the target is destroyed,
    // so cleanup() never touches dangling pointers and needs no
    // hasAncestorMarkedForDeletion walk.
    ElementHandle m_mouseCaptureHandle;
    ElementHandle m_keyboardFocusHandle;

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
    ElementRef() = default;
    ElementRef(GUIManager& manager, T* ptr)
        : m_manager(&manager)
        , m_handle(manager.getHandle(ptr))
        , m_raw(ptr) {}

    // Resolves through the generational slot: null after destroy, even if
    // the address was reused by a different element (no ABA).
    T* get() const {
        if (!m_manager || !m_handle.valid()) return nullptr;
        GUIElement* live = m_manager->resolve(m_handle);
        if (live != m_raw) return nullptr;
        return static_cast<T*>(live);
    }

    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    explicit operator bool() const { return get() != nullptr; }

    bool operator==(std::nullptr_t) const { return get() == nullptr; }

    [[nodiscard]] ElementHandle handle() const { return m_handle; }

private:
    GUIManager* m_manager = nullptr;
    ElementHandle m_handle;
    GUIElement* m_raw = nullptr;
};

template<typename T>
ElementRef<T> GUIManager::makeRef(T* element) {
    return ElementRef<T>(*this, element);
}
