#ifndef RADIOGROUP_HPP
#define RADIOGROUP_HPP

#include <vector>
#include <algorithm> // Dla std::remove

// Forward declaration
class RadioButton;

class RadioGroup {
public:
    // Metoda do dodawania RadioButtona do grupy
    void addRadioButton(RadioButton* button);

    // Metoda wywoływana przez RadioButton, gdy zostanie zaznaczony
    void buttonSelected(RadioButton* selectedButton);

    // Metoda do usuwania RadioButtona z grupy (opcjonalnie, do rozważenia w przyszłości)
    // void removeRadioButton(RadioButton* button);

    // Metoda zwracająca wskaźnik do aktualnie zaznaczonego RadioButtona (opcjonalnie)
    // RadioButton* getSelectedButton() const;

private:
    std::vector<RadioButton*> m_buttons;
};

#endif // RADIOGROUP_HPP