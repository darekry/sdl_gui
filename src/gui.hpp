#ifndef GUI_HPP
#define GUI_HPP

#include "SDL2/SDL.h"
#include "texture_manager.hpp" // Dodano include dla TextureManager
#include "font_manager.hpp" // Dodano include dla FontManager i SharedFont
#include <functional>
#include <string>
#include <memory>

// Forward declaration
class GUIManager;

// Podstawowa klasa bazowa dla elementów GUI
class GUIElement {
public:
    // Konstruktor
    GUIElement(int x, int y, int width, int height);

    // Wirtualny destruktor dla poprawnego dziedziczenia
    virtual ~GUIElement() = default;

    // Metody dostępu do położenia i rozmiaru
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Metody ustawiające położenie i rozmiar
    void setPosition(int x, int y);
    void setSize(int width, int height);

    // Metoda zwracająca absolutną pozycję elementu
    SDL_Point getAbsolutePosition() const;

    // Metoda sprawdzająca, czy punkt (x, y) znajduje się w obrębie elementu (uwzględniając pozycję rodzica)
    bool contains(int x, int y) const;

    // Wirtualna metoda do obsługi zdarzeń (do zaimplementowania w klasach pochodnych)
    // Na razie pusta, będzie rozszerzona w przyszłości
    virtual bool handleEvent(SDL_Event& e);

    // Wirtualna metoda do renderowania (do zaimplementowania w klasach pochodnych)
    virtual void render(SDL_Renderer* renderer);

    // Metoda do ustawiania tekstury
    void setTexture(SharedTexture texture);
    [[nodiscard]] SharedTexture getLabelTexture() const;

public:
    // Metody do zarządzania stanem włączony/wyłączony
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Metody do zarządzania widocznością
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    // Metody do zarządzania wskaźnikiem na GUIManager
    void setGUIManager(GUIManager* manager);
    GUIManager* getGUIManager() const;

    // Metoda do sprawdzania stanu najechania myszką
    bool isHovered() const { return m_isHovered; }

protected:
    int m_x, m_y;
        int m_width, m_height;
        bool m_enabled = true; // Domyślnie włączony
        bool m_visible = true; // Domyślnie widoczny
        bool m_isHovered = false; // Stan najechania myszką
        GUIElement* m_parent;
        GUIManager* m_guiManager;
        SharedTexture m_texture;
    
        std::vector<std::unique_ptr<GUIElement>> m_children;
public:
    // Metody do zarządzania relacją rodzic-dziecko
    void addChild(std::unique_ptr<GUIElement> child);
    void clearChildren();
    GUIElement* getParent() const { return m_parent; }
    const std::vector<std::unique_ptr<GUIElement>>& getChildren() const { return m_children; }
};

// Forward declaration for TextInput
class TextInput;

// Klasa przycisku dziedzicząca po GUIElement
#endif // GUI_HPP