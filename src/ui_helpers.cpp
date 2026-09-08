#include "ui_helpers.hpp"
#include "gui_manager.hpp"

namespace {

std::vector<std::vector<Panel*>> gridPanelsImpl(GUIManager& m, GUIElement* parent,
                                                int cols, int rows, int cellW, int cellH,
                                                int gap, int ox, int oy) {
    std::vector<std::vector<Panel*>> grid(static_cast<size_t>(rows),
                                          std::vector<Panel*>(static_cast<size_t>(cols), nullptr));
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int cx = ox + x * (cellW + gap);
            int cy = oy + y * (cellH + gap);
            auto owned = std::make_unique<Panel>(m, cx, cy, cellW, cellH);
            Panel* cell = owned.get();
            if (parent) {
                parent->addChild(std::move(owned));
            } else {
                m.addElement(std::move(owned));
            }
            grid[static_cast<size_t>(y)][static_cast<size_t>(x)] = cell;
        }
    }
    return grid;
}

std::vector<std::vector<FaceCell>> faceGridImpl(GUIManager& m, GUIElement* parent,
                                                int cols, int rows, int cellW, int cellH,
                                                int gap, int ox, int oy, int faceSize) {
    std::vector<std::vector<FaceCell>> grid(static_cast<size_t>(rows),
                                            std::vector<FaceCell>(static_cast<size_t>(cols)));
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int cx = ox + x * (cellW + gap);
            int cy = oy + y * (cellH + gap);
            auto btnOwned = std::make_unique<Button>(m, cx, cy, cellW, cellH, "");
            Button* btn = btnOwned.get();
            if (parent) {
                parent->addChild(std::move(btnOwned));
            } else {
                m.addElement(std::move(btnOwned));
            }
            auto faceOwned = std::make_unique<Label>(m, 0, 0, "", faceSize);
            Label* face = faceOwned.get();
            // Center for the (empty) initial text; single-char captions
            // stay ~centered. Re-center manually for wide dynamic text.
            int fw = 0, fh = 0;
            face->getSize(fw, fh);
            face->setPosition((cellW - fw) / 2, (cellH - fh) / 2);
            btn->addChild(std::move(faceOwned));
            grid[static_cast<size_t>(y)][static_cast<size_t>(x)] = {btn, face};
        }
    }
    return grid;
}

} // namespace

std::vector<std::vector<Panel*>> gridPanels(GUIElement& parent, int cols, int rows,
                                            int cellW, int cellH, int gap, int ox, int oy) {
    return gridPanelsImpl(parent.getManager(), &parent, cols, rows, cellW, cellH, gap, ox, oy);
}

std::vector<std::vector<Panel*>> gridPanels(GUIManager& manager, int cols, int rows,
                                            int cellW, int cellH, int gap, int ox, int oy) {
    return gridPanelsImpl(manager, nullptr, cols, rows, cellW, cellH, gap, ox, oy);
}

std::vector<std::vector<FaceCell>> faceGrid(GUIElement& parent, int cols, int rows,
                                            int cellW, int cellH, int gap, int ox, int oy,
                                            int faceSize) {
    return faceGridImpl(parent.getManager(), &parent, cols, rows, cellW, cellH, gap, ox, oy,
                        faceSize);
}

std::vector<std::vector<FaceCell>> faceGrid(GUIManager& manager, int cols, int rows,
                                            int cellW, int cellH, int gap, int ox, int oy,
                                            int faceSize) {
    return faceGridImpl(manager, nullptr, cols, rows, cellW, cellH, gap, ox, oy, faceSize);
}

std::vector<int> shuffledPairs(int pairs) {
    std::vector<int> deck;
    deck.reserve(static_cast<size_t>(pairs) * 2);
    for (int s = 0; s < pairs; ++s) {
        deck.push_back(s);
        deck.push_back(s);
    }
    std::shuffle(deck.begin(), deck.end(), std::mt19937{std::random_device{}()});
    return deck;
}

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

void styleCard(Panel& panel, SDL_Color bg, SDL_Color border, int borderWidth, int radius) {
    Style s;
    s.backgroundColor = bg;
    s.borderColor = border;
    s.borderWidth = borderWidth;
    s.borderRadius = radius;
    panel.setStyle(ElementState::Normal, s);
}

static Panel* addDarkPanelImpl(GUIManager& manager, GUIElement* parent,
                               int x, int y, int w, int h) {
    auto owned = std::make_unique<Panel>(manager, x, y, w, h);
    Panel* raw = owned.get();
    styleCard(*raw);
    if (parent) {
        parent->addChild(std::move(owned));
    } else {
        manager.addElement(std::move(owned));
    }
    return raw;
}

Panel* addDarkPanel(GUIElement& parent, int x, int y, int w, int h) {
    return addDarkPanelImpl(parent.getManager(), &parent, x, y, w, h);
}

Panel* addDarkPanel(GUIManager& manager, int x, int y, int w, int h) {
    return addDarkPanelImpl(manager, nullptr, x, y, w, h);
}

static Button* addButtonImpl(GUIManager& manager, GUIElement* parent,
                      int x, int y, int w, int h,
                      const std::string& text, Button::OnClickCallback onClick) {
    auto owned = std::make_unique<Button>(manager, x, y, w, h, text);
    Button* raw = owned.get();
    if (onClick) raw->setOnClickCallback(std::move(onClick));
    if (parent) {
        parent->addChild(std::move(owned));
    } else {
        manager.addElement(std::move(owned));
    }
    return raw;
}

Button* addButton(GUIElement& parent, int x, int y, int w, int h,
                  std::string text, Button::OnClickCallback onClick) {
    return addButtonImpl(parent.getManager(), &parent, x, y, w, h, text, std::move(onClick));
}

