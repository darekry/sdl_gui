#include "editor_window.hpp"
#include "layout_importer.hpp"
#include "../gui_manager.hpp"
#include "../theme.hpp"
#include "../composite/message_box.hpp"

import std.compat;

static constexpr std::array<const char*, 14> WIDGET_TYPES = {
    "Button", "Label", "Checkbox", "RadioButton", "RadioGroup",
    "Slider", "TextInput", "TextArea", "ComboBox", "ListView",
    "Panel", "TabControl", "Canvas", "StringGrid"
};

EditorWindow::EditorWindow(WindowManager& windowManager, EditorState& state)
    : m_windowManager(windowManager), m_editorState(state) {
    m_window = m_windowManager.createWindow("WYSIWYG Editor", WINDOW_WIDTH, WINDOW_HEIGHT);
    m_windowID = m_window->getWindowID();
    
    auto& guiManager = m_window->getGUIManager();
    
    m_mainPanel = new Panel(guiManager, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    m_mainPanel->setBackgroundColor(ElementState::Normal, guiManager.getTheme().getBackgroundColor());
    guiManager.addElement(std::unique_ptr<GUIElement>(m_mainPanel));
    
    createPalettePanel();
    createPropertiesPanel();
    createElementsListPanel();
    createBottomButtons();
    
    m_window->setOnCloseCallback([this](Window* w) {
        m_window = nullptr;
        m_windowOwned = false;
        m_windowManager.closeWindow(w->getWindowID());
    });
}

EditorWindow::~EditorWindow() {
    if (m_windowOwned && m_windowID != 0) {
        Window* w = m_windowManager.getWindowByID(m_windowID);
        if (w && !w->isMarkedForClose()) {
            m_windowManager.closeWindow(m_windowID);
        }
    }
}

GUIManager& EditorWindow::getGUIManager() {
    return m_window->getGUIManager();
}

void EditorWindow::rebuild() {
    updateElementsList();
    updatePropertiesPanel();
}

void EditorWindow::createPalettePanel() {
    auto& guiManager = m_window->getGUIManager();
    
    m_palettePanel = new Panel(guiManager, 5, 5, WINDOW_WIDTH - 10, PALETTE_HEIGHT);
    m_palettePanel->setBackgroundColor(ElementState::Normal, SDL_Color{40, 40, 50, 255});
    m_palettePanel->setBorder(ElementState::Normal, SDL_Color{80, 80, 100, 255}, 1);
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(m_palettePanel));
    
    auto titleLabel = new Label(guiManager, 10, 8, "Widget Palette", 14);
    titleLabel->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_palettePanel->addChild(std::unique_ptr<GUIElement>(titleLabel));
    
    constexpr int BUTTON_WIDTH = 75;
    constexpr int BUTTON_HEIGHT = 25;
    constexpr int BUTTONS_PER_ROW = 5;
    constexpr int START_Y = 30;
    constexpr int H_SPACING = 5;
    constexpr int V_SPACING = 5;
    
    for (size_t i = 0; i < WIDGET_TYPES.size(); ++i) {
        int row = static_cast<int>(i) / BUTTONS_PER_ROW;
        int col = static_cast<int>(i) % BUTTONS_PER_ROW;
        int x = 10 + col * (BUTTON_WIDTH + H_SPACING);
        int y = START_Y + row * (BUTTON_HEIGHT + V_SPACING);
        
        std::string type = WIDGET_TYPES[i];
        auto button = new Button(guiManager, x, y, BUTTON_WIDTH, BUTTON_HEIGHT, type);
        button->setBackgroundColor(ElementState::Normal, SDL_Color{60, 60, 80, 255});
        button->setBackgroundColor(ElementState::Hover, SDL_Color{80, 80, 100, 255});
        button->setBackgroundColor(ElementState::Pressed, SDL_Color{100, 100, 120, 255});
        button->setTextColor(ElementState::Normal, SDL_Color{180, 180, 200, 255});
        button->setOnClickCallback([this, type](GUIElement*) {
            handleWidgetTypeSelected(type);
        });
        
        m_paletteButtons[type] = button;
        m_palettePanel->addChild(std::unique_ptr<GUIElement>(button));
    }
}

