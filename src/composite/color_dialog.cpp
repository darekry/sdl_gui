#include "color_dialog.hpp"
#include "../gui_manager.hpp"
#include "../layout.hpp"
#include "../sdl_rect_helpers.hpp"

#include "std.hpp"

const SDL_Color ColorDialog::kPresets[kPresetCount] = {
    {0, 0, 0, 255},       {255, 255, 255, 255},
    {255, 0, 0, 255},     {0, 255, 0, 255},
    {0, 0, 255, 255},     {255, 255, 0, 255},
    {0, 255, 255, 255},   {255, 0, 255, 255},
    {255, 128, 0, 255},   {128, 0, 255, 255},
    {128, 128, 128, 255}, {165, 42, 42, 255},
};

ColorDialog* ColorDialog::create(GUIManager& manager,
                                 std::string_view title,
                                 SDL_Color initial,
                                 ColorCallback callback) {
    constexpr int width = 460;
    constexpr int height = 560;
    int screenW = 0, screenH = 0;
    manager.getWindowSize(screenW, screenH);
    auto [x, y] = CenterRect(screenW, screenH, width, height);

    auto dialog = std::unique_ptr<ColorDialog>(
        new ColorDialog(manager, x, y, width, height, title, initial, callback));

    auto* raw = dialog.get();
    manager.addElement(std::move(dialog));
    return raw;
}

ColorDialog::ColorDialog(GUIManager& manager, int x, int y, int width, int height,
                         std::string_view title, SDL_Color initial,
                         ColorCallback callback)
    : ModalDialog(manager, x, y, width, height)
    , m_title(title)
    , m_color{initial.r, initial.g, initial.b, 255}
    , m_initial{initial.r, initial.g, initial.b, 255}
    , m_callback(std::move(callback)) {
    setClipChildren(false);
    setDraggable(true);

    Style dialogStyle;
    dialogStyle.backgroundColor = {240, 240, 240, 255};
    dialogStyle.borderColor = {100, 100, 100, 255};
    dialogStyle.borderWidth = 2;
    dialogStyle.borderRadius = 0;
    setStyle(ElementState::Normal, dialogStyle);

    int h = 0, s = 0, v = 0;
    rgbToHsv(m_color.r, m_color.g, m_color.b, h, s, v);

    auto titleLabel = std::make_unique<Label>(manager, 10, 5, title);
    titleLabel->setID("cd_title");
    addChild(std::move(titleLabel));

    auto oldPreview = std::make_unique<Panel>(manager, 0, 0, 10, 10);
    oldPreview->setID("cd_old");
    oldPreview->setBackgroundColor(ElementState::Normal, m_initial);
    m_oldPreview = oldPreview.get();
    addChild(std::move(oldPreview));

    auto newPreview = std::make_unique<Panel>(manager, 0, 0, 10, 10);
    newPreview->setID("cd_new");
    newPreview->setBackgroundColor(ElementState::Normal, m_color);
    m_newPreview = newPreview.get();
    addChild(std::move(newPreview));

    const char* rowTags[6] = {"R", "G", "B", "H", "S", "V"};
    Slider** rowSliders[6] = {&m_rSlider, &m_gSlider, &m_bSlider,
                              &m_hSlider, &m_sSlider, &m_vSlider};
    const int rowInit[6] = {m_color.r, m_color.g, m_color.b, h, s, v};
    const int rowMax[6] = {255, 255, 255, 360, 100, 100};
    for (int i = 0; i < 6; ++i) {
        auto tag = std::make_unique<Label>(manager, 0, 0, rowTags[i], 13);
        tag->setTextColor(ElementState::Normal, {60, 60, 60, 255});
        m_rowTags.push_back(tag.get());
        addChild(std::move(tag));

        auto slider = std::make_unique<Slider>(manager, 0, 0, 100, 26,
                                               0, rowMax[i], rowInit[i],
                                               Orientation::Horizontal);
        *rowSliders[i] = slider.get();
        addChild(std::move(slider));
    }

    auto hexLabel = std::make_unique<Label>(manager, 0, 0, "HEX:", 13);
    hexLabel->setTextColor(ElementState::Normal, {60, 60, 60, 255});
    m_hexLabel = hexLabel.get();
    addChild(std::move(hexLabel));

    auto hexInput = std::make_unique<TextInput>(manager, 0, 0, 120, 28);
    m_hexInput = hexInput.get();
    addChild(std::move(hexInput));

    auto presetLabel = std::make_unique<Label>(manager, 0, 0, "Presets:", 13);
    presetLabel->setID("cd_presets_label");
    presetLabel->setTextColor(ElementState::Normal, {60, 60, 60, 255});
    addChild(std::move(presetLabel));

    for (int i = 0; i < kPresetCount; ++i) {
        const SDL_Color preset = kPresets[i];
        auto btn = std::make_unique<Button>(manager, 0, 0, 10, 10, "");
        btn->setBackgroundColor(ElementState::Normal, preset);
        btn->setBackgroundColor(ElementState::Hover, preset);
        btn->setBackgroundColor(ElementState::Pressed, preset);
        btn->setBorder(ElementState::Normal, {100, 100, 100, 255}, 1);
        btn->setOnClickCallback([this, preset](GUIElement*) {
            setColor(preset);
        });
        m_presets.push_back(btn.get());
        addChild(std::move(btn));
    }

    auto okBtn = std::make_unique<Button>(manager, 0, 0, 80, 30, "OK");
    okBtn->setOnClickCallback([this](GUIElement*) {
        onConfirm();
    });
    m_okBtn = okBtn.get();
    addChild(std::move(okBtn));

    auto cancelBtn = std::make_unique<Button>(manager, 0, 0, 80, 30, "Cancel");
    cancelBtn->setOnClickCallback([this](GUIElement*) {
        close();
    });
    m_cancelBtn = cancelBtn.get();
    addChild(std::move(cancelBtn));

    // Wire sync AFTER all widgets exist with initial values, then normalize
    // (the guarded refresh pushes the canonical HEX/preview state).
    for (Slider* slider : {m_rSlider, m_gSlider, m_bSlider, m_hSlider, m_sSlider, m_vSlider}) {
        slider->setOnChangeCallback([this](GUIElement* src) {
            onSliderChanged(src);
        });
    }
    m_hexInput->setOnTextChanged([this](TextInput*) {
        onHexChanged();
    });

    refreshFromColor();
    layoutChildren();
}

