#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "gui.hpp"
#include "timer_manager.hpp"
#include "panel.hpp"
#include "label.hpp"
import std.compat;

GUIManager::GUIManager(SDL_Renderer* renderer)
    : m_renderer(renderer), m_fontManager(), m_textureManager(renderer) {
    timerManager = std::make_unique<TimerManager>();
    tooltipElement = nullptr;
    // Załaduj domyślną czcionkę
    m_fontManager.loadDefaultFont("assets/fonts/font.ttf", 24);

    // Utwórz domyślną teksturę zastępczą
    m_textureManager.createDefaultTexture(m_renderer, m_fontManager, "No Texture");
}

GUIManager::~GUIManager() {
    // Obiekty m_fontManager i m_textureManager są automatycznie niszczone,
    // a unique_ptrs w m_elements dbają o zwolnienie pamięci po elementach GUI.
}


GUIElement* GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        auto* raw_ptr = element.get();
        m_elements.push_back(std::move(element)); // Przenieś własność do wektora
        return raw_ptr;
    }
    return nullptr;
}

bool GUIManager::processEvent(const SDL_Event& e) {
    // Przekaż zdarzenie do wszystkich elementów najwyższego poziomu.
    // Pętla zatrzyma się, gdy któryś element "skonsumuje" zdarzenie.
    for (const auto& element : m_elements) {
        if (element && element->handleEvent(e)) {
            // Jeśli element obsłużył zdarzenie, zwracamy true.
            return true;
        }
    }
    // Żaden element nie obsłużył zdarzenia.
    return false;
}

void GUIManager::render() {
    // Renderuj wszystkie zarządzane elementy
    for (const auto& element : m_elements) { // Iteracja po unique_ptr
        if (element) {
            element->render();
        }
    }
    
    if (tooltipElement) {
        tooltipElement->render();
    }
}

void GUIManager::cleanup() {
    // Zaktualizuj timery
    timerManager->update();

    if (tooltipElement && tooltipElement->isMarkedForDeletion()) {
        tooltipElement.reset();
    }
    
    // Najpierw rekurencyjnie wywołaj cleanup dla wszystkich elementów
    for (const auto& element : m_elements) {
        if (element) {
            element->cleanup();
        }
    }

    // Następnie usuń oznaczone elementy z głównego kontenera
    auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });
    
    m_elements.erase(new_end, m_elements.end());
}

void GUIManager::showTooltip(GUIElement* target, const std::string& text) {
    if (!target) return;

    const int fontSize = 14;
    const int padding = 5;

    // Precyzyjne obliczanie rozmiaru tekstu
    int textWidth, textHeight;
    m_fontManager.getTextSize(text, "assets/fonts/font.ttf", fontSize, &textWidth, &textHeight);
    
    auto targetPos = target->getAbsolutePosition();
    int x = targetPos.x;
    int y = targetPos.y + target->getHeight();

    // Utwórz panel
    auto panel = std::make_unique<Panel>(*this, x, y, textWidth + 2 * padding, textHeight + 2 * padding);
    panel->setBorderThickness(1);
    panel->setBackgroundColor({255, 255, 225, 255}); // Jasnożółte tło

    // Utwórz etykietę i dodaj ją do panelu
    auto label = std::make_unique<Label>(*this, padding, padding, text, fontSize, SDL_Color{0, 0, 0, 255});
    panel->addChild(std::move(label));

    tooltipElement = std::move(panel);
}

void GUIManager::hideTooltip() {
    tooltipElement.reset();
}


TimerManager* GUIManager::getTimerManager() {
    return timerManager.get();
}