void EditorWindow::createPropertiesPanel() {
    auto& guiManager = m_window->getGUIManager();
    
    int startY = PALETTE_HEIGHT + 10;
    m_propertiesPanel = new Panel(guiManager, 5, startY, WINDOW_WIDTH - 10, PROPERTIES_HEIGHT);
    m_propertiesPanel->setBackgroundColor(ElementState::Normal, SDL_Color{40, 40, 50, 255});
    m_propertiesPanel->setBorder(ElementState::Normal, SDL_Color{80, 80, 100, 255}, 1);
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(m_propertiesPanel));
    
    auto titleLabel = new Label(guiManager, 10, startY + 3, "Properties", 14);
    titleLabel->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(titleLabel));
    
    int labelY = startY + 25;
    constexpr int INPUT_WIDTH = 80;
    constexpr int INPUT_HEIGHT = 22;
    constexpr int LABEL_X = 10;
    constexpr int INPUT_X = 60;
    constexpr int ROW_HEIGHT = 28;
    
    auto idLabel = new Label(guiManager, LABEL_X, labelY, "ID:", 12);
    idLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(idLabel));
    
    m_idInput = new TextInput(guiManager, INPUT_X, labelY - 2, INPUT_WIDTH * 2, INPUT_HEIGHT);
    m_idInput->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 255});
    m_idInput->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_idInput->setBorder(ElementState::Normal, SDL_Color{70, 70, 90, 255}, 1);
    m_idInput->setOnTextChanged([this](TextInput*) {
        if (m_editorState.hasSelectedElement()) {
            m_editorState.updateElementProperty(m_editorState.getSelectedElementIndex(), "id", m_idInput->getText());
            updateElementsList();
            if (onElementUpdated) onElementUpdated(m_editorState.getSelectedElementIndex());
        }
    });
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_idInput));
    
    labelY += ROW_HEIGHT;
    
    auto xLabel = new Label(guiManager, LABEL_X, labelY, "X:", 12);
    xLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(xLabel));
    
    m_xInput = new TextInput(guiManager, INPUT_X, labelY - 2, INPUT_WIDTH, INPUT_HEIGHT);
    m_xInput->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 255});
    m_xInput->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_xInput->setBorder(ElementState::Normal, SDL_Color{70, 70, 90, 255}, 1);
    m_xInput->setOnTextChanged([this](TextInput*) {
        updatePositionFromInputs();
    });
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_xInput));
    
    auto yLabel = new Label(guiManager, INPUT_X + INPUT_WIDTH + 10, labelY, "Y:", 12);
    yLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(yLabel));
    
    m_yInput = new TextInput(guiManager, INPUT_X + INPUT_WIDTH + 10 + 30, labelY - 2, INPUT_WIDTH, INPUT_HEIGHT);
    m_yInput->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 255});
    m_yInput->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_yInput->setBorder(ElementState::Normal, SDL_Color{70, 70, 90, 255}, 1);
    m_yInput->setOnTextChanged([this](TextInput*) {
        updatePositionFromInputs();
    });
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_yInput));
    
    labelY += ROW_HEIGHT;
    
    auto wLabel = new Label(guiManager, LABEL_X, labelY, "W:", 12);
    wLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(wLabel));
    
    m_wInput = new TextInput(guiManager, INPUT_X, labelY - 2, INPUT_WIDTH, INPUT_HEIGHT);
    m_wInput->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 255});
    m_wInput->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_wInput->setBorder(ElementState::Normal, SDL_Color{70, 70, 90, 255}, 1);
    m_wInput->setOnTextChanged([this](TextInput*) {
        updateSizeFromInputs();
    });
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_wInput));
    
    auto hLabel = new Label(guiManager, INPUT_X + INPUT_WIDTH + 10, labelY, "H:", 12);
    hLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(hLabel));
    
    m_hInput = new TextInput(guiManager, INPUT_X + INPUT_WIDTH + 10 + 30, labelY - 2, INPUT_WIDTH, INPUT_HEIGHT);
    m_hInput->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 255});
    m_hInput->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_hInput->setBorder(ElementState::Normal, SDL_Color{70, 70, 90, 255}, 1);
    m_hInput->setOnTextChanged([this](TextInput*) {
        updateSizeFromInputs();
    });
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_hInput));
    
    addStyleStateSelector(labelY + ROW_HEIGHT);
}

