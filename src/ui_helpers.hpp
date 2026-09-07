#pragma once
// ui_helpers.hpp — free UI glue: data binding, generators, small builders.
//
// Extension point for helpers that don't belong to any single widget:
// add a declaration here + definition in ui_helpers.cpp. Keep each helper
// dependency-free (operates on public widget API) and liveness-safe
// (capture ElementRef, never raw widget pointers, in stored callbacks).
#include "slider.hpp"
#include "range_slider.hpp"
#include "label.hpp"

// Mirrors a Slider value into a Label, refreshed immediately and on every
// change. Replaces the hand-written setOnChangeCallback + to_string glue.
// NOTE: one binding per widget — linking the same slider again replaces the
// previous binding (same semantics as setOnChangeCallback).
void linkLabel(Slider& slider, Label& label,
               std::string prefix = {}, std::string suffix = {});

// Same, with a custom formatter (e.g. [](int v){ return std::to_string(v / 60) + " min"; }).
void linkLabel(Slider& slider, Label& label, std::function<std::string(int)> format);

// Mirrors a RangeSlider [lo, hi] window into a Label (default "[lo, hi]").
void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::string prefix = "[", std::string mid = ", ", std::string suffix = "]");

// Same, with a custom formatter receiving (lower, upper).
void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::function<std::string(int, int)> format);
