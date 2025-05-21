#ifndef RADIOBUTTON_HPP
#define RADIOBUTTON_HPP

#include "gui.hpp"
#include "gui.hpp"
#include "sdl_deleters.hpp" // Dla SharedFont i SDLTextureDeleter
#include "radio_group.hpp" // Dla klasy RadioGroup
#include <string>
#include <functional>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <memory>

// Typ dla współdzielonego wskaźnika na czcionkę (zdefiniowany w sdl_deleters.hpp)
using SharedFont = std::shared_ptr<TTF_Font>; // Już zdefiniowane w checkbox.hpp lub sdl_deleters.hpp
class RadioButton : public GUIElement {
public:
    // Konstruktor
    RadioButton(int x, int y, int w, int h, const std::string& label = "");

    // Destruktor
    ~RadioButton();

    // Metody do zarządzania stanem
    bool isSelected() const { return m_isSelected; }
    void setSelected(bool selected);

    // Metoda do ustawiania etykiety tekstowej
    void setLabel(const std::string& label);

    // Metoda do ustawiania czcionki dla etykiety
    void setFont(SharedFont font);

    // Metoda do ustawiania koloru tekstu etykiety
    void setTextColor(SDL_Color color);

    // Metoda do ustawiania grupy, do której należy RadioButton
    void setGroup(RadioGroup* group);

    // Typ callbacka dla zmiany stanu (zaznaczenia)
    using OnChangeCallback = std::function<void(RadioButton*)>;

    // Metoda do przypisywania callbacka
    void setOnChange(OnChangeCallback callback) { m_onChange = callback; }

    // Przesłonięte metody do obsługi zdarzeń i renderowania
    void handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool m_isSelected;
    std::string m_labelText;
    std::string m_renderedText;
    SDL_Color m_textColor;
    SharedFont m_font;
    RadioGroup* m_group; // Wskaźnik do grupy, do której należy przycisk
    OnChangeCallback m_onChange;

    // Tekstura dla etykiety
    std::shared_ptr<SDL_Texture> m_labelTexture;
    // Rozmiar etykiety
    int m_labelWidth;
    int m_labelHeight;

    // Metoda pomocnicza do renderowania etykiety
    void renderLabel(SDL_Renderer* renderer);
    // Metoda pomocnicza do aktualizacji tekstury etykiety
    void updateLabelTexture(SDL_Renderer* renderer);
};

#endif // RADIOBUTTON_HPP