#include "SDL2/SDL.h"
#include "gui_manager.hpp"
#include "gui.hpp"
#include "timer_manager.hpp"
#include "panel.hpp"
#include "label.hpp"

static constexpr int DEFAULT_FONT_SIZE = 24;
static constexpr int TOOLTIP_FONT_SIZE = 14;
static constexpr int TOOLTIP_PADDING = 5;
static const SDL_Color TOOLTIP_BG_COLOR = {.r=255, .g=255, .b=225, .a=255};

GUIManager::GUIManager(SDL_Renderer* renderer)
    : tooltipElement(nullptr), m_renderer(renderer), m_textureManager(renderer), m_theme(Theme::createDefaultTheme()) {
    timerManager = std::make_unique<TimerManager>();
    animation_manager = std::make_unique<AnimationManager>();

    // Załaduj domyślną czcionkę
    m_fontManager.loadDefaultFont("assets/fonts/font.ttf", DEFAULT_FONT_SIZE);

    // Utwórz domyślną teksturę zastępczą
    m_textureManager.createDefaultTexture(m_renderer, m_fontManager, "No Texture");
}

GUIManager::~GUIManager() = default;


GUIElement* GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        auto* raw_ptr = element.get();
        m_elements.push_back(std::move(element)); // Przenieś własność do wektora
        return raw_ptr;
    }
    return nullptr;
}

bool GUIManager::processEvent(const SDL_Event& event) {
    // Przekaż zdarzenie do wszystkich elementów najwyższego poziomu.
    // Pętla zatrzyma się, gdy któryś element "skonsumuje" zdarzenie.
    for (const auto& element : m_elements) {
        if (element && element->handleEvent(event)) {
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
            element->render(m_renderer);
        }
    }
    
    if (tooltipElement) {
        tooltipElement->render(m_renderer);
    }
}

void GUIManager::cleanup() {
    // Zaktualizuj timery i animacje
    timerManager->update();
    animation_manager->update();

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
size_t total_removed_count = 0;
for (const auto& element : m_elements) {
    if (element && element->isMarkedForDeletion()) {
        total_removed_count += 1 + element->countDescendants();
    }
}

auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                              [](const std::unique_ptr<GUIElement>& element) {
    return element->isMarkedForDeletion();
});

std::size_t prefix_distance = static_cast<std::size_t>(std::distance(m_elements.begin(), new_end));
if (prefix_distance < m_elements.size())
{
    m_elements.erase(new_end, m_elements.end());
}

if (total_removed_count > 0) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GUIManager::cleanup(): Removed %zu elements in total.", total_removed_count);
}
}

void GUIManager::showTooltip(GUIElement* target, const std::string& text) {
    if (!target) return;

    const int fontSize = TOOLTIP_FONT_SIZE;
    const int padding = TOOLTIP_PADDING;

    // Precyzyjne obliczanie rozmiaru tekstu
    int textWidth = 0;
    int textHeight = 0;
    m_fontManager.getTextSize(text, "assets/fonts/font.ttf", fontSize, &textWidth, &textHeight);
    
    auto targetPos = target->getAbsolutePosition();
    int posX = targetPos.x;
    int posY = targetPos.y + target->getHeight();

    // Utwórz panel
    auto panel = std::make_unique<Panel>(*this, posX, posY, (textWidth + (2 * padding)), (textHeight + (2 * padding)));
    panel->setBackgroundColor(ElementState::Normal, TOOLTIP_BG_COLOR);
    panel->setBorder(ElementState::Normal, {0, 0, 0, 255}, 1);

    // Utwórz etykietę i dodaj ją do panelu
    auto label = std::make_unique<Label>(*this, padding, padding, text, fontSize);
    panel->addChild(std::move(label));

    tooltipElement = std::move(panel);
}

void GUIManager::hideTooltip() {
    tooltipElement.reset();
}


TimerManager* GUIManager::getTimerManager() {
    return timerManager.get();
}

void GUIManager::setTheme(Theme theme) {
    m_theme = std::move(theme);
}

Theme& GUIManager::getTheme() {
    return m_theme;
}

AnimationManager* GUIManager::getAnimationManager() {
    return animation_manager.get();
}
