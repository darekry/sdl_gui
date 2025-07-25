#ifndef GUI_HPP
#define GUI_HPP

#include "texture_manager.hpp" // Dodano include dla TextureManager
#include "font_manager.hpp" // Dodano include dla FontManager i SharedFont
#include "SDL2/SDL.h"

#include "style.hpp"

import std.compat;
// Forward declaration
class GUIManager;

// Podstawowa klasa bazowa dla elementów GUI
class GUIElement {
private:
    std::string tooltip;
    uint32_t tooltipTimerId = 0;
public:
    // Konstruktor
    GUIElement(GUIManager& manager, int x, int y, int width, int height);
    
    // Wirtualny destruktor dla poprawnego dziedziczenia
    virtual ~GUIElement() = default;

    void setTooltip(const std::string& text);
    // Metody dostępu do położenia i rozmiaru
    [[nodiscard]] int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void getSize(int& width, int& height) const;

    // Metody ustawiające położenie i rozmiar
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setParent(GUIElement* parent);

    // Metoda zwracająca absolutną pozycję elementu
    SDL_Point getAbsolutePosition() const;

    // Metoda sprawdzająca, czy punkt (x, y) znajduje się w obrębie elementu (uwzględniając pozycję rodzica)
    bool contains(int x, int y) const;

    // Wirtualna metoda do obsługi zdarzeń (do zaimplementowania w klasach pochodnych)
    // Na razie pusta, będzie rozszerzona w przyszłości
    virtual bool handleEvent(const SDL_Event& e);

    // Metoda renderująca, nie jest już wirtualna
    void render();

    void setClipChildren(bool clip);

    // Metoda do ustawiania tekstury
    void setTexture(const SharedTexture& texture);
    void setLabel(std::string_view text, int font_size = -1);
    
        // Metody do zarządzania stanem włączony/wyłączony
        void setEnabled(bool enabled) { m_enabled = enabled; }
        [[nodiscard]] bool isEnabled() const { return m_enabled; }
    
        // Metody do zarządzania widocznością
        void setVisible(bool visible) { m_visible = visible; }
        [[nodiscard]] bool isVisible() const { return m_visible; }

    // Metoda do sprawdzania stanu najechania myszką
    [[nodiscard]] bool isHovered() const { return m_isHovered; }

    // --- Nowe API do stylizacji ---
    void setStyle(ElementState state, Style style);
    std::optional<Style> getStyle(ElementState state) const;
    Style getResolvedStyle() const;

    // Metody pomocnicze
    void setBackgroundColor(ElementState state, SDL_Color color);
    void setTextColor(ElementState state, SDL_Color color);
    void setTexture(ElementState state, SharedTexture texture);
    void setBorder(ElementState state, SDL_Color color, int width);
    
    virtual const char* getComponentType() const;

    // Metody do zarządzania usuwaniem
    void markForDeletion();
    bool isMarkedForDeletion() const;
    void cleanup();

protected:
    uint32_t startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void stopTimer(uint32_t timerId);
    
    bool m_clip_children = true;
    
    // Nowa, chroniona metoda wirtualna do rysowania zawartości elementu.
    // Klasy pochodne powinny ją nadpisywać.
    virtual void draw();

    Style resolveStyle(const Style& base, const std::optional<Style>& override) const;

    GUIManager& m_manager;
    int m_x, m_y;
    int m_width, m_height;
    bool m_enabled = true; // Domyślnie włączony
    bool m_visible = true; // Domyślnie widoczny
    bool m_isHovered = false; // Stan najechania myszką
    bool m_isMarkedForDeletion = false; // Flaga do oznaczania elementu do usunięcia
    GUIElement* m_parent;
    SharedTexture m_texture;
    std::unique_ptr<GUIElement> m_label = nullptr;
    
    // --- Nowe pola dla stylów ---
    std::map<ElementState, Style> m_styles;
    ElementState m_currentState = ElementState::Normal;
    bool m_style_dirty = true;

    std::vector<std::unique_ptr<GUIElement>> m_children;
public:
    // Metody do zarządzania relacją rodzic-dziecko
    void addChild(std::unique_ptr<GUIElement> child);
    void clearChildren();
    [[nodiscard]] GUIElement* getParent() const { return m_parent; }
    [[nodiscard]] const std::vector<std::unique_ptr<GUIElement>>& getChildren() const { return m_children; }
};

// Forward declaration for TextInput
class TextInput;

// Klasa przycisku dziedzicząca po GUIElement
#endif // GUI_HPP