#pragma once

// ComponentType: interned widget type ID — the ONLY type key inside the lib.
// String names ("Button", ...) exist solely at the outer boundary:
// layout files (componentTypeFromString) and the C-API
// (componentTypeToString).
//
// The global render cache (TextureManager::m_renderCache) is keyed by an
// integer hash that includes this ID — so N identical widgets (e.g. 100 unit
// cards in an RTS game) share ONE cache entry / ONE texture.

#include "std.hpp"

enum class ComponentType : uint8_t {
    Unknown = 0,
    GUIElement,
    Panel,
    Button,
    Label,
    Checkbox,
    RadioButton,
    RadioGroup,
    Slider,
    RangeSlider,
    StringGrid,
    ListView,
    TextInput,
    TextArea,
    ComboBox,
    TabControl,
    AnimatedImage,
    Canvas,
    ContextMenu,
    Cursor,
    ArcContainer,
    ProgressBar,
    ScrollArea,
    ShaderPanel,
    DialogBox,
    FileDialog,
    Count
};

inline std::string_view componentTypeToString(ComponentType t) {
    switch (t) {
        case ComponentType::GUIElement:    return "GUIElement";
        case ComponentType::Panel:         return "Panel";
        case ComponentType::Button:        return "Button";
        case ComponentType::Label:         return "Label";
        case ComponentType::Checkbox:      return "Checkbox";
        case ComponentType::RadioButton:   return "RadioButton";
        case ComponentType::RadioGroup:    return "RadioGroup";
        case ComponentType::Slider:        return "Slider";
        case ComponentType::RangeSlider:   return "RangeSlider";
        case ComponentType::StringGrid:    return "StringGrid";
        case ComponentType::ListView:      return "ListView";
        case ComponentType::TextInput:     return "TextInput";
        case ComponentType::TextArea:      return "TextArea";
        case ComponentType::ComboBox:      return "ComboBox";
        case ComponentType::TabControl:    return "TabControl";
        case ComponentType::AnimatedImage: return "AnimatedImage";
        case ComponentType::Canvas:        return "Canvas";
        case ComponentType::ContextMenu:   return "ContextMenu";
        case ComponentType::Cursor:        return "Cursor";
        case ComponentType::ArcContainer:  return "ArcContainer";
        case ComponentType::ProgressBar:   return "ProgressBar";
        case ComponentType::ScrollArea:    return "ScrollArea";
        case ComponentType::ShaderPanel:   return "ShaderPanel";
        case ComponentType::DialogBox:     return "DialogBox";
        case ComponentType::FileDialog:    return "FileDialog";
        default:                           return "GUIElement";
    }
}

inline ComponentType componentTypeFromString(std::string_view s) {
    if (s == "Panel")         return ComponentType::Panel;
    if (s == "Button")        return ComponentType::Button;
    if (s == "Label")         return ComponentType::Label;
    if (s == "Checkbox")      return ComponentType::Checkbox;
    if (s == "RadioButton")   return ComponentType::RadioButton;
    if (s == "RadioGroup")    return ComponentType::RadioGroup;
    if (s == "Slider")        return ComponentType::Slider;
    if (s == "RangeSlider")   return ComponentType::RangeSlider;
    if (s == "StringGrid")    return ComponentType::StringGrid;
    if (s == "ListView")      return ComponentType::ListView;
    if (s == "TextInput")     return ComponentType::TextInput;
    if (s == "TextArea")      return ComponentType::TextArea;
    if (s == "ComboBox")      return ComponentType::ComboBox;
    if (s == "TabControl")    return ComponentType::TabControl;
    if (s == "AnimatedImage") return ComponentType::AnimatedImage;
    if (s == "Canvas")        return ComponentType::Canvas;
    if (s == "ContextMenu")   return ComponentType::ContextMenu;
    if (s == "Cursor")        return ComponentType::Cursor;
    if (s == "ArcContainer")  return ComponentType::ArcContainer;
    if (s == "ProgressBar")   return ComponentType::ProgressBar;
    if (s == "ScrollArea")    return ComponentType::ScrollArea;
    if (s == "ShaderPanel")   return ComponentType::ShaderPanel;
    if (s == "DialogBox")     return ComponentType::DialogBox;
    if (s == "FileDialog")    return ComponentType::FileDialog;
    if (s == "GUIElement")    return ComponentType::GUIElement;
    return ComponentType::Unknown;
}
