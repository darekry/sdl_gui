#pragma once

#include "../panel.hpp"

#include "std.hpp"

/**
 * @file modal_dialog.hpp
 * @brief ModalDialog - thin modal base for DialogBox/FileDialog (parity #8).
 *
 * Extracts the duplicated modal boilerplate (close/isOpen/isOverlay + Esc):
 * - modal overlay (`isOverlay() == true`, Tab/OverlayStack handles the rest —
 *   no focus-trap here, GUIManager already provides it);
 * - key path: children first (Panel::handleEvent), then Esc → onCancel,
 *   Enter/KP_ENTER → onConfirm;
 * - `centerInViewport()` helper (viewport from the manager, no 800x600 hardcode).
 *
 * Stays abstract: no getComponentTypeId() override — subclasses keep theirs,
 * so no new ComponentType entry is needed.
 */

class ModalDialog : public Panel {
public:
    ModalDialog(GUIManager& manager, int x, int y, int width, int height);
    ~ModalDialog() override = default;

    bool isOverlay() const override { return true; }

    /// Closes the dialog (removes the element on next cleanup()).
    void close();
    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    /// Centers the dialog in the current viewport (manager window size).
    void centerInViewport();

    // Modal dialogs paint in the overlay pass (GUIManager skips isOverlay()
    // elements in the normal pass). Base GUIElement::renderOverlay() is a
    // deliberate no-op since the focus-outline fix — without this override
    // dialogs would hit-test but stay invisible.
    void renderOverlay(SDL_Renderer* renderer) override;

protected:
    bool handleEvent(const SDL_Event& e) override;
    virtual void onConfirm() {}
    virtual void onCancel() { close(); }

    bool m_isOpen = true;
};
