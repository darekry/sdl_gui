#include "custom_control.hpp"

CustomControl::CustomControl(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height) {
    setCanGetKeyboardFocus(true);
}

ComponentType CustomControl::getComponentTypeId() const {
    return ComponentType::CustomControl;
}

void CustomControl::drawDirect(SDL_Renderer* renderer) {
    // render() handles hooked elements by rendering into a local target, so
    // this runs only when the control has no hooks at all: draw nothing (the
    // app owns every pixel of an owner-draw control).
    (void)renderer;
}
