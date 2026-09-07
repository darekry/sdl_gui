#include "ui_helpers.hpp"
#include "gui_manager.hpp"

void linkLabel(Slider& slider, Label& label, std::string prefix, std::string suffix) {
    linkLabel(slider, label, [p = std::move(prefix), s = std::move(suffix)](int v) {
        return p + std::to_string(v) + s;
    });
}

void linkLabel(Slider& slider, Label& label, std::function<std::string(int)> format) {
    auto labelRef = slider.getManager().makeRef(&label);
    auto fmt = std::move(format);
    // Refresh immediately so the label never shows a stale initial text.
    if (labelRef) labelRef->setText(fmt(slider.getValue()));
    slider.setOnChangeCallback([labelRef, fmt = std::move(fmt)](GUIElement* e) {
        auto* s = static_cast<Slider*>(e);
        if (s && labelRef) labelRef->setText(fmt(s->getValue()));
    });
}

void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::string prefix, std::string mid, std::string suffix) {
    linkRangeLabel(slider, label,
                   [p = std::move(prefix), m = std::move(mid), s = std::move(suffix)](int lo, int hi) {
                       return p + std::to_string(lo) + m + std::to_string(hi) + s;
                   });
}

void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::function<std::string(int, int)> format) {
    auto labelRef = slider.getManager().makeRef(&label);
    auto fmt = std::move(format);
    if (labelRef) labelRef->setText(fmt(slider.getLowerValue(), slider.getUpperValue()));
    slider.setOnChangeCallback([labelRef, fmt = std::move(fmt)](GUIElement* e) {
        auto* s = static_cast<RangeSlider*>(e);
        if (s && labelRef) labelRef->setText(fmt(s->getLowerValue(), s->getUpperValue()));
    });
}
