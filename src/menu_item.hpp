#pragma once

#include <SDL3/SDL_keycode.h>

#include "std.hpp"

// ---------------------------------------------------------------------------
// Menu text helpers (Win32 mnemonics + shortcut strings).
// Declared before MenuItem so its inline builder can use them.
// ---------------------------------------------------------------------------
namespace menu_text {

// Returns the text with Win32 '&' mnemonic markers removed. "&&" is a literal
// '&'; a trailing lone '&' is dropped.
inline std::string displayText(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '&') {
            out += text[i];
            continue;
        }
        if (i + 1 < text.size() && text[i + 1] == '&') {
            out += '&';
            ++i;
        }
        // Lone '&' marks the next char as the mnemonic and is not drawn.
    }
    return out;
}

// Mnemonic character (after the first lone '&'), or '\0'.
inline char mnemonic(std::string_view text) {
    for (size_t i = 0; i + 1 < text.size(); ++i) {
        if (text[i] != '&') continue;
        if (text[i + 1] == '&') {
            ++i; // literal ampersand, not a mnemonic
            continue;
        }
        return text[i + 1];
    }
    return '\0';
}

// Index of the mnemonic character inside displayText(), or npos.
inline size_t mnemonicDisplayIndex(std::string_view text) {
    size_t outIndex = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '&') {
            ++outIndex;
            continue;
        }
        if (i + 1 < text.size() && text[i + 1] == '&') {
            ++outIndex;
            ++i;
            continue;
        }
        // Lone '&': the next character is the mnemonic, emitted at outIndex.
        if (i + 1 < text.size()) return outIndex;
    }
    return std::string::npos;
}

