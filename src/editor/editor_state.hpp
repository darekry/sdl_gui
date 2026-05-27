#pragma once

#include "editor_element.hpp"

import std.compat;

class EditorState {
public:
    EditorState();

    [[nodiscard]] const std::vector<EditorElement>& getElements() const { return m_elements; }
    [[nodiscard]] std::vector<EditorElement>& getElements() { return m_elements; }
    
    [[nodiscard]] const std::string& getSelectedWidgetType() const { return m_selectedWidgetType; }
    void setSelectedWidgetType(const std::string& type) { m_selectedWidgetType = type; }
    
    [[nodiscard]] size_t getSelectedElementIndex() const { return m_selectedElementIndex; }
    [[nodiscard]] bool hasSelectedElement() const { return m_selectedElementIndex < m_elements.size(); }
    [[nodiscard]] const EditorElement* getSelectedElement() const;
    [[nodiscard]] EditorElement* getSelectedElement();
    
    [[nodiscard]] int getGridSize() const { return m_gridSize; }
    void setGridSize(int size) { m_gridSize = size; }
    
    [[nodiscard]] int snapToGrid(int value) const;
    
    size_t addElement(const std::string& type, int x, int y, const std::string& parentId = "");
    
    void updateElement(size_t index, const EditorElement& changes);
    void updateElementProperty(size_t index, const std::string& key, const std::string& value);
    void updateElementPosition(size_t index, int x, int y);
    void updateElementSize(size_t index, int width, int height);
    void updateElementStyle(size_t index, ElementState state, const Style& style);
    
    void deleteElement(size_t index);
    size_t duplicateElement(size_t index);
    
    void moveElement(size_t index, int x, int y);
    
    void selectElement(size_t index);
    void clearSelection();
    
    [[nodiscard]] std::vector<size_t> getElementsByParent(const std::string& parentId) const;
    [[nodiscard]] std::vector<size_t> getRootElements() const;
    
    [[nodiscard]] std::optional<size_t> findElementById(const std::string& id) const;
    [[nodiscard]] std::optional<size_t> findElementAtPosition(int x, int y) const;
    
    void clear();
    
    [[nodiscard]] std::string generateId(const std::string& type);

private:
    std::vector<EditorElement> m_elements;
    std::string m_selectedWidgetType;
    size_t m_selectedElementIndex = static_cast<size_t>(-1);
    int m_gridSize = 20;
    std::map<std::string, int> m_idCounters;
};