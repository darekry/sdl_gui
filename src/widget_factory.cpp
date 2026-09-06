#include "widget_factory.hpp"

#include "animated_image.hpp"
#include "arc_container.hpp"
#include "button.hpp"
#include "canvas.hpp"
#include "checkbox.hpp"
#include "combobox.hpp"
#include "constants.hpp"
#include "gui_manager.hpp"
#include "label.hpp"
#include "list_view.hpp"
#include "panel.hpp"
#include "progress_bar.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "range_slider.hpp"
#include "scroll_area.hpp"
#include "slider.hpp"
#include "string_grid.hpp"
#include "tab_control.hpp"
#include "text_area.hpp"
#include "text_input.hpp"

#include "std.hpp"

namespace {

std::pair<int, int> fallbackSize(ComponentType t) {
    switch (t) {
        case ComponentType::Button:        return {120, 40};
        case ComponentType::Label:         return {100, 25};
        case ComponentType::Checkbox:      return {150, 25};
        case ComponentType::RadioButton:   return {150, 25};
        case ComponentType::RadioGroup:    return {150, 100};
        case ComponentType::Slider:        return {150, 20};
        case ComponentType::RangeSlider:   return {150, 20};
        case ComponentType::TextInput:     return {150, 30};
        case ComponentType::TextArea:      return {200, 150};
        case ComponentType::TabControl:    return {300, 200};
        case ComponentType::Panel:         return {300, 200};
        case ComponentType::AnimatedImage: return {100, 100};
        case ComponentType::ComboBox:      return {150, 30};
        case ComponentType::Canvas:        return {200, 200};
        case ComponentType::StringGrid:    return {400, 300};
        case ComponentType::ListView:      return {200, 200};
        case ComponentType::ProgressBar:   return {200, 30};
        case ComponentType::ScrollArea:    return {300, 200};
        case ComponentType::ArcContainer:  return {200, 200};
        default:                           return {100, 50};
    }
}

bool constructible(ComponentType t) {
    switch (t) {
        case ComponentType::Panel:
        case ComponentType::Button:
        case ComponentType::Label:
        case ComponentType::Checkbox:
        case ComponentType::RadioButton:
        case ComponentType::RadioGroup:
        case ComponentType::Slider:
        case ComponentType::RangeSlider:
        case ComponentType::StringGrid:
        case ComponentType::ListView:
        case ComponentType::TextInput:
        case ComponentType::TextArea:
        case ComponentType::ComboBox:
        case ComponentType::TabControl:
        case ComponentType::AnimatedImage:
        case ComponentType::Canvas:
        case ComponentType::ProgressBar:
        case ComponentType::ScrollArea:
        case ComponentType::ArcContainer:
            return true;
        default:
            return false;
    }
}

Orientation toOrientation(bool vertical) {
    return vertical ? Orientation::Vertical : Orientation::Horizontal;
}

}  // namespace

std::pair<int, int> WidgetFactory::defaultSize(std::string_view type) {
    return fallbackSize(componentTypeFromString(type));
}

bool WidgetFactory::isKnownType(std::string_view type) {
    return constructible(componentTypeFromString(type));
}

std::vector<std::string_view> WidgetFactory::knownTypes() {
    return {"Panel", "Button",     "Label",      "Checkbox",    "RadioButton",
            "RadioGroup", "Slider", "RangeSlider", "StringGrid", "ListView",
            "TextInput",  "TextArea", "ComboBox",  "TabControl", "AnimatedImage",
            "Canvas",     "ProgressBar", "ScrollArea", "ArcContainer"};
}

std::unique_ptr<GUIElement> WidgetFactory::createBare(GUIManager& manager,
                                                      std::string_view type,
                                                      int x, int y, int w, int h) {
    return createBare(manager, componentTypeFromString(type), x, y, w, h);
}

std::unique_ptr<GUIElement> WidgetFactory::createBare(GUIManager& manager,
                                                      ComponentType type,
                                                      int x, int y, int w, int h) {
    WidgetProps props;
    props.x = x;
    props.y = y;
    props.w = w;
    props.h = h;
    // Bare = defaults only. Widget-specific fallbacks (e.g. Slider 100x20
    // when w/h are 0) live in create() so both paths share them.
    switch (type) {
        case ComponentType::Label:
            return std::make_unique<Label>(manager, x, y, "", -1);
        case ComponentType::TextArea:
            return std::make_unique<TextArea>(manager, x, y, w, h,
                                              constants::kDefaultFontPath, 16);
        case ComponentType::TabControl:
            return std::make_unique<TabControl>(manager, x, y, w, h, 30);
        default:
            break;
    }
    return create(manager, type, props);
}