void EditorWindow::addStyleStateSelector(int y) {
    auto& guiManager = m_window->getGUIManager();
    constexpr int LABEL_X = 10;
    constexpr int COMBO_X = 60;
    constexpr int COMBO_WIDTH = 100;
    constexpr int COMBO_HEIGHT = 22;
    
    auto stateLabel = new Label(guiManager, LABEL_X, y, "State:", 12);
    stateLabel->setTextColor(ElementState::Normal, SDL_Color{160, 160, 180, 255});
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(stateLabel));
    
    m_styleStateCombo = new ComboBox(guiManager, COMBO_X, y - 2, COMBO_WIDTH, COMBO_HEIGHT);
    m_styleStateCombo->addItem("Normal");
    m_styleStateCombo->addItem("Hover");
    m_styleStateCombo->addItem("Pressed");
    m_styleStateCombo->addItem("Disabled");
    m_styleStateCombo->setSelectedIndex(0);
    m_styleStateCombo->on_selection_changed = [this](int index, const std::string&) {
        m_currentStyleState = static_cast<ElementState>(index);
        updatePropertiesPanel();
    };
    m_propertiesPanel->addChild(std::unique_ptr<GUIElement>(m_styleStateCombo));
}

void EditorWindow::createElementsListPanel() {
    auto& guiManager = m_window->getGUIManager();
    
    int startY = PALETTE_HEIGHT + PROPERTIES_HEIGHT + 20;
    m_elementsListPanel = new Panel(guiManager, 5, startY, WINDOW_WIDTH - 10, ELEMENTS_LIST_HEIGHT);
    m_elementsListPanel->setBackgroundColor(ElementState::Normal, SDL_Color{40, 40, 50, 255});
    m_elementsListPanel->setBorder(ElementState::Normal, SDL_Color{80, 80, 100, 255}, 1);
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(m_elementsListPanel));
    
    auto titleLabel = new Label(guiManager, 10, startY + 3, "Elements", 14);
    titleLabel->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(titleLabel));
    
    constexpr int LIST_X = 10;
    int LIST_Y = startY + 25;
    constexpr int LIST_WIDTH = WINDOW_WIDTH - 100;
    constexpr int LIST_HEIGHT = ELEMENTS_LIST_HEIGHT - 35;
    
    m_elementsListView = new ListView(guiManager, LIST_X, LIST_Y, LIST_WIDTH, LIST_HEIGHT);
    m_elementsListView->setBackgroundColor(ElementState::Normal, SDL_Color{30, 30, 40, 255});
    m_elementsListView->setSelectionColor(SDL_Color{70, 70, 100, 255});
    m_elementsListView->setOnRowClick([this](ListView*, size_t index) {
        handleElementSelected(index);
    });
    m_elementsListPanel->addChild(std::unique_ptr<GUIElement>(m_elementsListView));
    
    constexpr int BUTTON_X = LIST_X + LIST_WIDTH + 5;
    constexpr int BUTTON_WIDTH = 45;
    constexpr int BUTTON_HEIGHT = 22;
    constexpr int BUTTON_SPACING = 3;
    
    m_deleteButton = new Button(guiManager, BUTTON_X, LIST_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Del");
    m_deleteButton->setBackgroundColor(ElementState::Normal, SDL_Color{150, 50, 50, 255});
    m_deleteButton->setBackgroundColor(ElementState::Hover, SDL_Color{180, 60, 60, 255});
    m_deleteButton->setTextColor(ElementState::Normal, SDL_Color{220, 220, 220, 255});
    m_deleteButton->setOnClickCallback([this](GUIElement*) {
        handleDeleteElement();
    });
    m_elementsListPanel->addChild(std::unique_ptr<GUIElement>(m_deleteButton));
    
    m_duplicateButton = new Button(guiManager, BUTTON_X, LIST_Y + BUTTON_HEIGHT + BUTTON_SPACING, 
                                   BUTTON_WIDTH, BUTTON_HEIGHT, "Dup");
    m_duplicateButton->setBackgroundColor(ElementState::Normal, SDL_Color{50, 100, 50, 255});
    m_duplicateButton->setBackgroundColor(ElementState::Hover, SDL_Color{60, 120, 60, 255});
    m_duplicateButton->setTextColor(ElementState::Normal, SDL_Color{220, 220, 220, 255});
    m_duplicateButton->setOnClickCallback([this](GUIElement*) {
        handleDuplicateElement();
    });
    m_elementsListPanel->addChild(std::unique_ptr<GUIElement>(m_duplicateButton));
    
    m_addAsChildButton = new Button(guiManager, BUTTON_X, LIST_Y + 2 * (BUTTON_HEIGHT + BUTTON_SPACING),
                                    BUTTON_WIDTH, BUTTON_HEIGHT, "Child");
    m_addAsChildButton->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 150, 255});
    m_addAsChildButton->setBackgroundColor(ElementState::Hover, SDL_Color{60, 60, 180, 255});
    m_addAsChildButton->setTextColor(ElementState::Normal, SDL_Color{220, 220, 220, 255});
    m_addAsChildButton->setOnClickCallback([this](GUIElement*) {
        handleAddAsChildToggle();
    });
    m_elementsListPanel->addChild(std::unique_ptr<GUIElement>(m_addAsChildButton));
}

