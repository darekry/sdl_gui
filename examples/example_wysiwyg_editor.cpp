/**
 * @file example_wysiwyg_editor.cpp
 * @brief WYSIWYG GUI Editor - Two-window architecture demonstration
 * 
 * This example demonstrates:
 * - WindowManager managing two independent windows
 * - EditorState shared between both windows
 * - EditorWindow: palette, properties panel, elements list
 * - PreviewWindow: live canvas preview in second window
 * 
 * Architecture:
 * 1. WindowManager creates and manages SDL windows
 * 2. EditorState holds shared state (elements, selection, grid)
 * 3. EditorWindow (first window) - control panels
 * 4. PreviewWindow (second window) - interactive canvas
 * 
 * Workflow:
 * - Click palette button -> select widget type
 * - Click preview canvas -> add widget at snapped position
 * - Click widget in preview -> select and enable drag
 * - Properties panel shows selected widget's properties
 * - Elements list shows all widgets with tree structure
 * - Save XML/JSON buttons export layout to files
 */

#include "window_manager.hpp"
#include "editor/editor_window.hpp"
#include "editor/preview_window.hpp"
#include "editor/editor_state.hpp"
#include "logger.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        WindowManager windowManager;
        
        EditorState editorState;
        
        EditorWindow editorWindow(windowManager, editorState);
        
        Window* previewWnd = windowManager.createWindow("Preview Canvas", 800, 600);
        if (!previewWnd) {
            std::cerr << "Failed to create preview window\n";
            return 1;
        }
        
        PreviewWindow previewWindow(previewWnd->getGUIManager(), editorState, 800, 600);
        
        editorWindow.onElementAdded = [&previewWindow](size_t index) {
            previewWindow.refreshElement(index);
        };
        
        editorWindow.onElementUpdated = [&previewWindow](size_t index) {
            previewWindow.refreshElement(index);
        };
        
        editorWindow.onElementDeleted = [&previewWindow](size_t index) {
            if (index == static_cast<size_t>(-1)) {
                previewWindow.clearAllWidgets();
            } else {
                previewWindow.removeElementWidget(index);
            }
        };
        
        editorWindow.onElementSelected = [&editorWindow](size_t) {
            editorWindow.updatePropertiesPanel();
        };
        
        previewWindow.setOnSelectionChanged([&editorWindow](size_t) {
            editorWindow.updateElementsList();
            editorWindow.updatePropertiesPanel();
        });
        
        previewWindow.setOnElementMoved([&editorWindow](size_t, int, int) {
            editorWindow.updatePropertiesPanel();
        });
        
        previewWnd->setOnCloseCallback([&windowManager](Window* w) {
            windowManager.closeWindow(w->getWindowID());
        });
        
        LOG_INFO("WysiwygEditor", "WYSIWYG Editor started\nEditor Window: palette, properties, elements list\nPreview Window: interactive canvas with grid\n\nHow to use:\n1. Click a widget type in the palette (first window)\n2. Click on the preview canvas to add that widget\n3. Click on a widget in preview to select it\n4. Drag selected widgets to move them (snaps to grid)\n5. Edit properties in the properties panel\n6. Save layout with Save XML/Save JSON buttons");
        
        while (!windowManager.shouldQuit()) {
            windowManager.processEvents();
            windowManager.updateAll();
            windowManager.renderAll();
            windowManager.cleanupAll();
            SDL_Delay(16);
        }
        
        LOG_INFO("WysiwygEditor", "WYSIWYG Editor exiting");
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}