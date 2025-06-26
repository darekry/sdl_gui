#include "tab_control.hpp"
#include "SDL2/SDL_ttf.h"
#include <iostream>
#include "gui_manager.hpp"
#include "font_manager.hpp"

// --- Implementacja TabControl ---

TabControl::TabControl(int x, int y, int width, int height)
    : GUIElement(x, y, width, height) {
}

Panel* TabControl::addTab(const std::string& title) {
    if (!getGUIManager()) return nullptr;
    SDL_Renderer* renderer = getGUIManager()->getRenderer();
    FontManager* fontManager = getGUIManager()->getFontManager();
    if (!renderer || !fontManager) return nullptr;

    SharedFont font = fontManager->loadFont("assets/ARIAL.TTF", 16); // Load default font
    if (!font) {
        std::cerr << "Failed to load font for tab button" << std::endl;
        // Continue without text? Or return nullptr? For now, continue.
    }

    // Tworzenie przycisku dla zakładki
    auto button = std::make_unique<Button>(0, 0, 100, m_tabButtonHeight);
    if (font) {
        SDL_Color textColor = {255, 255, 255, 255}; // Biały tekst
        SharedTexture textTexture = createTextTexture(renderer, font, title, textColor);
        button->setTexture(textTexture);
    }
    
    Button* buttonPtr = button.get();
    m_tabButtons.push_back(buttonPtr);

    // Ustawienie callbacku na kliknięcie przycisku
    button->setOnClickCallback([this, buttonPtr](GUIElement*){
        setActiveTab(buttonPtr);
    });

    // Tworzenie panelu na zawartość zakładki
    auto panel = std::make_unique<Panel>(0, m_tabButtonHeight, m_width, m_height - m_tabButtonHeight);
    panel->setEnabled(false); // Domyślnie nieaktywny
    Panel* panelPtr = panel.get();
    m_tabPanels.push_back(panelPtr);

    // Dodaj przycisk i panel jako dzieci TabControl
    addChild(std::move(button));
    addChild(std::move(panel));

    // Jeśli to pierwsza zakładka, ustaw ją jako aktywną
    if (m_tabButtons.size() == 1) {
        setActiveTab(buttonPtr);
    }

    reorderTabs();
    return panelPtr;
}

void TabControl::setActiveTab(Button* tabButton) {
    if (m_activeTabButton == tabButton) {
        return; // Już aktywna
    }

    // Deaktywuj poprzednią aktywną zakładkę
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            m_tabPanels[i]->setEnabled(false);
            break;
        }
    }

    // Aktywuj nową zakładkę
    m_activeTabButton = tabButton;
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            m_tabPanels[i]->setEnabled(true);
            break;
        }
    }
}

void TabControl::reorderTabs() {
    int current_x = 0;
    for (Button* button : m_tabButtons) {
        button->setPosition(current_x, 0);
        current_x += button->getWidth() + 5; // 5px przerwy
    }
}

// handleEvent i render są teraz poprawnie obsługiwane przez klasę bazową GUIElement