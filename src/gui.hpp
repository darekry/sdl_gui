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

protected:
    protected:
        int m_x, m_y;
        int m_width, m_height;
        bool m_enabled = true; // Domyślnie włączony
        bool m_visible = true; // Domyślnie widoczny
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
class Button : public GUIElement {
public:
    // Konstruktor
    Button(int x, int y, int width, int height, SharedTexture texture = nullptr);

    // Destruktor
    ~Button() = default;

    // Typy callbacków dla zdarzeń
    // Typy callbacków dla zdarzeń
    using OnClickCallback = std::function<void(GUIElement*)>;
    using OnMouseOverCallback = std::function<void(GUIElement*)>;

    // Metody do przypisywania callbacków
    void setOnClickCallback(OnClickCallback callback) { m_onClick = callback; }
    void setOnMouseOverCallback(OnMouseOverCallback callback) { m_onMouseOver = callback; }

    // Metody wywołujące callbacki (publiczne na potrzeby testów/integracji)
    void triggerOnClick() { if (m_onClick) m_onClick(this); }
    void triggerOnMouseOver() { if (m_onMouseOver) m_onMouseOver(this); }
    void triggerOnRelease() {
        if (m_onClick) m_onClick(this);
    } // Używamy m_onClick dla zdarzenia puszczenia przycisku

    // Przesłonięte metody do obsługi zdarzeń i renderowania (na razie puste)
    bool handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;
    void setLabel(const std::string& text, GUIManager& guiManager);

private:
    std::string m_labelText;
    OnClickCallback m_onClick;
    OnMouseOverCallback m_onMouseOver;
};

// Klasa Panel dziedzicząca po GUIElement
class Panel : public GUIElement {
public:
    // Konstruktor
    Panel(int x, int y, int width, int height);

    // Przesłonięta metoda do renderowania
    void render(SDL_Renderer* renderer) override;

    // Metody do ustawiania obramowania
    void setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void setBorderThickness(int thickness);

private:
    SDL_Color m_borderColor = {0, 0, 0, 255}; // Domyślny kolor obramowania (czarny)
    int m_borderThickness = 1; // Domyślna grubość obramowania
};
#endif // GUI_HPP