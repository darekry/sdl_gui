#include "radio_group.hpp"
#include "radio_button.hpp" // Potrzebne do pracy z obiektami RadioButton

void RadioGroup::addRadioButton(RadioButton* button) {
    if (button) {
        m_buttons.push_back(button);
        button->setGroup(this); // Ustaw wskaźnik do tej grupy w przycisku
    }
}

void RadioGroup::buttonSelected(RadioButton* selectedButton) {
    // Odznacz wszystkie inne przyciski w grupie
    for (RadioButton* button : m_buttons) {
        if (button != selectedButton && button->isSelected()) {
            button->setSelected(false);
        }
    }
    // Upewnij się, że wybrany przycisk jest zaznaczony (setSelected w RadioButton wywoła callback)
    if (selectedButton && !selectedButton->isSelected()) {
        selectedButton->setSelected(true);
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