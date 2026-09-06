#pragma once

// WidgetFactory — single widget registry (point 5).
//
// Replaces the three copies of `if type=="Button" ...` dispatch that used to
// live in layout_parser.cpp, editor/preview_window.cpp and editor_state.cpp
// (plus ad-hoc make_unique chains in C-API examples). Adding a widget now
// means ONE entry in the factory table: type name <-> ComponentType,
// default size, bare construction, scalar-props construction.
//
// Two creation modes:
//   - createBare(type, mgr, x,y,w,h): default-constructed widget, for hosts
//     that apply properties themselves (C-API generic create, tests).
//   - create(type, mgr, props): fully configured widget from a WidgetProps
//     bag. Both LayoutParser and PreviewWindow build the bag from their own
//     source (layout node vs EditorElement) and share ALL construction code.
//   - defaultSize(type): canonical default rect part, shared with EditorState.

#include "component_type.hpp"

#include "std.hpp"

class GUIManager;
class GUIElement;

struct WidgetTabSpec {
    std::string title;
    int width = 100;
    int height = -1;
};

struct WidgetOptionSpec {
    std::string text;
    bool selected = false;
};

struct WidgetProps {
    int x = 0, y = 0, w = 0, h = 0;

    // Scalars (only the owning widget type reads its own fields).
    std::string text;
    std::string fontPath;
    int fontSize = -1;
    bool checked = false;
    bool selected = false;
    bool locked = false;
    bool wordWrap = true;
    bool draggable = false;
    bool vertical = false;

    int minVal = 0, maxVal = 100, value = 0;
    int lowerVal = 0, upperVal = 100;
    int wheelStep = 1;

    float minF = 0.0f, maxF = 100.0f, valueF = 0.0f;
    bool showText = true;
    std::string textFormat;
    bool hasTextFormat = false;

    int selectedIndex = -1;
    bool hasSelectedIndex = false;

    size_t rowCount = 5, colCount = 5;
    bool showRowHeaders = true, showColumnHeaders = true, editable = true;
    int rowHeight = -1, headerHeight = -1, rowHeaderWidth = -1;  // <0 = keep widget default
    bool hasHScroll = false, hScrollEnabled = true;
    bool hasVScroll = false, vScrollEnabled = true;

    int tabHeight = 30;

    std::string path;
    int frames = 1, rows = 1, frameW = 0, frameH = 0;
    float fps = 12.0f, frameDuration = -1.0f;  // frameDuration >= 0 wins over fps
    bool loop = true, useCache = true, preserveAspect = true, autoplay = true;
    std::string scaleMode = "Fit";

    int contentWidth = -1, contentHeight = -1;  // <0 = keep widget default

    int radius = 100;
    float startAngle = 0.0f, endAngle = 360.0f;

    int optionSpacing = 40;
    int buttonX = 20, labelX = 45, startY = 20;
    int buttonSize = 20, labelFontSize = 16;
    bool hasOptionSpacing = false, hasOptionMargins = false, hasOptionSizes = false;

    std::vector<std::string> items;
    std::vector<WidgetTabSpec> tabs;
    std::vector<WidgetOptionSpec> options;
};

class WidgetFactory {
public:
    // Canonical default size for a type name (editor palette, parser
    // fallbacks). Unknown type -> {100, 50}.
    static std::pair<int, int> defaultSize(std::string_view type);

    // True for every constructible widget type name.
    static bool isKnownType(std::string_view type);

    // All known type names (for palettes, validation, tests).
    static std::vector<std::string_view> knownTypes();

    // Bare widget with defaults (no scalar props applied).
    static std::unique_ptr<GUIElement> createBare(GUIManager& manager,
                                                  std::string_view type,
                                                  int x, int y, int w, int h);
    static std::unique_ptr<GUIElement> createBare(GUIManager& manager,
                                                  ComponentType type,
                                                  int x, int y, int w, int h);

    // Fully configured widget. Unknown/unsupported type -> nullptr
    // (caller decides: parser warns, preview falls back to Panel).
    static std::unique_ptr<GUIElement> create(GUIManager& manager,
                                              std::string_view type,
                                              const WidgetProps& props);
    static std::unique_ptr<GUIElement> create(GUIManager& manager,
                                              ComponentType type,
                                              const WidgetProps& props);
};
