#ifndef RADIOGROUP_HPP
#define RADIOGROUP_HPP

#include "panel.hpp"


class RadioButton;

class RadioGroup : public Panel {
public:
    RadioGroup(GUIManager& manager, int x, int y, int w, int h);

    void onButtonSelected(RadioButton* selectedButton);
    RadioButton* getSelectedButton() const;
};

#endif // RADIOGROUP_HPP