#pragma once

#include "../gui.hpp"
#include "../panel.hpp"
#include "../button.hpp"
#include "../label.hpp"

#include "std.hpp"

/**
 * @file dialog_box.hpp
 * @brief DialogBox - composite dialog window component
 * 
 * DialogBox is a ready-to-use higher-level component
 * composed of a Panel (background/border), a Label (title/message) and Buttons (actions).
 * 
 * Usage example:
 * @code
 * auto dialog = DialogBox::createConfirm(manager, "Czy na pewno?", "Tak", "Nie",
 *     [](bool confirmed) { if (confirmed) { // action } });
 * manager.addElement(std::move(dialog));
 * @endcode
 */

class DialogBox : public Panel {
public:
    /// Dialog window types
    enum class DialogType {
        Confirm,    ///< Dialog with two buttons (Yes/No or similar)
        Alert,      ///< Dialog with one button (OK)
        Custom      ///< Dialog with custom buttons
    };

    /// Callback invoked after a button is clicked
    using DialogCallback = std::function<void(int buttonIndex)>;

    /**
     * @brief Creates a confirmation dialog (Yes/No)
     * @param manager GUIManager
     * @param message Message content
     * @param yesLabel Text of the "Yes" button (default "Tak")
     * @param noLabel Text of the "No" button (default "Nie")
     * @param callback Function invoked on click (true = Yes, false = No)
     * @param width Window width (default 400)
     * @param height Window height (default 150)
     * @return unique_ptr to the DialogBox
     */
    static std::unique_ptr<DialogBox> createConfirm(
        GUIManager& manager,
        std::string_view message,
        std::string_view yesLabel = "Tak",
        std::string_view noLabel = "Nie",
        std::function<void(bool confirmed)> callback = nullptr,
        int width = 400,
        int height = 150
    );

    /**
     * @brief Creates an alert dialog (OK)
     * @param manager GUIManager
     * @param message Message content
     * @param okLabel Button text (default "OK")
     * @param callback Function invoked after OK is clicked
     * @param width Window width (default 350)
     * @param height Window height (default 120)
     * @return unique_ptr to the DialogBox
     */
    static std::unique_ptr<DialogBox> createAlert(
        GUIManager& manager,
        std::string_view message,
        std::string_view okLabel = "OK",
        DialogCallback callback = nullptr,
        int width = 350,
        int height = 120
    );

    /**
     * @brief Creates a dialog with custom buttons
     * @param manager GUIManager
     * @param message Message content
     * @param buttonLabels List of button labels
     * @param callback Function invoked on click (button index)
     * @param width Window width
     * @param height Window height
     * @return unique_ptr to the DialogBox
     */
    static std::unique_ptr<DialogBox> createCustom(
        GUIManager& manager,
        std::string_view message,
        const std::vector<std::string>& buttonLabels,
        DialogCallback callback = nullptr,
        int width = 400,
        int height = 150
    );

    /**
     * @brief Creates a dialog with a title
     * @param manager GUIManager
     * @param title Window title
     * @param message Message content
     * @param buttonLabels List of button labels
     * @param callback Function invoked on click
     * @param width Window width
     * @param height Window height
     * @return unique_ptr to the DialogBox
     */
    static std::unique_ptr<DialogBox> createWithTitle(
        GUIManager& manager,
        std::string_view title,
        std::string_view message,
        const std::vector<std::string>& buttonLabels,
        DialogCallback callback = nullptr,
        int width = 400,
        int height = 180
    );

    // Constructor (use the static create* methods)
    DialogBox(GUIManager& manager, int x, int y, int width, int height,
              std::string_view message, const std::vector<std::string>& buttonLabels,
              DialogType type, DialogCallback callback = nullptr);

    void setMessage(std::string_view message);

    void setTitle(std::string_view title);

    /// Closes the dialog (removes the element)
    void close();

    bool isOpen() const { return m_isOpen; }

    DialogType getDialogType() const { return m_type; }

    /// Returns the index of the clicked button (-1 if none)
    int getLastClickedButton() const { return m_lastClickedButton; }

    ComponentType getComponentTypeId() const override;

    bool isOverlay() const override { return true; }

protected:
    void draw(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    // Pas przycisków jako StackLayout (wycentrowany poziomy strip na dole).
    void layoutChildren() override;

private:
    void createTitleBar(std::string_view title);

    Label* m_messageLabel = nullptr;
    Label* m_titleLabel = nullptr;
    std::vector<Button*> m_buttons;

    DialogType m_type;
    DialogCallback m_callback;
    std::string m_message;
    bool m_isOpen = true;
    int m_lastClickedButton = -1;
    
    // Title bar
    bool m_hasTitleBar = false;
    int m_titleBarHeight = 30;
};