void ColorDialog::setColor(SDL_Color c) {
    m_color = {c.r, c.g, c.b, 255};
    refreshFromColor();
}

void ColorDialog::onConfirm() {
    if (m_callback) m_callback(m_color);
    close();
}

void ColorDialog::refreshFromColor() {
    if (m_syncing) return;
    m_syncing = true;

    int h = 0, s = 0, v = 0;
    rgbToHsv(m_color.r, m_color.g, m_color.b, h, s, v);
    m_rSlider->setValue(m_color.r);
    m_gSlider->setValue(m_color.g);
    m_bSlider->setValue(m_color.b);
    m_hSlider->setValue(h);
    m_sSlider->setValue(s);
    m_vSlider->setValue(v);

    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", m_color.r, m_color.g, m_color.b);
    m_hexInput->setText(std::string_view(buf, 7));
    m_newPreview->setBackgroundColor(ElementState::Normal, m_color);

    m_syncing = false;
}

void ColorDialog::onSliderChanged(GUIElement* src) {
    if (m_syncing) return;
    if (src == m_hSlider || src == m_sSlider || src == m_vSlider) {
        uint8_t r = 0, g = 0, b = 0;
        hsvToRgb(m_hSlider->getValue(), m_sSlider->getValue(), m_vSlider->getValue(), r, g, b);
        m_color = {r, g, b, 255};
    } else {
        m_color = {static_cast<uint8_t>(m_rSlider->getValue()),
                   static_cast<uint8_t>(m_gSlider->getValue()),
                   static_cast<uint8_t>(m_bSlider->getValue()), 255};
    }
    refreshFromColor();
}

void ColorDialog::onHexChanged() {
    if (m_syncing) return;
    if (auto parsed = parseHex(m_hexInput->getText())) {
        m_color = *parsed;
        refreshFromColor();
    }
    // Bad input is ignored (no crash, dialog stays open).
}

void ColorDialog::rgbToHsv(uint8_t r, uint8_t g, uint8_t b, int& h, int& s, int& v) {
    const int ri = r, gi = g, bi = b;
    const int mx = std::max({ri, gi, bi});
    const int mn = std::min({ri, gi, bi});
    const int delta = mx - mn;

    v = mx * 100 / 255;
    s = (mx == 0) ? 0 : delta * 100 / mx;

    if (delta == 0) {
        h = 0;
    } else if (mx == ri) {
        h = (60 * (gi - bi) / delta) % 360;
        if (h < 0) h += 360;
    } else if (mx == gi) {
        h = 60 * (bi - ri) / delta + 120;
    } else {
        h = 60 * (ri - gi) / delta + 240;
    }
}

