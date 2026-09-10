#include "world_view.hpp"
#include "gui_manager.hpp"

WorldView::WorldView(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height)
    , m_worldWidth(width)
    , m_worldHeight(height) {
    auto viewport = std::make_unique<Panel>(manager, 0, 0, width, height);
    viewport->setClipChildren(true);
    m_viewport = viewport.get();

    auto content = std::make_unique<Panel>(manager, 0, 0, width, height);
    m_content = content.get();
    viewport->addChild(std::move(content));

    Panel::addChild(std::move(viewport));
    layoutChildren();
}

void WorldView::setWorldSize(int width, int height) {
    m_worldWidth = std::max(0, width);
    m_worldHeight = std::max(0, height);
    m_content->setSize(std::max(1, m_worldWidth), std::max(1, m_worldHeight));
    clampCamera();
    applyCamera();
    markDirty();
}

void WorldView::setCamera(int x, int y) {
    m_camX = x;
    m_camY = y;
    clampCamera();
    applyCamera();
}

void WorldView::centerOn(int worldX, int worldY) {
    setCamera(worldX - m_width / 2, worldY - m_height / 2);
}

GUIElement* WorldView::addWorldChild(std::unique_ptr<GUIElement> child) {
    return m_content->addChild(std::move(child));
}

SDL_Point WorldView::worldToScreen(int worldX, int worldY) const {
    return {worldX - m_camX, worldY - m_camY};
}

SDL_Point WorldView::screenToWorld(int screenX, int screenY) const {
    return {screenX + m_camX, screenY + m_camY};
}

void WorldView::layoutChildren() {
    // Viewport fills the whole widget (no sliders — camera is programmatic).
    m_viewport->setPosition(0, 0);
    m_viewport->setSize(m_width, m_height);
    clampCamera();
    applyCamera();
}

void WorldView::clampCamera() {
    int maxX = std::max(0, m_worldWidth - m_width);
    int maxY = std::max(0, m_worldHeight - m_height);
    m_camX = std::clamp(m_camX, 0, maxX);
    m_camY = std::clamp(m_camY, 0, maxY);
}

void WorldView::applyCamera() {
    m_content->setPosition(-m_camX, -m_camY);
}
