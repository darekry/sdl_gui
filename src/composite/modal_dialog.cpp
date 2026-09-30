#include "modal_dialog.hpp"
#include "../gui_manager.hpp"
#include "../sdl_rect_helpers.hpp"

#include "std.hpp"

ModalDialog::ModalDialog(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height) {}

void ModalDialog::close() {
    m_isOpen = false;
    markForDeletion();
}

void ModalDialog::centerInViewport() {
    int screenW = 0, screenH = 0;
    m_manager.getWindowSize(screenW, screenH);
    auto [x, y] = CenterRect(screenW, screenH, m_width, m_height);
    setPosition(x, y);
}

bool ModalDialog::handleEvent(const SDL_Event& e) {
    if (!m_isOpen || !m_visible) return false;

    // Children first (buttons, inputs) — a focused TextInput consumes Enter
    // itself, so the dialog confirm below only fires when no child took it.
    if (Panel::handleEvent(e)) return true;

    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
        onCancel();
        return true;
    }

    if (e.type == SDL_EVENT_KEY_DOWN &&
        (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER)) {
        onConfirm();
        return true;
    }

    return false;
}
