#pragma once

#include "../gui.hpp"
#include "../gui_manager.hpp"
#include "dialog_box.hpp"

#include "std.hpp"

/**
 * @file message_box.hpp
 * @brief MessageBox - statyczna klasa pomocnicza dla szybkich alertów
 * 
 * MessageBox dostarcza proste, statyczne metody do pokazywania komunikatów
 * bez konieczności tworzenia pełnego DialogBox.
 * 
 * Przykład użycia:
 * @code
 * MessageBox::showInfo(manager, "Plik został zapisany.");
 * MessageBox::showError(manager, "Błąd: Nie można otworzyć pliku.");
 * MessageBox::showWarning(manager, "Czy chcesz kontynuować?", []() { // akcja });
 * @endcode
 */

class MessageBox {
public:
    /// Typy komunikatów
    enum class IconType {
        None,
        Info,
        Warning,
        Error,
        Question
    };

    /**
     * @brief Pokazuje komunikat informacyjny
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param callback Callback po kliknięciu OK (opcjonalny)
     * @return Pointer do dodanego elementu (dla ewentualnej manipulacji)
     */
    static GUIElement* showInfo(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Pokazuje komunikat błędu
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param callback Callback po kliknięciu OK (opcjonalny)
     * @return Pointer do dodanego elementu
     */
    static GUIElement* showError(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Pokazuje komunikat ostrzeżenia
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param callback Callback po kliknięciu OK (opcjonalny)
     * @return Pointer do dodanego elementu
     */
    static GUIElement* showWarning(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> callback = nullptr
    );

    /**
     * @brief Pokazuje dialog potwierdzenia
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param onYes Callback gdy użytkownik kliknie "Tak"
     * @param onNo Callback gdy użytkownik kliknie "Nie" (opcjonalny)
     * @return Pointer do dodanego elementu
     */
    static GUIElement* showQuestion(
        GUIManager& manager,
        std::string_view message,
        std::function<void()> onYes,
        std::function<void()> onNo = nullptr
    );

    /**
     * @brief Pokazuje własny komunikat
     * @param manager GUIManager
     * @param title Tytuł okna
     * @param message Treść komunikatu
     * @param buttonText Tekst przycisku
     * @param icon Typ ikony (dla przyszłej implementacji)
     * @param callback Callback po kliknięciu
     * @return Pointer do dodanego elementu
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
    /// Oblicz szerokość dialogu na podstawie tekstu
    static int calculateWidth(std::string_view message, int minWidth = 300, int maxWidth = 600);
    
    /// Oblicz wysokość dialogu na podstawie tekstu
    static int calculateHeight(std::string_view message, int minHeight = 100, int maxHeight = 300);
};