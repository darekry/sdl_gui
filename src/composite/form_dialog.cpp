#include "form_dialog.hpp"

#include "../button.hpp"
#include "../constants.hpp"
#include "../gui_manager.hpp"
#include "../label.hpp"
#include "../sdl_rect_helpers.hpp"
#include "../text_input.hpp"

#include "std.hpp"

FormDialog::FormDialog(GUIManager& manager, int x, int y, int width, int height,
                       std::string_view title)
    : ModalDialog(manager, x, y, width, height), m_title(title) {
    m_titleBarHeight = m_title.empty() ? 0 : 30;
}

void FormDialog::setTitle(std::string_view title) {
    m_title = title;
    m_titleBarHeight = m_title.empty() ? 0 : 30;
    markDirty();
}

GUIElement* FormDialog::addWidget(int id, std::unique_ptr<GUIElement> child) {
    if (!child) {
        return nullptr;
    }
    GUIElement* raw = child.get();
    if (raw->getParent() != nullptr) {
        return nullptr;
    }

    // The element may still be a manager-owned top-level: detach it first so
    // ownership moves into this dialog without double ownership.
    if (auto detached = m_manager.detachElement(raw)) {
        child = std::move(detached);
    }

    raw->setControlId(id);
    addChild(std::move(child));
    m_items.push_back(Item{id, raw});
    return raw;
}

void FormDialog::placeNewWidget(GUIElement* child, int x, int y, int w, int h) {
    if (!child) return;
    child->setPosition(x, y);
    if (w > 0 || h > 0) {
        child->setSize(w > 0 ? w : child->getWidth(),
                       h > 0 ? h : child->getHeight());
    }
}

Label* FormDialog::addStatic(int id, std::string_view text,
                             int x, int y, int w, int h) {
    auto label = std::make_unique<Label>(m_manager, x, y, text, -1);
    Label* raw = label.get();
    if (!addWidget(id, std::move(label))) {
        return nullptr;
    }
    placeNewWidget(raw, x, y, w, h);
    return raw;
}

Button* FormDialog::addButton(int id, std::string_view text,
                              int x, int y, int w, int h) {
    auto button = std::make_unique<Button>(m_manager, x, y, w, h, text);
    Button* raw = button.get();
    if (!addWidget(id, std::move(button))) {
        return nullptr;
    }
    raw->setOnClickCallback([this, id](GUIElement*) { on_command(id); });
    placeNewWidget(raw, x, y, w, h);
    return raw;
}

TextInput* FormDialog::addEdit(int id, std::string_view text,
                               int x, int y, int w, int h) {
    auto edit = std::make_unique<TextInput>(m_manager, x, y, w, h);
    TextInput* raw = edit.get();
    if (!addWidget(id, std::move(edit))) {
        return nullptr;
    }
    raw->setText(text);
    placeNewWidget(raw, x, y, w, h);
    return raw;
}

CustomControl* FormDialog::addCustom(int id, int x, int y, int w, int h) {
    auto control = std::make_unique<CustomControl>(m_manager, x, y, w, h);
    CustomControl* raw = control.get();
    if (!addWidget(id, std::move(control))) {
        return nullptr;
    }
    placeNewWidget(raw, x, y, w, h);
    return raw;
}

FormDialog::Item* FormDialog::findItemData(int id) {
    for (Item& item : m_items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

const FormDialog::Item* FormDialog::findItemData(int id) const {
    for (const Item& item : m_items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

GUIElement* FormDialog::findItem(int id) const {
    const Item* item = findItemData(id);
    return item ? item->child : nullptr;
}

SDL_Rect FormDialog::getItemRect(int id) const {
    const Item* item = findItemData(id);
    if (!item || !item->child) {
        return SDL_Rect{0, 0, 0, 0};
    }
    return SDL_Rect{item->child->getX(), item->child->getY(),
                    item->child->getWidth(), item->child->getHeight()};
}

bool FormDialog::setItemVisible(int id, bool visible) {
    GUIElement* child = findItem(id);
    if (!child) return false;
    child->setVisible(visible);
    return true;
}

bool FormDialog::setItemEnabled(int id, bool enabled) {
    GUIElement* child = findItem(id);
    if (!child) return false;
    child->setEnabled(enabled);
    return true;
}

bool FormDialog::setItemText(int id, std::string_view text) {
    Item* item = findItemData(id);
    if (!item || !item->child) return false;

    GUIElement* child = item->child;
    const SDL_Rect rect = getItemRect(id);

    bool handled = false;
    if (auto* label = dynamic_cast<Label*>(child)) {
        label->setText(text);
        handled = true;
    } else if (auto* button = dynamic_cast<Button*>(child)) {
        button->setText(text);
        handled = true;
    } else if (auto* editable = dynamic_cast<TextEditable*>(child)) {
        editable->setText(text);
        handled = true;
    }
    if (!handled) {
        return false;
    }

    // Label::setText re-runs the auto-size pass; keep the form-assigned rect.
    child->setPosition(rect.x, rect.y);
    child->setSize(rect.w, rect.h);
    return true;
}

bool FormDialog::setItemRect(int id, int x, int y, int w, int h) {
    GUIElement* child = findItem(id);
    if (!child) return false;
    child->setPosition(x, y);
    child->setSize(w, h);
    return true;
}

void FormDialog::end(int result) {
    closeWithResult(result);
}

void FormDialog::setOnCommand(std::function<void(int)> callback) {
    m_onCommand = std::move(callback);
}

void FormDialog::on_command(int id) {
    if (m_onCommand) {
        m_onCommand(id);
    }
}

ComponentType FormDialog::getComponentTypeId() const {
    return ComponentType::FormDialog;
}

void FormDialog::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    if (m_titleBarHeight > 0) {
        drawTitleBar(renderer, 0, 0, m_width, m_titleBarHeight);
        const Style normal = getComposedStyle(ElementState::Normal);
        const SDL_Color textColor = normal.textColor.value_or(constants::kDefaultTextColor);
        const int fontSize = normal.fontSize.value_or(
            m_manager.getTheme().getDefaultStyle().fontSize.value_or(16));
        const std::string fontPath = normal.fontName.value_or(constants::kDefaultFontPath);
        auto font = m_manager.getFontManager().loadFont(fontPath, fontSize);
        if (font && !m_title.empty()) {
            auto texture = m_manager.getTextureManager().createTextureFromText(m_title, font, textColor);
            if (texture) {
                SDL_Rect dst{8, (m_titleBarHeight - TextureHeight(texture.get())) / 2,
                             TextureWidth(texture.get()), TextureHeight(texture.get())};
                RenderTexture(renderer, texture.get(), dst);
            }
        }
    }
}
