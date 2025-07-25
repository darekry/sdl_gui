#include "radio_group.hpp"
#include "radio_button.hpp"
import std.compat;

RadioGroup::RadioGroup(GUIManager& manager, int x, int y, int w, int h)
    : Panel(manager, x, y, w, h) {
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