void EditorWindow::createBottomButtons() {
    auto& guiManager = m_window->getGUIManager();
    
    int startY = WINDOW_HEIGHT - BOTTOM_BUTTONS_HEIGHT - 5;
    
    constexpr int BUTTON_WIDTH = 80;
    constexpr int BUTTON_HEIGHT = 30;
    constexpr int BUTTON_SPACING = 10;
    constexpr int START_X = 10;
    
    auto saveXMLBtn = new Button(guiManager, START_X, startY, BUTTON_WIDTH, BUTTON_HEIGHT, "Save XML");
    saveXMLBtn->setBackgroundColor(ElementState::Normal, SDL_Color{60, 80, 120, 255});
    saveXMLBtn->setBackgroundColor(ElementState::Hover, SDL_Color{80, 100, 150, 255});
    saveXMLBtn->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    saveXMLBtn->setOnClickCallback([this](GUIElement*) {
        handleSaveXML();
    });
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(saveXMLBtn));
    
    auto saveJSONBtn = new Button(guiManager, START_X + BUTTON_WIDTH + BUTTON_SPACING, startY, 
                                  BUTTON_WIDTH, BUTTON_HEIGHT, "Save JSON");
    saveJSONBtn->setBackgroundColor(ElementState::Normal, SDL_Color{60, 80, 120, 255});
    saveJSONBtn->setBackgroundColor(ElementState::Hover, SDL_Color{80, 100, 150, 255});
    saveJSONBtn->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    saveJSONBtn->setOnClickCallback([this](GUIElement*) {
        handleSaveJSON();
    });
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(saveJSONBtn));
    
    auto loadBtn = new Button(guiManager, START_X + 2 * (BUTTON_WIDTH + BUTTON_SPACING), startY,
                              BUTTON_WIDTH, BUTTON_HEIGHT, "Load");
    loadBtn->setBackgroundColor(ElementState::Normal, SDL_Color{80, 60, 120, 255});
    loadBtn->setBackgroundColor(ElementState::Hover, SDL_Color{100, 80, 150, 255});
    loadBtn->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    loadBtn->setOnClickCallback([this](GUIElement*) {
        handleLoad();
    });
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(loadBtn));
    
    auto clearBtn = new Button(guiManager, START_X + 3 * (BUTTON_WIDTH + BUTTON_SPACING), startY,
                               BUTTON_WIDTH, BUTTON_HEIGHT, "Clear");
    clearBtn->setBackgroundColor(ElementState::Normal, SDL_Color{120, 60, 60, 255});
    clearBtn->setBackgroundColor(ElementState::Hover, SDL_Color{150, 80, 80, 255});
    clearBtn->setTextColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
    clearBtn->setOnClickCallback([this](GUIElement*) {
        handleClear();
    });
    m_mainPanel->addChild(std::unique_ptr<GUIElement>(clearBtn));
}