inline std::string lowerAscii(std::string_view s) {
    std::string out(s);
    for (auto& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

inline std::string trimAscii(std::string_view s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return std::string(s.substr(start, end - start));
}

// Named keys accepted in shortcut strings (case-insensitive).
inline SDL_Keycode keyFromName(std::string_view name) {
    const std::string key = lowerAscii(name);
    if (key.size() == 1) {
        const char c = key[0];
        if (c >= 'a' && c <= 'z') return static_cast<SDL_Keycode>(c);
        if (c >= '0' && c <= '9') return static_cast<SDL_Keycode>(c);
    }
    if (key.size() >= 2 && key[0] == 'f') {
        int n = 0;
        bool valid = true;
        for (size_t i = 1; i < key.size(); ++i) {
            if (key[i] < '0' || key[i] > '9') {
                valid = false;
                break;
            }
            n = n * 10 + (key[i] - '0');
            // Stop before the accumulator can overflow (only F1-F12 exist).
            if (n > 12) {
                valid = false;
                break;
            }
        }
        if (valid && n >= 1) return static_cast<SDL_Keycode>(SDLK_F1 + static_cast<Uint32>(n - 1));
    }
    if (key == "escape" || key == "esc") return SDLK_ESCAPE;
    if (key == "return" || key == "enter") return SDLK_RETURN;
    if (key == "space") return SDLK_SPACE;
    if (key == "tab") return SDLK_TAB;
    if (key == "backspace") return SDLK_BACKSPACE;
    if (key == "delete" || key == "del") return SDLK_DELETE;
    if (key == "insert" || key == "ins") return SDLK_INSERT;
    if (key == "home") return SDLK_HOME;
    if (key == "end") return SDLK_END;
    if (key == "pageup" || key == "pgup") return SDLK_PAGEUP;
    if (key == "pagedown" || key == "pgdn") return SDLK_PAGEDOWN;
    if (key == "left") return SDLK_LEFT;
    if (key == "right") return SDLK_RIGHT;
    if (key == "up") return SDLK_UP;
    if (key == "down") return SDLK_DOWN;
    return SDLK_UNKNOWN;
}

// Parses "Ctrl+Shift+S", "Alt+F4", "F10", "Esc"... Unknown keys yield
// SDLK_UNKNOWN but the display string is kept as typed.
inline bool parseShortcutString(std::string_view text, SDL_Keycode& key, SDL_Keymod& mod) {
    key = SDLK_UNKNOWN;
    mod = SDL_KMOD_NONE;

    size_t start = 0;
    while (start <= text.size()) {
        size_t plus = text.find('+', start);
        const std::string_view token = (plus == std::string_view::npos)
            ? text.substr(start)
            : text.substr(start, plus - start);
        const std::string part = lowerAscii(trimAscii(token));
        if (!part.empty()) {
            if (part == "ctrl" || part == "control") mod = static_cast<SDL_Keymod>(mod | SDL_KMOD_CTRL);
            else if (part == "shift") mod = static_cast<SDL_Keymod>(mod | SDL_KMOD_SHIFT);
            else if (part == "alt") mod = static_cast<SDL_Keymod>(mod | SDL_KMOD_ALT);
            else key = keyFromName(part);
        }
        if (plus == std::string_view::npos) break;
        start = plus + 1;
    }

    return key != SDLK_UNKNOWN;
}

} // namespace menu_text

// ---------------------------------------------------------------------------
// MenuItem: node of the menu tree.
//
// submenu lives in unique_ptrs so node addresses stay stable while siblings
// are added — MenuPopup holds a pointer to a submenu vector and MenuBar maps
// "node with children" -> popup once, at setup time.
//
// Text supports Win32 mnemonics: "&File" renders "File" with 'F' underlined,
// "&&" renders a literal '&'. Shortcuts are display strings ("Ctrl+S")
// parsed once into (SDL_Keycode, SDL_Keymod), so MenuBar can dispatch them
// application-wide while no menu is open.
//
// Action resolution order on activation:
//   1. `action` callback (C++ apps),
//   2. `command` string passed to MenuBar::setCommandHandler (layout files /
//      command-id style apps).
// ---------------------------------------------------------------------------
enum class MenuCheck {
    None,
    Check,
    Radio
};

struct MenuItem {
    std::string text;
    std::string shortcut;
    std::string command;
    std::function<void()> action;

    bool enabled = true;
    bool separator = false;
    MenuCheck check = MenuCheck::None;
    bool checked = false;

    SDL_Keycode shortcutKey = SDLK_UNKNOWN;
    SDL_Keymod shortcutMod = SDL_KMOD_NONE;

    // Children. Empty for leaf items.
    std::vector<std::unique_ptr<MenuItem>> submenu;

    // MenuBar hook: fired as (parent, newChild) when a child is appended.
    // MenuBar uses it to create the dropdown popup for a node at the moment it
    // gains its first child (trees are usually built chained after addMenu).
    std::function<void(MenuItem&, MenuItem&)> onItemAdded;

    MenuItem() = default;
    explicit MenuItem(std::string_view label,
                      std::function<void()> actionFn = nullptr,
                      bool isEnabled = true)
        : text(label), action(std::move(actionFn)), enabled(isEnabled) {}

    static std::unique_ptr<MenuItem> makeSeparator() {
        auto item = std::make_unique<MenuItem>();
        item->separator = true;
        item->enabled = false;
        return item;
    }

    // === Builder API ===

    MenuItem& addItem(std::string_view label,
                      std::function<void()> actionFn = nullptr,
                      bool isEnabled = true) {
        auto child = std::make_unique<MenuItem>(label, std::move(actionFn), isEnabled);
        MenuItem& ref = *child;
        submenu.push_back(std::move(child));
        if (onItemAdded) onItemAdded(*this, ref);
        return ref;
    }

    // Same as addItem(), named for menus whose child owns nested items.
    MenuItem& addSubmenu(std::string_view label) {
        return addItem(label);
    }

    // Adds a separator and returns THIS node, so chains stay on the parent.
    MenuItem& addSeparator() {
        auto separatorNode = makeSeparator();
        MenuItem& ref = *separatorNode;
        submenu.push_back(std::move(separatorNode));
        if (onItemAdded) onItemAdded(*this, ref);
        return *this;
    }

    MenuItem& setShortcut(std::string_view shortcutText) {
        shortcut = shortcutText;
        shortcutKey = SDLK_UNKNOWN;
        shortcutMod = SDL_KMOD_NONE;
        menu_text::parseShortcutString(shortcut, shortcutKey, shortcutMod);
        return *this;
    }

    // Display-only shortcut: the text is drawn right-aligned like setShortcut,
    // but no global accelerator is registered. For apps that own the key
    // handling themselves (Win16-style accelerators handled by the game loop).
    MenuItem& setShortcutDisplay(std::string_view shortcutText) {
        shortcut = shortcutText;
        shortcutKey = SDLK_UNKNOWN;
        shortcutMod = SDL_KMOD_NONE;
        return *this;
    }

    MenuItem& setCommand(std::string_view commandId) {
        command = commandId;
        return *this;
    }

    MenuItem& setEnabled(bool value = true) {
        enabled = value;
        return *this;
    }

    MenuItem& setChecked(bool value = true) {
        check = MenuCheck::Check;
        checked = value;
        return *this;
    }

    MenuItem& setRadio(bool value = true) {
        check = MenuCheck::Radio;
        checked = value;
        return *this;
    }

    [[nodiscard]] bool hasSubmenu() const { return !submenu.empty(); }
    [[nodiscard]] bool isSelectable() const { return enabled && !separator; }
    [[nodiscard]] MenuItem* itemAt(size_t index) const {
        return index < submenu.size() ? submenu[index].get() : nullptr;
    }
    [[nodiscard]] size_t itemCount() const { return submenu.size(); }

    // Text without mnemonic markers ("&File" -> "File").
    [[nodiscard]] std::string displayText() const { return menu_text::displayText(text); }
};
