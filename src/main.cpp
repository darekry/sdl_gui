#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include "gui_manager.hpp"
#include "gui.hpp" // Zawiera GUIElement, Button, Panel
#include "slider.hpp"
#include "texture_manager.hpp"
#include "font_manager.hpp"
#include "text_input.hpp"
#include "checkbox.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include <iostream>
#include <memory> // Dla std::unique_ptr i std::make_unique

// Callback dla przycisku
void onButtonClick(GUIElement* element) {
    std::cout << "Button clicked!" << std::endl;
}

// Callback dla suwaka
void onSliderValueChanged(GUIElement* element) {
    Slider* slider = dynamic_cast<Slider*>(element);
    if (slider) {
        std::cout << "Slider value changed: " << slider->getValue() << std::endl;
    }
}

// Callback dla pola tekstowego (zmiana tekstu)
void onTextInputTextChanged(TextInput* element) {
    if (element) {
        std::cout << "TextInput text changed: " << element->getText() << std::endl;
    }
}

// Callback dla pola tekstowego (naciśnięcie Enter)
void onTextInputEnterPressed(TextInput* element) {
    if (element) {
        std::cout << "TextInput Enter pressed: " << element->getText() << std::endl;
    }
}

// Callback dla Checkboxa
void onCheckboxChanged(Checkbox* element, bool checked) {
     if (element) {
        std::cout << "Checkbox state changed to: " << (checked ? "checked" : "unchecked") << std::endl;
    }
}

// Callback dla RadioButtona
void onRadioButtonSelected(RadioButton* element) {
    if (element) {
        std::cout << "RadioButton selected." << std::endl;
    }
}


int main(int argc, char* argv[]) {
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Inicjalizacja SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Utworzenie okna
    SDL_Window* window = SDL_CreateWindow("GUI Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Utworzenie renderera
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Inicjalizacja menedżerów
    GUIManager gui_manager;
    TextureManager texture_manager(renderer);
    FontManager font_manager;

    // Załadowanie czcionki
    SharedFont font = font_manager.loadFont("assets/ARIAL.TTF", 20); // Użyj istniejącej czcionki
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        // Kontynuuj bez czcionki lub zakończ, w zależności od wymagań
    }

    // --- Tworzenie widgetów ---

    // Panel
    auto panel = std::make_unique<Panel>(50, 50, 700, 500);
    panel->setBorderColor(0, 255, 0, 255); // Zielona ramka
    panel->setBorderThickness(2);

    // Przycisk
    auto button = std::make_unique<Button>(10, 10, 100, 40); // Pozycja względem panelu
    // button->setTexture(texture_manager.loadTexture("assets/button.png")); // Jeśli masz teksturę przycisku
    button->setOnClickCallback(onButtonClick);
    panel->addChild(button.get()); // Dodaj przycisk do panelu

    // Suwak poziomy
    auto horizontalSlider = std::make_unique<Slider>(120, 10, 200, 30, 0, 100, 50, Orientation::Horizontal);
    horizontalSlider->setOnChangeCallback(onSliderValueChanged);
    panel->addChild(horizontalSlider.get());

    // Suwak pionowy
    auto verticalSlider = std::make_unique<Slider>(10, 60, 30, 200, 0, 100, 50, Orientation::Vertical);
    verticalSlider->setOnChangeCallback(onSliderValueChanged);
    panel->addChild(verticalSlider.get());

    // Pole tekstowe
    auto textInput = std::make_unique<TextInput>(120, 60, 200, 40);
    if (font) {
        textInput->setFont(font);
        textInput->setTextColor({255, 255, 255, 255}); // Biały tekst
    }
    textInput->setOnTextChanged(onTextInputTextChanged);
    textInput->setOnEnterPressed(onTextInputEnterPressed);
    panel->addChild(textInput.get());

    // Checkbox
    auto checkbox = std::make_unique<Checkbox>(120, 110, 200, 30, "Enable Option");
     if (font) {
        checkbox->setFont(font);
        checkbox->setTextColor({255, 255, 255, 255}); // Biały tekst
    }
    checkbox->setOnChange(onCheckboxChanged);
    panel->addChild(checkbox.get());

    // Grupa Radio Buttonów
    auto radioGroup = std::make_unique<RadioGroup>();

    // Radio Button 1
    auto radio1 = std::make_unique<RadioButton>(120, 150, 150, 30, "Choice One");
    if (font) {
        radio1->setFont(font);
        radio1->setTextColor({255, 255, 255, 255}); // Biały tekst
    }
    radio1->setOnChange(onRadioButtonSelected);
    radioGroup->addRadioButton(radio1.get());
    panel->addChild(radio1.get());

    // Radio Button 2
    auto radio2 = std::make_unique<RadioButton>(120, 190, 150, 30, "Choice Two");
     if (font) {
        radio2->setFont(font);
        radio2->setTextColor({255, 255, 255, 255}); // Biały tekst
    }
    radio2->setOnChange(onRadioButtonSelected);
    radioGroup->addRadioButton(radio2.get());
    panel->addChild(radio2.get());


    // Dodanie panelu (jako głównego kontenera) do menedżera GUI
    gui_manager.addElement(panel.release()); // Przekazanie własności do menedżera

    // --- Główna pętla aplikacji ---
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Obsługa zdarzeń
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Przekazanie zdarzenia do menedżera GUI
            quit = gui_manager.handleEvents(); // handleEvents nie przyjmuje argumentu e
        }

        // Czyszczenie renderera
        SDL_SetRenderDrawColor(renderer, 0x1E, 0x1E, 0x1E, 0xFF); // Ciemnoszare tło
        SDL_RenderClear(renderer);

        // Renderowanie elementów GUI
        gui_manager.render(renderer);

        // Aktualizacja ekranu
        SDL_RenderPresent(renderer);
    }

    // Czyszczenie zasobów SDL
    // Menedżer GUI powinien zwolnić swoje elementy
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