void EditorWindow::handleWidgetTypeSelected(const std::string& type) {
    m_selectedPaletteType = type;
    m_editorState.setSelectedWidgetType(type);
    selectPaletteButton(type);
    if (onSelectedWidgetTypeChanged) {
        onSelectedWidgetTypeChanged(type);
    }
}

void EditorWindow::selectPaletteButton(const std::string& widgetType) {
    for (auto& [type, button] : m_paletteButtons) {
        if (type == widgetType) {
            button->setBackgroundColor(ElementState::Normal, SDL_Color{100, 120, 160, 255});
        } else {
            button->setBackgroundColor(ElementState::Normal, SDL_Color{60, 60, 80, 255});
        }
        button->markDirty();
    }
}

void EditorWindow::handleElementSelected(size_t index) {
    m_editorState.selectElement(index);
    updatePropertiesPanel();
    if (onElementSelected) {
        onElementSelected(index);
    }
}

void EditorWindow::handleDeleteElement() {
    if (m_editorState.hasSelectedElement()) {
        size_t index = m_editorState.getSelectedElementIndex();
        m_editorState.deleteElement(index);
        updateElementsList();
        updatePropertiesPanel();
        if (onElementDeleted) {
            onElementDeleted(index);
        }
    }
}

void EditorWindow::handleDuplicateElement() {
    if (m_editorState.hasSelectedElement()) {
        size_t index = m_editorState.getSelectedElementIndex();
        size_t newIndex = m_editorState.duplicateElement(index);
        updateElementsList();
        updatePropertiesPanel();
        if (onElementAdded) {
            onElementAdded(newIndex);
        }
    }
}

void EditorWindow::handleAddAsChildToggle() {
    m_addAsChildMode = !m_addAsChildMode;
    if (m_addAsChildMode) {
        m_addAsChildButton->setBackgroundColor(ElementState::Normal, SDL_Color{100, 100, 200, 255});
    } else {
        m_addAsChildButton->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 150, 255});
    }
    m_addAsChildButton->markDirty();
}

void EditorWindow::handleSaveXML() {
    std::string filePath = "layout.xml";
    bool success = LayoutExporter::saveToXML(m_editorState.getElements(), filePath);
    
    if (success) {
        MessageBox::showInfo(getGUIManager(), "Saved to " + filePath);
    } else {
        MessageBox::showError(getGUIManager(), "Failed to save to " + filePath);
    }
}

void EditorWindow::handleSaveJSON() {
    std::string filePath = "layout.json";
    bool success = LayoutExporter::saveToJSON(m_editorState.getElements(), filePath);
    
    if (success) {
        MessageBox::showInfo(getGUIManager(), "Saved to " + filePath);
    } else {
        MessageBox::showError(getGUIManager(), "Failed to save to " + filePath);
    }
}

