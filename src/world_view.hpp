#pragma once
#include "panel.hpp"

// WorldView — camera viewport for 2D world content (pan-only, no zoom).
//
// Children live in WORLD coordinates: add them via addWorldChild() (or into
// getContent()). The view shows a window of size (m_width, m_height) into a
// world of size (m_worldWidth, m_worldHeight); the camera offset is applied
// as a single content shift (like ScrollArea, but programmatic — no sliders).
// Rendering is clipped to the viewport, so off-camera children are cut.
//
// Isometric note: projection stays orthogonal on purpose. A future
// IProjection hook (worldToScreen/screenToWorld/depthKey) plugs in here
// without touching callers — see .kilo/plans/worldview-camera-plan.md.
class WorldView : public Panel {
public:
    WorldView(GUIManager& manager, int x, int y, int width, int height);

    void setWorldSize(int width, int height);
    [[nodiscard]] int getWorldWidth() const { return m_worldWidth; }
    [[nodiscard]] int getWorldHeight() const { return m_worldHeight; }

    void setCamera(int x, int y);
    void panBy(int dx, int dy) { setCamera(m_camX + dx, m_camY + dy); }
    void centerOn(int worldX, int worldY);
    [[nodiscard]] int getCamX() const { return m_camX; }
    [[nodiscard]] int getCamY() const { return m_camY; }

    GUIElement* addWorldChild(std::unique_ptr<GUIElement> child);
    [[nodiscard]] GUIElement* getContent() const { return m_content; }

    [[nodiscard]] SDL_Point worldToScreen(int worldX, int worldY) const;
    [[nodiscard]] SDL_Point screenToWorld(int screenX, int screenY) const;

    ComponentType getComponentTypeId() const override { return ComponentType::WorldView; }

protected:
    void layoutChildren() override;

private:
    void clampCamera();
    void applyCamera();

    Panel* m_viewport = nullptr;
    GUIElement* m_content = nullptr;
    int m_worldWidth = 0;
    int m_worldHeight = 0;
    int m_camX = 0;
    int m_camY = 0;
};
