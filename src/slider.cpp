#include "slider.hpp"
#include "gui_manager.hpp"
#include "label.hpp"


Slider::Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation)
    : Panel(manager, x, y, width, height),
      m_orientation(orientation),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_currentValue(std::clamp(initialValue, minValue, maxValue)) {

    setClipChildren(true);
    auto buttonSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();
    
    if (m_orientation == Orientation::Horizontal) {
        m_trackOffsetX = buttonSize;
        m_trackSize = getWidth() - 2 * buttonSize;
        m_decreaseButton = std::make_unique<Button>(manager, 0, 0, buttonSize, getHeight());
        m_increaseButton = std::make_unique<Button>(manager, getWidth() - buttonSize, 0, buttonSize, getHeight());
    } else { // Vertical
        m_trackOffsetY = buttonSize;
        m_trackSize = getHeight() - 2 * buttonSize;
        m_decreaseButton = std::make_unique<Button>(manager, 0, 0, getWidth(), buttonSize);
        m_increaseButton = std::make_unique<Button>(manager, 0, getHeight() - buttonSize, getWidth(), buttonSize);
    }
    
    decrementButton = m_decreaseButton.get();
    incrementButton = m_increaseButton.get();

    auto decLabel = std::make_unique<Label>(manager, 0, 0, "<", 12);
    decLabel->setPosition((decrementButton->getWidth() - decLabel->getWidth()) / 2, (decrementButton->getHeight() - decLabel->getHeight()) / 2);
    decrementButton->addChild(std::move(decLabel));

    auto incLabel = std::make_unique<Label>(manager, 0, 0, ">", 12);
    incLabel->setPosition((incrementButton->getWidth() - incLabel->getWidth()) / 2, (incrementButton->getHeight() - incLabel->getHeight()) / 2);
    incrementButton->addChild(std::move(incLabel));
 
    m_decreaseButton->setOnClickCallback([this](GUIElement*) {
       setValue(m_currentValue - 1);
    });
 
    m_increaseButton->setOnClickCallback([this](GUIElement*) {
       setValue(m_currentValue + 1);
    });
 
    addChild(std::move(m_decreaseButton));
    addChild(std::move(m_increaseButton));
}

void Slider::updateValueFromMouse(int mouseX, int mouseY) {
    // Guard against division by zero
    if (m_trackSize <= 0) {
        return;
    }
    
    auto absPos = getAbsolutePosition();
    float ratio = 0.0f;

    if (m_orientation == Orientation::Horizontal) {
        float relativeMouseX = static_cast<float>(mouseX - (absPos.x + m_trackOffsetX));
        ratio = std::clamp(relativeMouseX / static_cast<float>(m_trackSize), 0.0f, 1.0f);
    } else {
        float relativeMouseY = static_cast<float>(mouseY - (absPos.y + m_trackOffsetY));
        ratio = std::clamp(relativeMouseY / static_cast<float>(m_trackSize), 0.0f, 1.0f);
    }

    int newValue = m_minValue + static_cast<int>(ratio * static_cast<float>(m_maxValue - m_minValue));
    setValue(newValue);
}

bool Slider::handleEvent(const SDL_Event& e) {
    if (!m_enabled) return false;

    if (GUIElement::handleEvent(e)) {
        return true;
    }
    
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        auto absPos = getAbsolutePosition();
        SDL_Rect trackArea;
        if (m_orientation == Orientation::Horizontal) {
            trackArea = {absPos.x + m_trackOffsetX, absPos.y, m_trackSize, getHeight()};
        } else {
            trackArea = {absPos.x, absPos.y + m_trackOffsetY, getWidth(), m_trackSize};
        }
        SDL_Point mousePoint = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)};
        if (SDL_PointInRect(&mousePoint, &trackArea)) {
            m_isDragging = true;
            updateValueFromMouse(e.button.x, e.button.y);
            return true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_isDragging) {
            m_isDragging = false;
            return true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
        if (m_isDragging) {
            updateValueFromMouse(e.motion.x, e.motion.y);
            return true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_WHEEL && m_isHovered) {
        // Obsługa kółka myszy - zmiana wartości gdy slider jest hoverowany
        int delta = e.wheel.y * m_wheelStep;
        if (delta != 0) {
            setValue(m_currentValue + delta);
            return true;
        }
    }
    return false;
}

const char* Slider::getComponentType() const {
    return "Slider";
}

void Slider::setValue(int value) {
    auto oldValue = m_currentValue;
    m_currentValue = std::clamp(value, m_minValue, m_maxValue);
    if (m_currentValue != oldValue) {
        markDirty();
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void Slider::draw(SDL_Renderer* renderer) {
    Panel::draw(renderer);
    
    const auto& style = getComposedStyle(m_state);
    
    // Rysowanie na buforze, więc pozycje są względne (0,0)
    SDL_Color trackColor;
    if (style.backgroundColor) {
        trackColor = *style.backgroundColor;
        trackColor.r = std::max<uint8_t>(uint8_t{0}, trackColor.r - 20);
        trackColor.g = std::max<uint8_t>(uint8_t{0}, trackColor.g - 20);
        trackColor.b = std::max<uint8_t>(uint8_t{0}, trackColor.b - 20);
    } else {
        trackColor = {.r=200, .g=200, .b=200, .a=255};
    }
    SDL_SetRenderDrawColor(renderer, trackColor.r, trackColor.g, trackColor.b, trackColor.a);
    
    SDL_Rect trackRect;
    const int trackThickness = 4;
    if (m_orientation == Orientation::Horizontal) {
        trackRect = {m_trackOffsetX, getHeight() / 2 - trackThickness / 2, m_trackSize, trackThickness};
    } else {
        trackRect = {getWidth() / 2 - trackThickness / 2, m_trackOffsetY, trackThickness, m_trackSize};
    }
    ({ SDL_FRect _fr = {static_cast<float>(trackRect.x), static_cast<float>(trackRect.y), static_cast<float>(trackRect.w), static_cast<float>(trackRect.h)}; SDL_RenderFillRect(renderer, &_fr); });

    SDL_Color thumbColor = style.borderColor.value_or(SDL_Color{100, 100, 100, 255});
    SDL_SetRenderDrawColor(renderer, thumbColor.r, thumbColor.g, thumbColor.b, thumbColor.a);

    SDL_Rect thumbRect;
    int thumbSize = (m_orientation == Orientation::Horizontal) ? std::min(getHeight(), 20) : std::min(getWidth(), 20);
    
    float ratio = (m_maxValue > m_minValue) ? static_cast<float>(m_currentValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;

    if (m_orientation == Orientation::Horizontal) {
        int thumbX = m_trackOffsetX + static_cast<int>(ratio * static_cast<float>(m_trackSize - thumbSize));
        thumbRect = {.x=thumbX, .y=getHeight() / 2 - thumbSize / 2, .w=thumbSize, .h=thumbSize};
    } else {
        int thumbY = m_trackOffsetY + static_cast<int>(ratio * static_cast<float>(m_trackSize - thumbSize));
        thumbRect = {.x=getWidth() / 2 - thumbSize / 2, .y=thumbY, .w=thumbSize, .h=thumbSize};
    }
    
    ({ SDL_FRect _fr = {static_cast<float>(thumbRect.x), static_cast<float>(thumbRect.y), static_cast<float>(thumbRect.w), static_cast<float>(thumbRect.h)}; SDL_RenderFillRect(renderer, &_fr); });
}

Button* Slider::getDecrementButton() {
    return decrementButton;
}

Button* Slider::getIncrementButton() {
    return incrementButton;
}
