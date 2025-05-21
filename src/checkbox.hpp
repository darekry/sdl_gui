#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"
#include "gui.hpp"
#include "sdl_deleters.hpp" // Dla SharedFont i SDLTextureDeleter
#include <string>
#include <functional>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <memory>

// Typ dla współdzielonego wskaźnika na czcionkę (zdefiniowany w sdl_deleters.hpp)
using SharedFont = std::shared_ptr<TTF_Font>;
class Checkbox : public GUIElement {
public:
    // Konstruktor
    Checkbox(int x, int y, int w, int h, const std::string& label = "");

    // Destruktor
    ~Checkbox();

    // Metody do zarządzania stanem
    bool isChecked() const { return m_isChecked; }
    void setChecked(bool checked);

    // Metoda do ustawiania etykiety tekstowej
    void setLabel(const std::string& label);

    // Metoda do ustawiania czcionki dla etykiety
    void setFont(SharedFont font);

    // Metoda do ustawiania koloru tekstu etykiety
    void setTextColor(SDL_Color color);

    // Typ callbacka dla zmiany stanu
    using OnChangeCallback = std::function<void(Checkbox*, bool)>;

    // Metoda do przypisywania callbacka
    void setOnChange(OnChangeCallback callback) { m_onChange = callback; }

    // Przesłonięte metody do obsługi zdarzeń i renderowania
    void handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool m_isChecked;
    std::string m_labelText;
    SDL_Color m_textColor;
    SharedFont m_font;
    OnChangeCallback m_onChange;
    std::string m_renderedText; // Tekst, który został ostatnio wyrenderowany

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

#endif // CHECKBOX_HPP