#pragma once

#include "modal_dialog.hpp"
#include "../custom_control.hpp"

#include "std.hpp"

class Label;
class Button;
class TextInput;

/**
 * @file form_dialog.hpp
 * @brief FormDialog - generic Win16-style dialog host (owner of item ids).
 *
 * A modal overlay owning a registry of {control id, child widget} pairs.
 * Children keep their explicitly assigned rects (default AnchorLayout leaves
 * anchor-less children alone). The title is drawn as a 30 px bar like
 * DialogBox; an empty title means no bar. Unlike DialogBox the dialog is NOT
 * draggable by default (CivLin rows must receive BUTTON_UP).
 *
 * Item ownership: the form owns reparented children (the registry does not
 * own anything). addWidget() only accepts top-level elements (parent == NULL)
 * and reparents them with GUIElement::addChild().
 */
class FormDialog : public ModalDialog {
public:
    FormDialog(GUIManager& manager, int x, int y, int width, int height,
               std::string_view title = "");

    void setTitle(std::string_view title);
    [[nodiscard]] std::string_view getTitle() const { return m_title; }

    // Reparents a top-level element and assigns its control id. Returns NULL
    // when the child already has a parent (not top-level) or is null.
    GUIElement* addWidget(int id, std::unique_ptr<GUIElement> child);

    Label*       addStatic(int id, std::string_view text,
                           int x, int y, int w, int h);
    Button*      addButton(int id, std::string_view text,
                           int x, int y, int w, int h);
    TextInput*   addEdit(int id, std::string_view text,
                         int x, int y, int w, int h);
    CustomControl* addCustom(int id, int x, int y, int w, int h);

    [[nodiscard]] GUIElement* findItem(int id) const;
    // Local rect (child coordinates are form-local for direct children).
    [[nodiscard]] SDL_Rect getItemRect(int id) const;

    bool setItemVisible(int id, bool visible);
    bool setItemEnabled(int id, bool enabled);
    // Re-applies the current rect after the widget's own text metrics would
    // otherwise auto-resize it (Label).
    bool setItemText(int id, std::string_view text);
    bool setItemRect(int id, int x, int y, int w, int h);

    // Close with a result code (getResult() keeps it; read before cleanup()).
    void end(int result);
    [[nodiscard]] bool isClosed() const { return !m_isOpen; }

    // Command notifications (buttons + GUIElement::notifyCommand).
    void setOnCommand(std::function<void(int)> callback);
    void on_command(int id);

    ComponentType getComponentTypeId() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

    struct Item {
        int id = -1;
        GUIElement* child = nullptr;
    };

private:
    [[nodiscard]] Item* findItemData(int id);
    [[nodiscard]] const Item* findItemData(int id) const;
    void placeNewWidget(GUIElement* child, int x, int y, int w, int h);

    std::vector<Item> m_items;
    std::string m_title;
    int m_titleBarHeight = 30;
    std::function<void(int)> m_onCommand;
};
