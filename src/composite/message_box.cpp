#include "message_box.hpp"

#include "std.hpp"

int MessageBox::calculateWidth(std::string_view message, int minWidth, int maxWidth) {
    // Prosta heurystyka: ~10px per znak
    int estimatedWidth = static_cast<int>(message.length() * 10) + 40;
    return std::clamp(estimatedWidth, minWidth, maxWidth);
}

int MessageBox::calculateHeight(std::string_view message, int minHeight, int maxHeight) {
    // ~20px per linia (zakładamy ~50 znaków per linia)
    int lines = static_cast<int>(message.length() / 50) + 1;
    int estimatedHeight = lines * 20 + 80;  // +80 dla przycisków i padding
    return std::clamp(estimatedHeight, minHeight, maxHeight);
}

GUIElement* MessageBox::showInfo(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback
) {
    int width = calculateWidth(message);
    int height = calculateHeight(message, 100, 200);
    
    auto dialog = DialogBox::createWithTitle(
        manager,
        "Informacja",
        message,
        {"OK"},
        [callback](int) { if (callback) callback(); },
        width, height
    );
    
    // Styl informacyjny - jasny niebieski
    Style style;
    style.backgroundColor = {230, 240, 250, 255};
    style.borderColor = {100, 150, 200, 255};
    style.borderWidth = 2;
    dialog->setStyle(ElementState::Normal, style);
    
    return manager.addElement(std::move(dialog));
}

GUIElement* MessageBox::showError(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback
) {
    int width = calculateWidth(message);
    int height = calculateHeight(message, 100, 200);
    
    auto dialog = DialogBox::createWithTitle(
        manager,
        "Błąd",
        message,
        {"OK"},
        [callback](int) { if (callback) callback(); },
        width, height
    );
    
    // Styl błędu - czerwony
    Style style;
    style.backgroundColor = {255, 235, 235, 255};
    style.borderColor = {200, 100, 100, 255};
    style.borderWidth = 2;
    dialog->setStyle(ElementState::Normal, style);
    
    return manager.addElement(std::move(dialog));
}

GUIElement* MessageBox::showWarning(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback
) {
    int width = calculateWidth(message);
    int height = calculateHeight(message, 100, 200);
    
    auto dialog = DialogBox::createWithTitle(
        manager,
        "Ostrzeżenie",
        message,
        {"OK"},
        [callback](int) { if (callback) callback(); },
        width, height
    );
    
    // Styl ostrzeżenia - żółty/pomarańczowy
    Style style;
    style.backgroundColor = {255, 250, 230, 255};
    style.borderColor = {200, 150, 50, 255};
    style.borderWidth = 2;
    dialog->setStyle(ElementState::Normal, style);
    
    return manager.addElement(std::move(dialog));
}

GUIElement* MessageBox::showQuestion(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> onYes,
    std::function<void()> onNo
) {
    int width = calculateWidth(message, 400, 500);
    int height = calculateHeight(message, 120, 200);
    
    auto dialog = DialogBox::createConfirm(
        manager,
        message,
        "Tak",
        "Nie",
        [onYes, onNo](bool confirmed) {
            if (confirmed) {
                if (onYes) onYes();
            } else {
                if (onNo) onNo();
            }
        },
        width, height
    );
    
    // Styl pytania - neutralny
    dialog->setTitle("Pytanie");
    
    return manager.addElement(std::move(dialog));
}

GUIElement* MessageBox::showCustom(
    GUIManager& manager,
    std::string_view title,
    std::string_view message,
    std::string_view buttonText,
    IconType icon,
    std::function<void()> callback
) {
    int width = calculateWidth(message);
    int height = calculateHeight(message);
    
    auto dialog = DialogBox::createWithTitle(
        manager,
        title,
        message,
        {std::string(buttonText)},
        [callback](int) { if (callback) callback(); },
        width, height
    );
    
    // TODO: Dodać obsługę ikon w przyszłości
    (void)icon;  // Placeholder
    
    return manager.addElement(std::move(dialog));
}