#pragma once

#include "panel.hpp"

class RadioButton;

class RadioGroup : public Panel {
public:
    RadioGroup(GUIManager& manager, int x, int y, int w, int h);

    RadioButton* addOption(std::string_view text, bool selected = false);

    void setOptionSpacing(int spacing);
    void setOptionMargins(int buttonX, int labelX, int startY);
    void setOptionSizes(int buttonSize, int labelFontSize);

    void onButtonSelected(RadioButton* selectedButton);
    RadioButton* getSelectedButton() const;

    using SelectionChangeCallback = std::function<void(int index, const std::string& text)>;
    void setOnSelectionChange(SelectionChangeCallback callback);

    ComponentType getComponentTypeId() const override;

private:
    int m_nextOptionY = 20;
    int m_optionSpacing = 40;
    int m_buttonX = 20;
    int m_labelX = 45;
    int m_buttonSize = 20;
    int m_labelFontSize = 16;
    SelectionChangeCallback m_onSelectionChange;
    std::vector<std::string> m_optionTexts;
};