void EditorWindow::handleLoad() {
    std::vector<EditorElement> loadedElements;
    std::string loadedFile;
    
    loadedElements = LayoutImporter::loadFromXML("layout.xml");
    if (!loadedElements.empty()) {
        loadedFile = "layout.xml";
    } else {
        loadedElements = LayoutImporter::loadFromJSON("layout.json");
        if (!loadedElements.empty()) {
            loadedFile = "layout.json";
        }
    }
    
    if (!loadedElements.empty()) {
        m_editorState.clear();
        for (const auto& elem : loadedElements) {
            m_editorState.getElements().push_back(elem);
        }
        updateElementsList();
        updatePropertiesPanel();
        
        MessageBox::showInfo(getGUIManager(), "Loaded " + std::to_string(loadedElements.size()) + " elements from " + loadedFile);
        
        if (onElementAdded) {
            for (size_t i = 0; i < loadedElements.size(); ++i) {
                onElementAdded(i);
            }
        }
    } else {
        MessageBox::showError(getGUIManager(), "Failed to load layout file (layout.xml or layout.json)");
    }
}

void EditorWindow::handleClear() {
    m_editorState.clear();
    updateElementsList();
    updatePropertiesPanel();
    if (onElementDeleted) {
        onElementDeleted(static_cast<size_t>(-1));
    }
}

void EditorWindow::updateElementsList() {
    m_elementsListView->clearItems();
    
    auto rootIndices = m_editorState.getRootElements();
    for (size_t rootIdx : rootIndices) {
        const auto& elem = m_editorState.getElements()[rootIdx];
        std::string displayText = elem.id + " (" + elem.type + ")";
        m_elementsListView->addItem(displayText);
        
        auto childIndices = m_editorState.getElementsByParent(elem.id);
        for (size_t childIdx : childIndices) {
            const auto& child = m_editorState.getElements()[childIdx];
            std::string childText = "  " + child.id + " (" + child.type + ")";
            m_elementsListView->addItem(childText);
        }
    }
    
    if (m_editorState.hasSelectedElement()) {
        size_t selectedIndex = m_editorState.getSelectedElementIndex();
        m_elementsListView->setSelectedRow(selectedIndex);
    }
}

void EditorWindow::updatePropertiesPanel() {
    auto* selected = m_editorState.getSelectedElement();
    if (!selected) {
        m_idInput->setText(std::string(""));
        m_xInput->setText(std::string(""));
        m_yInput->setText(std::string(""));
        m_wInput->setText(std::string(""));
        m_hInput->setText(std::string(""));
        return;
    }
    
    m_idInput->setText(std::string(selected->id));
    m_xInput->setText(std::to_string(selected->x));
    m_yInput->setText(std::to_string(selected->y));
    m_wInput->setText(std::to_string(selected->width));
    m_hInput->setText(std::to_string(selected->height));
    
    std::string text = selected->getProperty("text", "");
    if (selected->type == "TextArea") {
        if (m_propertyTextAreas.find("text") == m_propertyTextAreas.end()) {
        }
    }
}

void EditorWindow::updatePositionFromInputs() {
    if (!m_editorState.hasSelectedElement()) return;
    
    try {
        int x = std::stoi(m_xInput->getText());
        int y = std::stoi(m_yInput->getText());
        m_editorState.updateElementPosition(m_editorState.getSelectedElementIndex(), x, y);
        if (onElementUpdated) {
            onElementUpdated(m_editorState.getSelectedElementIndex());
        }
    } catch (...) {
    }
}

void EditorWindow::updateSizeFromInputs() {
    if (!m_editorState.hasSelectedElement()) return;
    
    try {
        int w = std::stoi(m_wInput->getText());
        int h = std::stoi(m_hInput->getText());
        m_editorState.updateElementSize(m_editorState.getSelectedElementIndex(), w, h);
        if (onElementUpdated) {
            onElementUpdated(m_editorState.getSelectedElementIndex());
        }
    } catch (...) {
    }
}