Button* addButton(GUIManager& manager, int x, int y, int w, int h,
                  std::string text, Button::OnClickCallback onClick) {
    return addButtonImpl(manager, nullptr, x, y, w, h, text, std::move(onClick));
}

static Label* addLabelImpl(GUIManager& manager, GUIElement* parent,
                    int x, int y, const std::string& text,
                    int fontSize, std::optional<SDL_Color> textColor) {
    auto owned = std::make_unique<Label>(manager, x, y, text, fontSize);
    Label* raw = owned.get();
    if (textColor) {
        Style s;
        s.textColor = *textColor;
        raw->setStyle(ElementState::Normal, s);
    }
    if (parent) {
        parent->addChild(std::move(owned));
    } else {
        manager.addElement(std::move(owned));
    }
    return raw;
}

Label* addLabel(GUIElement& parent, int x, int y, std::string text,
                int fontSize, std::optional<SDL_Color> textColor) {
    return addLabelImpl(parent.getManager(), &parent, x, y, text, fontSize, textColor);
}

Label* addLabel(GUIManager& manager, int x, int y, std::string text,
                int fontSize, std::optional<SDL_Color> textColor) {
    return addLabelImpl(manager, nullptr, x, y, text, fontSize, textColor);
}

static LabeledCheckbox addLabeledCheckboxImpl(GUIManager& manager, GUIElement* parent,
                                       int x, int y, const std::string& text,
                                       int boxSize, int fontSize,
                                       std::optional<SDL_Color> textColor) {
    auto boxOwned = std::make_unique<Checkbox>(manager, x, y, boxSize, boxSize);
    Checkbox* box = boxOwned.get();
    // Caption right of the box, roughly vertically centered on it.
    int lx = x + boxSize + 8;
    int ly = y + (boxSize - fontSize) / 2;
    if (ly < y) ly = y;
    auto labelOwned = std::make_unique<Label>(manager, lx, ly, text, fontSize);
    Label* label = labelOwned.get();
    if (textColor) {
        Style s;
        s.textColor = *textColor;
        label->setStyle(ElementState::Normal, s);
    }
    if (parent) {
        parent->addChild(std::move(boxOwned));
        parent->addChild(std::move(labelOwned));
    } else {
        manager.addElement(std::move(boxOwned));
        manager.addElement(std::move(labelOwned));
    }
    return {box, label};
}

LabeledCheckbox addLabeledCheckbox(GUIElement& parent, int x, int y, std::string text,
                                   int boxSize, int fontSize,
                                   std::optional<SDL_Color> textColor) {
    return addLabeledCheckboxImpl(parent.getManager(), &parent, x, y, text, boxSize, fontSize,
                                  textColor);
}

LabeledCheckbox addLabeledCheckbox(GUIManager& manager, int x, int y, std::string text,
                                   int boxSize, int fontSize,
                                   std::optional<SDL_Color> textColor) {
    return addLabeledCheckboxImpl(manager, nullptr, x, y, text, boxSize, fontSize, textColor);
}

void onAnyChange(std::initializer_list<Slider*> sliders, std::function<void()> refresh) {
    for (Slider* s : sliders) {
        if (s) s->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
    }
    refresh();
}

void onAnyChange(std::initializer_list<RangeSlider*> sliders, std::function<void()> refresh) {
    for (RangeSlider* s : sliders) {
        if (s) s->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
    }
    refresh();
}

void bindSliderValue(Slider& slider, int& out) {
    out = slider.getValue();
    slider.setOnChangeCallback([&out](GUIElement* e) {
        auto* s = static_cast<Slider*>(e);
        if (s) out = s->getValue();
    });
}

void bindSliderValue(Slider& slider, float& out, float scale) {
    out = static_cast<float>(slider.getValue()) * scale;
    slider.setOnChangeCallback([&out, scale](GUIElement* e) {
        auto* s = static_cast<Slider*>(e);
        if (s) out = static_cast<float>(s->getValue()) * scale;
    });
}

static StatusBar makeTopBarImpl(GUIManager& manager, GUIElement* parent,
                         int x, int y, int w, int h,
                         const std::string& text, int fontSize) {
    auto barOwned = std::make_unique<Panel>(manager, x, y, w, h);
    Panel* bar = barOwned.get();
    auto labelOwned = std::make_unique<Label>(manager, 8, (h - fontSize) / 2, text, fontSize);
    Label* label = labelOwned.get();
    bar->addChild(std::move(labelOwned));
    if (parent) {
        parent->addChild(std::move(barOwned));
    } else {
        manager.addElement(std::move(barOwned));
    }
    return {bar, label};
}

StatusBar makeTopBar(GUIManager& manager, int screenW, int h,
                     std::string text, int fontSize) {
    return makeTopBarImpl(manager, nullptr, 0, 0, screenW, h, text, fontSize);
}

StatusBar makeBottomBar(GUIManager& manager, int screenW, int screenH, int h,
                        std::string text, int fontSize) {
    return makeTopBarImpl(manager, nullptr, 0, screenH - h, screenW, h, text, fontSize);
}

StatusBar makeTopBar(GUIElement& parent, int h, std::string text, int fontSize) {
    return makeTopBarImpl(parent.getManager(), &parent, 0, 0,
                          parent.getWidth(), h, text, fontSize);
}

StatusBar makeBottomBar(GUIElement& parent, int h, std::string text, int fontSize) {
    return makeTopBarImpl(parent.getManager(), &parent, 0, parent.getHeight() - h,
                          parent.getWidth(), h, text, fontSize);
}
