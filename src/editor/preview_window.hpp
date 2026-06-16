#pragma once

#include "../gui.hpp"
#include "../panel.hpp"
#include "editor_state.hpp"

import std.compat;

class CanvasPanel;

class PreviewWindow {
public:
    PreviewWindow(GUIManager& manager, EditorState& state, int width = 800, int height = 600);
    ~PreviewWindow();

    CanvasPanel* getCanvas() { return m_canvas; }
    
    void setOnSelectionChanged(std::function<void(size_t)> callback) { m_onSelectionChanged = callback; }
    void setOnElementMoved(std::function<void(size_t, int, int)> callback) { m_onElementMoved = callback; }
    
    void refreshAllElements();
    void refreshElement(size_t index);
    void removeElementWidget(size_t index);
    void clearAllWidgets();
    
    void setShowGrid(bool show) { m_showGrid = show; }
    [[nodiscard]] bool isShowGrid() const { return m_showGrid; }

    friend class CanvasPanel;

private:
    GUIManager& m_manager;
    EditorState& m_state;
    CanvasPanel* m_canvas = nullptr;
    
    std::map<size_t, GUIElement*> m_widgetMap;
    
    bool m_showGrid = true;
    
    std::function<void(size_t)> m_onSelectionChanged;
    std::function<void(size_t, int, int)> m_onElementMoved;
    
    void createWidgetForElement(size_t index);
    std::unique_ptr<GUIElement> createWidget(const EditorElement& elem);
    void applyElementProperties(GUIElement* widget, const EditorElement& elem);
    void applyElementStyles(GUIElement* widget, const EditorElement& elem);
};

class CanvasPanel : public Panel {
public:
    CanvasPanel(GUIManager& manager, int x, int y, int width, int height, EditorState& state);
    
    void setPreviewWindow(PreviewWindow* preview) { m_previewWindow = preview; }
    
    void setShowGrid(bool show) { m_showGrid = show; markDirty(); }
    [[nodiscard]] bool isShowGrid() const { return m_showGrid; }
    
    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;
    
    void renderOverlay(SDL_Renderer* renderer) override;

protected:
    void draw(SDL_Renderer* renderer) override;
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;

private:
    EditorState& m_state;
    PreviewWindow* m_previewWindow = nullptr;
    
    bool m_showGrid = true;
    bool m_isDragging = false;
    int m_dragOffsetX = 0;
    int m_dragOffsetY = 0;
    int m_dragStartX = 0;
    int m_dragStartY = 0;
    
    void drawGrid(SDL_Renderer* renderer);
    void drawSelectionHighlight(SDL_Renderer* renderer, size_t index);
    
    [[nodiscard]] int snapToGrid(int value) const { return m_state.snapToGrid(value); }
    
    void handleCanvasClick(float mouseX, float mouseY);
    void handleElementClick(float mouseX, float mouseY, size_t elementIndex);
    void startDrag(float mouseX, float mouseY);
    void updateDrag(float mouseX, float mouseY);
    void endDrag();
};