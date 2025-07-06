#include "radio_group.hpp"
#include "radio_button.hpp" // Potrzebne do pracy z obiektami RadioButton
import std.compat;

void RadioGroup::addRadioButton(RadioButton* button) {
    if (button) {
        m_buttons.push_back(button);
        button->setGroup(this); // Ustaw wskaźnik do tej grupy w przycisku
    }
}

void RadioGroup::buttonSelected(RadioButton* selectedButton) {
    for (RadioButton* button : m_buttons) {
        if (button != selectedButton) {
            // Odznacz inne przyciski, nie powiadamiając grupy (aby uniknąć pętli)
            if (button->isSelected()) {
                button->setSelected(false, false);
            }
        }
    }
    // Zaznacz wybrany przycisk, również bez powiadamiania grupy
    if (selectedButton && !selectedButton->isSelected()) {
        selectedButton->setSelected(true, false);
    }
}

// Opcjonalna metoda do usuwania RadioButtona z grupy
// void RadioGroup::removeRadioButton(RadioButton* button) {
//     m_buttons.erase(std::remove(m_buttons.begin(), m_buttons.end(), button), m_buttons.end());
// }

// Opcjonalna metoda zwracająca wskaźnik do aktualnie zaznaczonego RadioButtona
// RadioButton* RadioGroup::getSelectedButton() const {
//     for (RadioButton* button : m_buttons) {
//         if (button->isSelected()) {
//             return button;
//         }
//     }
//     return nullptr; // Brak zaznaczonego przycisku
// }