#pragma once

#include "../gui.hpp"
#include "../panel.hpp"
#include "../button.hpp"
#include "../label.hpp"

import std.compat;

/**
 * @file dialog_box.hpp
 * @brief DialogBox - złożony komponent okna dialogowego
 * 
 * DialogBox to gotowy do użycia komponent wyższego poziomu, 
 * który składa się z Panel (tło/ramka), Label (tytuł/przekaz) i Buttonów (akcje).
 * 
 * Przykład użycia:
 * @code
 * auto dialog = DialogBox::createConfirm(manager, "Czy na pewno?", "Tak", "Nie",
 *     [](bool confirmed) { if (confirmed) { // akcja } });
 * manager.addElement(std::move(dialog));
 * @endcode
 */

class DialogBox : public Panel {
public:
    /// Typy okna dialogowego
    enum class DialogType {
        Confirm,    ///< Dialog z dwoma przyciskami (Tak/Nie lub podobne)
        Alert,      ///< Dialog z jednym przyciskiem (OK)
        Custom      ///< Dialog z własnymi przyciskami
    };

    /// Callback wywoływany po kliknięciu przycisku
    using DialogCallback = std::function<void(int buttonIndex)>;

    /**
     * @brief Tworzy dialog potwierdzający (Tak/Nie)
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param yesLabel Tekst przycisku "Tak" (domyślnie "Tak")
     * @param noLabel Tekst przycisku "Nie" (domyślnie "Nie")
     * @param callback Funkcja wywoływana po kliknięciu (true = Tak, false = Nie)
     * @param width Szerokość okna (domyślnie 400)
     * @param height Wysokość okna (domyślnie 150)
     * @return unique_ptr do DialogBox
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
     * @brief Tworzy dialog alertu (OK)
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param okLabel Tekst przycisku (domyślnie "OK")
     * @param callback Funkcja wywoływana po kliknięciu OK
     * @param width Szerokość okna (domyślnie 350)
     * @param height Wysokość okna (domyślnie 120)
     * @return unique_ptr do DialogBox
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
     * @brief Tworzy dialog z własnymi przyciskami
     * @param manager GUIManager
     * @param message Treść komunikatu
     * @param buttonLabels Lista tekstów przycisków
     * @param callback Funkcja wywoływana po kliknięciu (indeks przycisku)
     * @param width Szerokość okna
     * @param height Wysokość okna
     * @return unique_ptr do DialogBox
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
     * @brief Tworzy dialog z tytułem
     * @param manager GUIManager
     * @param title Tytuł okna
     * @param message Treść komunikatu
     * @param buttonLabels Lista tekstów przycisków
     * @param callback Funkcja wywoływana po kliknięciu
     * @param width Szerokość okna
     * @param height Wysokość okna
     * @return unique_ptr do DialogBox
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

    // Konstruktor (używaj statycznych metod create*)
    DialogBox(GUIManager& manager, int x, int y, int width, int height,
              std::string_view message, const std::vector<std::string>& buttonLabels,
              DialogType type, DialogCallback callback = nullptr);

    /// Ustawia treść komunikatu
    void setMessage(std::string_view message);

    /// Ustawia tytuł okna
    void setTitle(std::string_view title);

    /// Zamyka dialog (usuwa element)
    void close();

    /// Czy dialog jest otwarty
    bool isOpen() const { return m_isOpen; }

    /// Zwraca typ dialogu
    DialogType getDialogType() const { return m_type; }

    /// Zwraca indeks klikniętego przycisku (-1 jeśli brak)
    int getLastClickedButton() const { return m_lastClickedButton; }

    const char* getComponentType() const override;

    bool isOverlay() const override { return true; }

protected:
    void draw(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    void layoutButtons();
    void createTitleBar(std::string_view title);

    Label* m_messageLabel = nullptr;
    Label* m_titleLabel = nullptr;
    std::vector<Button*> m_buttons;

    DialogType m_type;
    DialogCallback m_callback;
    std::string m_message;
    bool m_isOpen = true;
    int m_lastClickedButton = -1;
    
    // Tytuł bar
    bool m_hasTitleBar = false;
    int m_titleBarHeight = 30;
};