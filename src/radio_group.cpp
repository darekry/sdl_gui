#include "radio_group.hpp"
#include "radio_button.hpp"
#include "label.hpp"


RadioGroup::RadioGroup(GUIManager& manager, int x, int y, int w, int h)
    : Panel(manager, x, y, w, h) {
}

RadioButton* RadioGroup::addOption(std::string_view text, bool selected) {
    auto rb = std::make_unique<RadioButton>(m_manager, m_buttonX, m_nextOptionY, m_buttonSize, m_buttonSize);
    RadioButton* rbPtr = rb.get();
    if (selected) {
        rb->setSelected(true);
    }
    
    auto label = std::make_unique<Label>(m_manager, m_labelX, m_nextOptionY, text, m_labelFontSize);
    
    addChild(std::move(rb));
    addChild(std::move(label));
    
    m_nextOptionY += m_optionSpacing;
    
    return rbPtr;
}

void RadioGroup::setOptionSpacing(int spacing) {
    m_optionSpacing = spacing;
}

void RadioGroup::setOptionMargins(int buttonX, int labelX, int startY) {
    m_buttonX = buttonX;
    m_labelX = labelX;
    m_nextOptionY = startY;
}

void RadioGroup::setOptionSizes(int buttonSize, int labelFontSize) {
    m_buttonSize = buttonSize;
    m_labelFontSize = labelFontSize;
}

void RadioGroup::onButtonSelected(RadioButton* selectedButton) {
    for (const auto& child : getChildren()) {
        if (auto* rb = dynamic_cast<RadioButton*>(child.get())) {
            if (rb != selectedButton && rb->isSelected()) {
                rb->setSelected(false);
            }
        }
    }
}

RadioButton* RadioGroup::getSelectedButton() const {
    for (const auto& child : getChildren()) {
        if (auto* rb = dynamic_cast<RadioButton*>(child.get())) {
            if (rb->isSelected()) {
                return rb;
            }
        }
    }
    return nullptr;
}