std::unique_ptr<GUIElement> WidgetFactory::create(GUIManager& manager,
                                                  std::string_view type,
                                                  const WidgetProps& props) {
    return create(manager, componentTypeFromString(type), props);
}

std::unique_ptr<GUIElement> WidgetFactory::create(GUIManager& manager,
                                                  ComponentType type,
                                                  const WidgetProps& p) {
    const int x = p.x, y = p.y;
    switch (type) {
        case ComponentType::Panel: {
            auto panel = std::make_unique<Panel>(manager, x, y, p.w, p.h);
            panel->setDraggable(p.draggable);
            return panel;
        }
        case ComponentType::Button: {
            return std::make_unique<Button>(manager, x, y, p.w, p.h, p.text);
        }
        case ComponentType::Label: {
            return std::make_unique<Label>(manager, x, y, p.text, p.fontSize);
        }
        case ComponentType::Checkbox: {
            auto c = std::make_unique<Checkbox>(manager, x, y, p.w, p.h);
            c->setChecked(p.checked);
            return c;
        }
        case ComponentType::RadioButton: {
            auto rb = std::make_unique<RadioButton>(manager, x, y, p.w, p.h);
            rb->setSelected(p.selected);
            return rb;
        }
        case ComponentType::RadioGroup: {
            auto rg = std::make_unique<RadioGroup>(manager, x, y, p.w, p.h);
            if (p.hasOptionSpacing) rg->setOptionSpacing(p.optionSpacing);
            if (p.hasOptionMargins) rg->setOptionMargins(p.buttonX, p.labelX, p.startY);
            if (p.hasOptionSizes) rg->setOptionSizes(p.buttonSize, p.labelFontSize);
            for (const auto& opt : p.options) {
                if (!opt.text.empty()) rg->addOption(opt.text, opt.selected);
            }
            return rg;
        }
        case ComponentType::Slider: {
            auto s = std::make_unique<Slider>(manager, x, y,
                                              p.w > 0 ? p.w : 100, p.h > 0 ? p.h : 20,
                                              p.minVal, p.maxVal, p.value,
                                              toOrientation(p.vertical));
            s->setWheelStep(p.wheelStep);
            return s;
        }
        case ComponentType::RangeSlider: {
            auto rs = std::make_unique<RangeSlider>(manager, x, y,
                                                    p.w > 0 ? p.w : 100, p.h > 0 ? p.h : 20,
                                                    p.minVal, p.maxVal, p.lowerVal, p.upperVal,
                                                    toOrientation(p.vertical));
            rs->setWheelStep(p.wheelStep);
            return rs;
        }
        case ComponentType::StringGrid: {
            auto g = std::make_unique<StringGrid>(manager, x, y,
                                                  p.w > 0 ? p.w : 400, p.h > 0 ? p.h : 300,
                                                  p.rowCount, p.colCount);
            g->setShowRowHeaders(p.showRowHeaders);
            g->setShowColumnHeaders(p.showColumnHeaders);
            g->setEditable(p.editable);
            if (p.rowHeight >= 0) g->setRowHeight(p.rowHeight);
            if (p.headerHeight >= 0) g->setHeaderHeight(p.headerHeight);
            if (p.rowHeaderWidth >= 0) g->setRowHeaderWidth(p.rowHeaderWidth);
            if (p.hasHScroll) g->setHorizontalScrollEnabled(p.hScrollEnabled);
            if (p.hasVScroll) g->setVerticalScrollEnabled(p.vScrollEnabled);
            return g;
        }
        case ComponentType::ListView: {
            auto lv = std::make_unique<ListView>(manager, x, y,
                                                 p.w > 0 ? p.w : 200, p.h > 0 ? p.h : 200);
            if (p.rowHeight >= 0) lv->setRowHeight(p.rowHeight);
            for (const auto& item : p.items) {
                if (!item.empty()) lv->addItem(item);
            }
            if (p.hasSelectedIndex && p.selectedIndex >= 0) {
                lv->setSelectedRow(static_cast<size_t>(p.selectedIndex));
            }
            return lv;
        }
        case ComponentType::TextInput: {
            auto ti = std::make_unique<TextInput>(manager, x, y,
                                                  p.w > 0 ? p.w : 100, p.h > 0 ? p.h : 30);
            if (!p.text.empty()) ti->setText(std::string_view(p.text));
            ti->setLocked(p.locked);
            return ti;
        }
        case ComponentType::TextArea: {
            const std::string& fontPath =
                p.fontPath.empty() ? constants::kDefaultFontPath : p.fontPath;
            const int fontSize = p.fontSize >= 0 ? p.fontSize : 16;
            auto ta = std::make_unique<TextArea>(manager, x, y,
                                                 p.w > 0 ? p.w : 200, p.h > 0 ? p.h : 150,
                                                 fontPath, fontSize);
            if (!p.text.empty()) ta->setText(std::string_view(p.text));
            ta->setWordWrap(p.wordWrap);
            ta->setLocked(p.locked);
            return ta;
        }
        case ComponentType::ComboBox: {
            auto cb = std::make_unique<ComboBox>(manager, x, y,
                                                 p.w > 0 ? p.w : 150, p.h > 0 ? p.h : 30);
            for (const auto& item : p.items) cb->addItem(item);
            if (p.hasSelectedIndex) cb->setSelectedIndex(p.selectedIndex);
            return cb;
        }
        case ComponentType::TabControl: {
            auto tc = std::make_unique<TabControl>(manager, x, y,
                                                   p.w > 0 ? p.w : 200, p.h > 0 ? p.h : 200,
                                                   p.tabHeight);
            for (const auto& tab : p.tabs) {
                tc->addTab(tab.title, tab.width, tab.height);
            }
            return tc;
        }
        case ComponentType::AnimatedImage: {
            auto ai = std::make_unique<AnimatedImage>(manager, x, y,
                                                      p.w > 0 ? p.w : 100, p.h > 0 ? p.h : 100);
            if (!p.path.empty()) {
                ai->setSpriteSheet(p.path, p.frames, p.rows, p.frameW, p.frameH);
            }
            if (p.frameDuration >= 0.0f) {
                ai->setFrameDuration(p.frameDuration);
            } else {
                ai->setFPS(p.fps);
            }
            ai->setLoop(p.loop);
            ai->setUseCache(p.useCache);
            ai->setPreserveAspect(p.preserveAspect);
            if (p.scaleMode == "Center") {
                ai->setScaleMode(AnimatedImage::ScaleMode::Center);
            } else if (p.scaleMode == "None") {
                ai->setScaleMode(AnimatedImage::ScaleMode::None);
            } else {
                ai->setScaleMode(AnimatedImage::ScaleMode::Fit);
            }
            if (p.autoplay) ai->play();
            return ai;
        }
        case ComponentType::Canvas: {
            return std::make_unique<Canvas>(manager, x, y,
                                            p.w > 0 ? p.w : 100, p.h > 0 ? p.h : 100);
        }
        case ComponentType::ProgressBar: {
            auto pb = std::make_unique<ProgressBar>(manager, x, y,
                                                    p.w > 0 ? p.w : 200, p.h > 0 ? p.h : 30);
            pb->setRange(p.minF, p.maxF);
            pb->setValue(p.valueF);
            pb->setOrientation(toOrientation(p.vertical));
            pb->setShowText(p.showText);
            if (p.hasTextFormat) pb->setTextFormat(p.textFormat);
            return pb;
        }
        case ComponentType::ScrollArea: {
            auto sa = std::make_unique<ScrollArea>(manager, x, y,
                                                   p.w > 0 ? p.w : 300, p.h > 0 ? p.h : 200);
            if (p.contentWidth >= 0 || p.contentHeight >= 0) {
                sa->setContentSize(p.contentWidth >= 0 ? p.contentWidth : sa->getWidth(),
                                   p.contentHeight >= 0 ? p.contentHeight : sa->getHeight());
            }
            if (p.hasVScroll) sa->setVerticalScroll(p.vScrollEnabled);
            if (p.hasHScroll) sa->setHorizontalScroll(p.hScrollEnabled);
            return sa;
        }
        case ComponentType::ArcContainer: {
            return std::make_unique<ArcContainer>(manager, x, y, p.radius,
                                                  p.startAngle, p.endAngle);
        }
        default:
            return nullptr;
    }
}