void ColorDialog::hsvToRgb(int h, int s, int v, uint8_t& r, uint8_t& g, uint8_t& b) {
    h = std::clamp(h, 0, 360) % 360;
    s = std::clamp(s, 0, 100);
    v = std::clamp(v, 0, 100);

    if (s == 0) {
        const auto gray = static_cast<uint8_t>(v * 255 / 100);
        r = g = b = gray;
        return;
    }

    const int c = v * s / 100;  // chroma, 0-100
    const int hm = h % 120;
    const int x = c * (60 - std::abs(hm - 60)) / 60;
    const int m = v - c;

    int r1 = 0, g1 = 0, b1 = 0;
    switch (h / 60) {
        case 0:  r1 = c; g1 = x; break;
        case 1:  r1 = x; g1 = c; break;
        case 2:  g1 = c; b1 = x; break;
        case 3:  g1 = x; b1 = c; break;
        case 4:  r1 = x; b1 = c; break;
        default: r1 = c; b1 = x; break;
    }
    r = static_cast<uint8_t>((r1 + m) * 255 / 100);
    g = static_cast<uint8_t>((g1 + m) * 255 / 100);
    b = static_cast<uint8_t>((b1 + m) * 255 / 100);
}

std::optional<SDL_Color> ColorDialog::parseHex(std::string_view text) {
    if (!text.empty() && text.front() == '#') text.remove_prefix(1);
    if (text.size() != 6) return std::nullopt;
    for (char c : text) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return std::nullopt;
    }
    unsigned value = 0;
    for (char c : text) {
        value = value * 16 + static_cast<unsigned>(std::isdigit(static_cast<unsigned char>(c))
            ? c - '0'
            : std::tolower(static_cast<unsigned char>(c)) - 'a' + 10);
    }
    return SDL_Color{static_cast<uint8_t>((value >> 16) & 0xFF),
                     static_cast<uint8_t>((value >> 8) & 0xFF),
                     static_cast<uint8_t>(value & 0xFF), 255};
}

void ColorDialog::layoutChildren() {
    constexpr int rowH = 26;
    constexpr int rowStep = 32;
    constexpr int firstRowY = 104;

    for (const auto& child : getChildren()) {
        const std::string_view id = child->getID();
        if (id == "cd_title") child->setPosition(10, 5);
        else if (id == "cd_presets_label") child->setPosition(10, 338);
    }

    const int previewW = (m_width - 30) / 2;
    if (m_oldPreview) {
        m_oldPreview->setPosition(10, 40);
        m_oldPreview->setSize(previewW, 48);
    }
    if (m_newPreview) {
        m_newPreview->setPosition(10 + previewW + 10, 40);
        m_newPreview->setSize(previewW, 48);
    }

    Slider* rows[6] = {m_rSlider, m_gSlider, m_bSlider, m_hSlider, m_sSlider, m_vSlider};
    for (size_t i = 0; i < m_rowTags.size() && i < 6; ++i) {
        m_rowTags[i]->setPosition(10, firstRowY + static_cast<int>(i) * rowStep + 5);
    }
    for (int i = 0; i < 6; ++i) {
        if (rows[i]) {
            rows[i]->setPosition(28, firstRowY + i * rowStep);
            rows[i]->setSize(m_width - 38, rowH);
        }
    }

    if (m_hexLabel) m_hexLabel->setPosition(10, 307);
    if (m_hexInput) {
        m_hexInput->setPosition(54, 302);
        m_hexInput->setSize(120, 28);
    }

    constexpr int cols = 6;
    constexpr int gap = 6;
    const int btnW = (m_width - 20 - (cols - 1) * gap) / cols;
    for (size_t i = 0; i < m_presets.size(); ++i) {
        const int col = static_cast<int>(i) % cols;
        const int row = static_cast<int>(i) / cols;
        m_presets[i]->setPosition(10 + col * (btnW + gap), 358 + row * (30 + gap));
        m_presets[i]->setSize(btnW, 30);
    }

    if (m_okBtn && m_cancelBtn) {
        const int buttonY = m_height - 46;
        const StackLayout strip(StackLayout::Direction::Horizontal, 10,
                                0, 0, 20, 0, StackLayout::Align::End);
        strip.arrangeStrip(std::vector<GUIElement*>{m_okBtn, m_cancelBtn}, m_width, buttonY);
    }
}

void ColorDialog::draw(SDL_Renderer* renderer) {
    drawTitleBar(renderer, m_x, m_y, m_width, m_titleBarHeight);
    Panel::draw(renderer);
}

ComponentType ColorDialog::getComponentTypeId() const {
    return ComponentType::ColorDialog;
}
