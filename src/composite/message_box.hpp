#pragma once

#include "../gui.hpp"
#include "../gui_manager.hpp"
#include "dialog_box.hpp"

#include "std.hpp"

/**
 * @file message_box.hpp
 * @brief MessageBox - static helper class for quick alerts
 * 
 * MessageBox provides simple, static methods to show messages
 * without the need to create a full DialogBox.
 * 
 * Usage example:
 * @code
 * MessageBox::showInfo(manager, "Plik został zapisany.");
 * MessageBox::showError(manager, "Błąd: Nie można otworzyć pliku.");
 * MessageBox::showWarning(manager, "Czy chcesz kontynuować?", []() { // action });
 * @endcode
 */

class MessageBox {
public:
    /// Message types
    enum class IconType {
        None,
        Info,
        Warning,
        Error,
        Question
    };

    /**
     * @brief Shows an informational message
     * @param manager GUIManager
     * @param message Message content
     * @param callback Callback after OK is clicked (optional)
     * @return Pointer to the added element (for possible manipulation)
     */
    static GUIElement* showInfo(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Shows an error message
     * @param manager GUIManager
     * @param message Message content
     * @param callback Callback after OK is clicked (optional)
     * @return Pointer to the added element
     */
    static GUIElement* showError(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Shows a warning message
     * @param manager GUIManager
     * @param message Message content
     * @param callback Callback after OK is clicked (optional)
     * @return Pointer to the added element
     */
    static GUIElement* showWarning(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Shows a confirmation dialog
     * @param manager GUIManager
     * @param message Message content
     * @param onYes Callback when the user clicks "Yes"
     * @param onNo Callback when the user clicks "No" (optional)
     * @return Pointer to the added element
     */
    static GUIElement* showQuestion(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> onYes,
        std::function<void()> onNo = nullptr
    );

    /**
     * @brief Shows a custom message
     * @param manager GUIManager
     * @param title Window title
     * @param message Message content
     * @param buttonText Button text
     * @param icon Icon type (for future implementation)
     * @param callback Callback after click
     * @return Pointer to the added element
     */
    static GUIElement* showCustom(
        GUIManager& manager,
        std::string_view title,
        std::string_view message,
        std::string_view buttonText = "OK",
        IconType icon = IconType::None,
        std::function<void()> callback = nullptr
    );

private:
    /// Calculates the dialog width based on the text
    static int calculateWidth(std::string_view message, int minWidth = 300, int maxWidth = 600);
    
    /// Calculates the dialog height based on the text
    static int calculateHeight(std::string_view message, int minHeight = 100, int maxHeight = 300);
};