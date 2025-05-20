#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include "gui_manager.hpp" // Dodano include dla menedżera GUI
#include <iostream> // Dodano include dla std::cout
#include "gui.hpp" // Dodano include dla Panel i Button
#include "texture_manager.hpp" // Dodano include dla TextureManager

// Funkcja zwrotna dla przycisku
void onButtonClick() {
    std::cout << "Przycisk został kliknięty!" << std::endl;
}

// Funkcja zwrotna dla innego przycisku
void onAnotherButtonClick() {
    std::cout << "Inny przycisk został kliknięty!" << std::endl;
}

int main(int argc, char const * argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    auto* window = SDL_CreateWindow("gui test", -1, -1, 800, 600, 0);
    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    GUIManager gui_manager; // Utworzono instancję menedżera GUI bez argumentów
    TextureManager texture_manager(renderer); // Utworzono instancję menedżera tekstur

    // Załadowanie tekstur
    // Załadowanie tekstur
    SharedTexture button_texture_1 = texture_manager.loadTexture("assets/button1.png"); // Załóżmy, że masz takie pliki
    SharedTexture button_texture_2 = texture_manager.loadTexture("assets/button2.png");

    // Utworzenie paneli
    Panel* panel1 = new Panel(50, 50, 300, 200);
    panel1->setBorderColor(255, 0, 0, 255); // Czerwone obramowanie
    panel1->setBorderThickness(2);

    Panel* panel2 = new Panel(400, 50, 350, 400);
    panel2->setBorderColor(0, 255, 0, 255); // Zielone obramowanie
    panel2->setBorderThickness(3);

    // Utworzenie przycisków
    Button* button1 = new Button(10, 10, 100, 40); // Przycisk na panelu 1
    button1->setOnClickCallback(onButtonClick);
    // button1->setText("Przycisk 1"); // Usunięto setText

    Button* button2 = new Button(120, 10, 100, 40); // Przycisk na panelu 1
    button2->setOnClickCallback(onAnotherButtonClick);
    // button2->setText("Przycisk 2"); // Usunięto setText
    button2->setTexture(button_texture_1); // Przycisk z teksturą

    Button* button3 = new Button(20, 300, 150, 50); // Przycisk na panelu 2
    button3->setOnClickCallback(onButtonClick);
    // button3->setText("Przycisk 3"); // Usunięto setText
    button3->setTexture(button_texture_1); // Ten sam tekstura co button2

    Button* button4 = new Button(180, 300, 150, 50); // Przycisk na panelu 2
    button4->setOnClickCallback(onAnotherButtonClick);
    // button4->setText("Przycisk 4"); // Usunięto setText
    button4->setTexture(button_texture_2); // Inna tekstura

    Button* button5 = new Button(20, 360, 310, 30); // Przycisk na panelu 2
    button5->setOnClickCallback(onButtonClick);
    // button5->setText("Przycisk 5"); // Usunięto setText


    // Dodanie przycisków do paneli
    panel1->addChild(button1);
    panel1->addChild(button2);

    panel2->addChild(button3);
    panel2->addChild(button4);
    panel2->addChild(button5);

    // Dodanie paneli do menedżera GUI
    gui_manager.addElement(panel1);
    gui_manager.addElement(panel2);

    // Istniejący przycisk (można go dodać do menedżera lub panelu)
    // Na potrzeby przykładu dodajmy go bezpośrednio do menedżera GUI
    Button* existing_button = new Button(100, 400, 150, 50);
    existing_button->setOnClickCallback(onButtonClick);
    // existing_button->setText("Istniejący Przycisk"); // Usunięto setText
    gui_manager.addElement(existing_button);


    uint should_close = false;
    while (!should_close)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 127);
        SDL_RenderClear(renderer);

        gui_manager.render(renderer); // Wywołanie renderowania GUI z przekazaniem renderera

        SDL_RenderPresent(renderer);

        // Obsługa zdarzeń przez menedżera GUI
        should_close = gui_manager.handleEvents();
    }


    SDL_Quit();
    return 0;
}
