#include "editor_state.hpp"

import std.compat;

EditorState::EditorState() = default;

int EditorState::snapToGrid(int value) const {
    if (m_gridSize <= 0) return value;
    return ((value + m_gridSize / 2) / m_gridSize) * m_gridSize;
}

std::string EditorState::generateId(const std::string& type) {
    std::string lowerType = type;
    if (!lowerType.empty()) {
        lowerType[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowerType[0])));
    }
    
    int counter = m_idCounters[lowerType] + 1;
    m_idCounters[lowerType] = counter;
    
    return lowerType + std::to_string(counter);
}

size_t EditorState::addElement(const std::string& type, int x, int y, const std::string& parentId) {
    std::string id = generateId(type);
    
    EditorElement element;
    element.id = id;
    element.type = type;
    element.x = snapToGrid(x);
    element.y = snapToGrid(y);
    element.parentId = parentId;
    
    if (type == "Button") { element.width = 120; element.height = 40; }
    else if (type == "Label") { element.width = 100; element.height = 25; }
    else if (type == "Checkbox") { element.width = 150; element.height = 25; }
    else if (type == "RadioButton") { element.width = 150; element.height = 25; }
    else if (type == "RadioGroup") { element.width = 150; element.height = 100; }
    else if (type == "Slider") { element.width = 150; element.height = 20; }
    else if (type == "TextInput") { element.width = 150; element.height = 30; }
    else if (type == "TextArea") { element.width = 200; element.height = 150; }
    else if (type == "TabControl") { element.width = 300; element.height = 200; }
    else if (type == "Panel") { element.width = 300; element.height = 200; }
    else if (type == "AnimatedImage") { element.width = 100; element.height = 100; }
    else if (type == "ComboBox") { element.width = 150; element.height = 30; }
    else if (type == "Canvas") { element.width = 200; element.height = 200; }
    else if (type == "StringGrid") { element.width = 400; element.height = 300; }
    else if (type == "ListView") { element.width = 200; element.height = 200; }
    else { element.width = 100; element.height = 50; }
    
    m_elements.push_back(std::move(element));
    size_t newIndex = m_elements.size() - 1;
    selectElement(newIndex);
    
    return newIndex;
}

void EditorState::updateElement(size_t index, const EditorElement& changes) {
    if (index >= m_elements.size()) return;
    
    if (!changes.id.empty()) m_elements[index].id = changes.id;
    if (!changes.type.empty()) m_elements[index].type = changes.type;
    // Position should be updated via updateElementPosition() for proper grid snapping
    // This method only updates non-zero dimensions
    if (changes.width > 0) m_elements[index].width = changes.width;
    if (changes.height > 0) m_elements[index].height = changes.height;
    
    for (const auto& [key, value] : changes.properties) {
        m_elements[index].setProperty(key, value);
    }
    
    for (const auto& [state, style] : changes.styles) {
        m_elements[index].setStyle(state, style);
    }
}

void EditorState::updateElementProperty(size_t index, const std::string& key, const std::string& value) {
    if (index >= m_elements.size()) return;
    m_elements[index].setProperty(key, value);
}

void EditorState::updateElementPosition(size_t index, int x, int y) {
    if (index >= m_elements.size()) return;
    m_elements[index].x = snapToGrid(x);
    m_elements[index].y = snapToGrid(y);
}

void EditorState::updateElementSize(size_t index, int width, int height) {
    if (index >= m_elements.size()) return;
    m_elements[index].width = width;
    m_elements[index].height = height;
}

void EditorState::updateElementStyle(size_t index, ElementState state, const Style& style) {
    if (index >= m_elements.size()) return;
    m_elements[index].setStyle(state, style);
}

void EditorState::deleteElement(size_t index) {
    if (index >= m_elements.size()) return;
    
    std::string deletedId = m_elements[index].id;
    m_elements.erase(m_elements.begin() + static_cast<std::ptrdiff_t>(index));
    
    for (auto& element : m_elements) {
        if (element.parentId == deletedId) {
            element.parentId = "";
        }
    }
    
    if (m_selectedElementIndex == index) {
        clearSelection();
    } else if (m_selectedElementIndex > index) {
        m_selectedElementIndex--;
    }
}

size_t EditorState::duplicateElement(size_t index) {
    if (index >= m_elements.size()) return static_cast<size_t>(-1);
    
    EditorElement copy = m_elements[index];
    copy.id = generateId(copy.type);
    copy.x = snapToGrid(copy.x + 20);
    copy.y = snapToGrid(copy.y + 20);
    
    m_elements.push_back(std::move(copy));
    size_t newIndex = m_elements.size() - 1;
    selectElement(newIndex);
    
    return newIndex;
}

void EditorState::moveElement(size_t index, int x, int y) {
    updateElementPosition(index, x, y);
}

void EditorState::selectElement(size_t index) {
    if (index < m_elements.size()) {
        m_selectedElementIndex = index;
    } else {
        clearSelection();
    }
}

void EditorState::clearSelection() {
    m_selectedElementIndex = static_cast<size_t>(-1);
}

const EditorElement* EditorState::getSelectedElement() const {
    if (m_selectedElementIndex < m_elements.size()) {
        return &m_elements[m_selectedElementIndex];
    }
    return nullptr;
}

EditorElement* EditorState::getSelectedElement() {
    if (m_selectedElementIndex < m_elements.size()) {
        return &m_elements[m_selectedElementIndex];
    }
    return nullptr;
}

std::vector<size_t> EditorState::getElementsByParent(const std::string& parentId) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (m_elements[i].parentId == parentId) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> EditorState::getRootElements() const {
    return getElementsByParent("");
}

std::optional<size_t> EditorState::findElementById(const std::string& id) const {
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (m_elements[i].id == id) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<size_t> EditorState::findElementAtPosition(int x, int y) const {
    for (size_t i = m_elements.size(); i > 0; --i) {
        size_t idx = i - 1;
        const auto& elem = m_elements[idx];
        if (x >= elem.x && x < elem.x + elem.width &&
            y >= elem.y && y < elem.y + elem.height) {
            return idx;
        }
    }
    return std::nullopt;
}

void EditorState::clear() {
    m_elements.clear();
    m_idCounters.clear();
    clearSelection();
}