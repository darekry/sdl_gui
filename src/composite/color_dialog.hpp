#pragma once

#include "../button.hpp"
#include "../label.hpp"
#include "../panel.hpp"
#include "../slider.hpp"
#include "../text_input.hpp"
#include "modal_dialog.hpp"

#include "std.hpp"

/**
 * @file color_dialog.hpp
 * @brief ColorDialog - modal RGB/HSV color picker (parity #9).
 *
 * Composition of existing widgets (like FileDialog, no new primitives):
 * 3x Slider R/G/B (0-255) + 3x Slider H (0-360) / S,V (0-100) + HEX
 * TextInput (#RRGGBB) + old/new preview Panels + preset Button grid
 * + OK/Cancel. Two-way sync guarded by m_syncing (Slider::setValue and
 * TextInput::setText both fire callbacks synchronously — without the guard
 * any refresh would recurse). Conversions run only in the event path,
 * never per-frame.
 *
 * Created from code via create() (like DialogBox::create*); not in
 * WidgetFactory (same precedent as the other dialogs).
 */

class ColorDialog : public ModalDialog {
public:
    using ColorCallback = std::function<void(SDL_Color)>;

    /// Creates a centered dialog, adds it to the manager, returns raw pointer.
    static ColorDialog* create(GUIManager& manager,
                               std::string_view title,
                               SDL_Color initial,
                               ColorCallback callback = nullptr);

    ColorDialog(GUIManager& manager, int x, int y, int width, int height,
                std::string_view title, SDL_Color initial,
                ColorCallback callback = nullptr);

    void setColor(SDL_Color c);
    [[nodiscard]] SDL_Color getColor() const { return m_color; }
    [[nodiscard]] SDL_Color getInitial() const { return m_initial; }

    [[nodiscard]] ComponentType getComponentTypeId() const override;

    // Int-math conversions (h: 0-360, s/v: 0-100). Public for unit tests.
    static void rgbToHsv(uint8_t r, uint8_t g, uint8_t b, int& h, int& s, int& v);
    static void hsvToRgb(int h, int s, int v, uint8_t& r, uint8_t& g, uint8_t& b);
    static std::optional<SDL_Color> parseHex(std::string_view text);

protected:
    void draw(SDL_Renderer* renderer) override;
    // OK path: callback + close (base ModalDialog key path feeds Enter here).
    void onConfirm() override;
    // Proportional layout from the dialog size + StackLayout button strip.
    void layoutChildren() override;

private:
    void refreshFromColor();  // pushes m_color to sliders/HEX/preview (guarded)
    void onSliderChanged(GUIElement* src);  // pulls slider state into m_color, then refreshes
    void onHexChanged();      // parses HEX input into m_color (bad input ignored)

    static constexpr int kPresetCount = 12;
    static const SDL_Color kPresets[kPresetCount];

    std::string m_title;
    int m_titleBarHeight = 30;

    SDL_Color m_color;
    SDL_Color m_initial;
    ColorCallback m_callback;
    bool m_syncing = false;

    Slider* m_rSlider = nullptr;
    Slider* m_gSlider = nullptr;
    Slider* m_bSlider = nullptr;
    Slider* m_hSlider = nullptr;
    Slider* m_sSlider = nullptr;
    Slider* m_vSlider = nullptr;
    std::vector<Label*> m_rowTags;
    Label* m_hexLabel = nullptr;
    TextInput* m_hexInput = nullptr;
    Panel* m_oldPreview = nullptr;
    Panel* m_newPreview = nullptr;
    std::vector<Button*> m_presets;
    Button* m_okBtn = nullptr;
    Button* m_cancelBtn = nullptr;
};
