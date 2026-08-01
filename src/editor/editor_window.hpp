#pragma once

#include "../gui.hpp"
#include "../panel.hpp"
#include "../button.hpp"
#include "../label.hpp"
#include "../text_input.hpp"
#include "../text_area.hpp"
#include "../checkbox.hpp"
#include "../combobox.hpp"
#include "../list_view.hpp"
#include "../slider.hpp"
#include "../window_manager.hpp"
#include "editor_state.hpp"
#include "layout_exporter.hpp"

#include "std.hpp"

class EditorWindow {
public:
    using ElementCallback = std::function<void(size_t)>;
    using VoidCallback = std::function<void()>;
    using WidgetTypeCallback = std::function<void(const std::string&)>;

    EditorWindow(WindowManager& windowManager, EditorState& state);
    ~EditorWindow();

    Window* getWindow() { return m_window; }
    GUIManager& getGUIManager();

    void updateElementsList();
    void updatePropertiesPanel();
    void selectPaletteButton(const std::string& widgetType);

    ElementCallback onElementAdded;
    ElementCallback onElementUpdated;
    ElementCallback onElementDeleted;
    ElementCallback onElementSelected;
    WidgetTypeCallback onSelectedWidgetTypeChanged;

private:
    void createPalettePanel();
    void createPropertiesPanel();
    void createElementsListPanel();
    void createBottomButtons();

    void addWidgetTypeButton(const std::string& type, int row, int col);
    void addStyleStateSelector(int y);

    void handleWidgetTypeSelected(const std::string& type);
    void handleElementSelected(size_t index);
    void handleDeleteElement();
    void handleDuplicateElement();
    void handleAddAsChildToggle();
    void handleSaveXML();
    void handleSaveJSON();
    void handleLoad();
    void handleClear();

    void updatePropertyFromInput(const std::string& key, const std::string& value);
    void updatePositionFromInputs();
    void updateSizeFromInputs();
    void updateColorFromSliders(const std::string& colorKey);
    void updateTextFromInput();
    void updateFontSizeFromInput();
    void updateBorderFromInputs();
    void updateStyleFromInputs();
    void populateStyleFields();

    WindowManager& m_windowManager;
    EditorState& m_editorState;
    Window* m_window = nullptr;
    Uint32 m_windowID = 0;
    bool m_windowOwned = true;
    Panel* m_mainPanel = nullptr;

    Panel* m_palettePanel = nullptr;
    std::unordered_map<std::string, Button*> m_paletteButtons;

    Panel* m_propertiesPanel = nullptr;
    TextInput* m_idInput = nullptr;
    TextInput* m_xInput = nullptr;
    TextInput* m_yInput = nullptr;
    TextInput* m_wInput = nullptr;
    TextInput* m_hInput = nullptr;
    TextInput* m_textInput = nullptr;
    TextInput* m_fontSizeInput = nullptr;
    TextInput* m_borderWidthInput = nullptr;
    TextInput* m_borderRadiusInput = nullptr;
    std::unordered_map<std::string, TextInput*> m_propertyInputs;
    std::unordered_map<std::string, Checkbox*> m_propertyCheckboxes;
    ComboBox* m_styleStateCombo = nullptr;
    std::array<Slider*, 4> m_bgColorSliders = {nullptr, nullptr, nullptr, nullptr};
    std::array<Slider*, 4> m_textColorSliders = {nullptr, nullptr, nullptr, nullptr};
    std::array<Slider*, 4> m_borderColorSliders = {nullptr, nullptr, nullptr, nullptr};
    ElementState m_currentStyleState = ElementState::Normal;

    Panel* m_elementsListPanel = nullptr;
    ListView* m_elementsListView = nullptr;
    Button* m_deleteButton = nullptr;
    Button* m_duplicateButton = nullptr;
    Button* m_addAsChildButton = nullptr;
    bool m_addAsChildMode = false;

    static constexpr int WINDOW_WIDTH = 400;
    static constexpr int WINDOW_HEIGHT = 900;
    static constexpr int PALETTE_HEIGHT = 120;
    static constexpr int PROPERTIES_HEIGHT = 550;
    static constexpr int ELEMENTS_LIST_HEIGHT = 150;
    static constexpr int BOTTOM_BUTTONS_HEIGHT = 50;
};