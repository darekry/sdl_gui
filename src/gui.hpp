#ifndef GUI_HPP
#define GUI_HPP

#include "texture_manager.hpp"
#include "font_manager.hpp"
#include "SDL2/SDL.h"
#include "animation_manager.hpp"
#include "style.hpp"
#include <functional>
#include <optional>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <string_view>

class GUIManager;

class GUIElement {
private:
    std::string tooltip;
    uint32_t tooltipTimerId = 0;
public:
    int m_x, m_y; // Pola publiczne zgodnie z życzeniem
    int m_width, m_height;
    bool m_enabled = true;
    bool m_visible = true;

    GUIElement(GUIManager& manager, int x, int y, int width, int height);
    virtual ~GUIElement() = default;

    void setTooltip(const std::string& text);
    [[nodiscard]] int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void getSize(int& width, int& height) const;

    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setParent(GUIElement* parent);
    SDL_Point getAbsolutePosition() const;
    SDL_Point getRelativePosition() const { return {m_x, m_y}; }
    bool contains(int x, int y) const;
    virtual bool handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer);
    void renderToCache();
    void setClipChildren(bool clip);
    void setTexture(const SharedTexture& texture);
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }
    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }
    [[nodiscard]] bool isHovered() const { return m_isHovered; }
    void setState(ElementState newState);
    void setStyle(ElementState state, Style style);
    void setBackgroundColor(ElementState state, SDL_Color color);
    void setTextColor(ElementState state, SDL_Color color);
    void setTexture(ElementState state, SharedTexture texture);
    void setBorder(ElementState state, SDL_Color color, int width);
    virtual const char* getComponentType() const;
    void markForDeletion();
    void markDirty(bool cascadeToParents = true);
    bool isMarkedForDeletion() const;
    void cleanup();

    void addChild(std::unique_ptr<GUIElement> child);
    void clearChildren();
    [[nodiscard]] GUIElement* getParent() const { return m_parent; }
    [[nodiscard]] const std::vector<std::unique_ptr<GUIElement>>& getChildren() const { return m_children; }
size_t countDescendants() const;

private:
    void render(SDL_Renderer* renderer, const SDL_Rect& parent_clip_rect);
 
protected:
    uint32_t startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void stopTimer(uint32_t timerId);
    
    bool m_clip_children = true;
    virtual void draw(SDL_Renderer* renderer) = 0;

    // Rozszerzenie: możliwość rysowania bezpośrednio (bez buforowania).
    // Domyślnie elementy nie korzystają z drawDirect — zwracają false w wantsDirectRender().
    virtual bool wantsDirectRender() const { return false; }
    virtual void drawDirect(SDL_Renderer* renderer) { /* domyślnie brak */ }
    void drawBackgroundAndBorder(SDL_Renderer* renderer);

        Style getComposedStyle(ElementState state) const;

    GUIManager& m_manager;
    bool m_isHovered = false;
    bool m_isMarkedForDeletion = false;
    GUIElement* m_parent;
    SharedTexture m_texture;
    bool m_isDirty = true;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_cachedTexture{nullptr, SDL_DestroyTexture};
        std::map<ElementState, Style> m_localStyles;
        ElementState m_state = ElementState::Normal;
    bool m_style_dirty = true;
    std::vector<std::unique_ptr<GUIElement>> m_children;
};

#endif // GUI_HPP