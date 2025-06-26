#ifndef RADIOBUTTON_HPP
#define RADIOBUTTON_HPP

#include "gui.hpp"
#include "sdl_deleters.hpp" // Dla SharedFont i SDLTextureDeleter
#include "radio_group.hpp" // Dla klasy RadioGroup
#include <string>
#include <functional>
#include <memory>

// Typ dla współdzielonego wskaźnika na czcionkę (zdefiniowany w sdl_deleters.hpp)
// using SharedFont = std::shared_ptr<TTF_Font>; // Już zdefiniowane w gui.hpp
class RadioButton : public GUIElement {
public:
    // Konstruktor
    RadioButton(int x, int y, int w, int h);

    // Destruktor
    ~RadioButton() = default;

    // Metody do zarządzania stanem
    bool isSelected() const { return m_isSelected; }
    void setSelected(bool selected, bool notifyGroup = true);

    // Metoda do ustawiania etykiety tekstowej
    void setLabel(SDL_Renderer* renderer, const std::string& text, SharedFont font, SDL_Color color, TextureManager& textureManager);

    // Metoda do ustawiania grupy, do której należy RadioButton
    void setGroup(RadioGroup* group);
    RadioGroup* getGroup() const { return m_group; }

    // Typ callbacka dla zmiany stanu (zaznaczenia)
    using OnChangeCallback = std::function<void(RadioButton*)>;

    // Metoda do przypisywania callbacka
    void setOnChange(OnChangeCallback callback) { m_onChange = callback; }

    // Przesłonięte metody do obsługi zdarzeń i renderowania
    bool handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool m_isSelected;
    RadioGroup* m_group; // Wskaźnik do grupy, do której należy przycisk
    OnChangeCallback m_onChange;
    SharedTexture m_labelTexture;
};

#endif // RADIOBUTTON_HPP