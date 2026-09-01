#include "dialog_box.hpp"
#include "../gui_manager.hpp"

#include "std.hpp"

// ============================================================================
// Static factory methods
// ============================================================================

std::unique_ptr<DialogBox> DialogBox::createConfirm(
    GUIManager& manager,
    std::string_view message,
    std::string_view yesLabel,
    std::string_view noLabel,
    std::function<void(bool confirmed)> callback,
    int width,
    int height
) {
    constexpr int kScreenW = 800;  // TODO: Get actual screen dimensions from GUIManager
    constexpr int kScreenH = 600;
    auto [x, y] = CenterRect(kScreenW, kScreenH, width, height);

    auto dialog = std::unique_ptr<DialogBox>(new DialogBox(
        manager, x, y, width, height,
        message, 
        std::vector<std::string>{std::string(yesLabel), std::string(noLabel)},
        DialogType::Confirm,
        nullptr
    ));

    // Convert callback from bool to int index
    if (callback) {
        dialog->m_callback = [callback](int buttonIndex) {
            callback(buttonIndex == 0);  // 0 = Yes, 1 = No
        };
    }

    return dialog;
}

std::unique_ptr<DialogBox> DialogBox::createAlert(
    GUIManager& manager,
    std::string_view message,
    std::string_view okLabel,
    DialogCallback callback,
    int width,
    int height
) {
    constexpr int kScreenW = 800;  // TODO: Get actual screen dimensions from GUIManager
    constexpr int kScreenH = 600;
    auto [x, y] = CenterRect(kScreenW, kScreenH, width, height);

    auto dialog = std::unique_ptr<DialogBox>(new DialogBox(
        manager, x, y, width, height,
        message,
        std::vector<std::string>{std::string(okLabel)},
        DialogType::Alert,
        callback
    ));

    return dialog;
}

std::unique_ptr<DialogBox> DialogBox::createCustom(
    GUIManager& manager,
    std::string_view message,
    const std::vector<std::string>& buttonLabels,
    DialogCallback callback,
    int width,
    int height
) {
    constexpr int kScreenW = 800;  // TODO: Get actual screen dimensions from GUIManager
    constexpr int kScreenH = 600;
    auto [x, y] = CenterRect(kScreenW, kScreenH, width, height);

    return std::unique_ptr<DialogBox>(new DialogBox(
        manager, x, y, width, height,
        message, buttonLabels,
        DialogType::Custom,
        callback
    ));
}

std::unique_ptr<DialogBox> DialogBox::createWithTitle(
    GUIManager& manager,
    std::string_view title,
    std::string_view message,
    const std::vector<std::string>& buttonLabels,
    DialogCallback callback,
    int width,
    int height
) {
    constexpr int kScreenW = 800;  // TODO: Get actual screen dimensions from GUIManager
    constexpr int kScreenH = 600;
    auto [x, y] = CenterRect(kScreenW, kScreenH, width, height);

    auto dialog = std::unique_ptr<DialogBox>(new DialogBox(
        manager, x, y, width, height,
        message, buttonLabels,
        DialogType::Custom,
        callback
    ));

    dialog->createTitleBar(title);
    return dialog;
}

// ============================================================================
// Constructor
// ============================================================================

DialogBox::DialogBox(
    GUIManager& manager, int x, int y, int width, int height,
    std::string_view message,
    const std::vector<std::string>& buttonLabels,
    DialogType type,
    DialogCallback callback
)
    : Panel(manager, x, y, width, height)
    , m_type(type)
    , m_callback(callback)
    , m_message(message)
{
    // Dialog style - Windows-like
    Style dialogStyle;
    dialogStyle.backgroundColor = {240, 240, 240, 255};  // Light gray
    dialogStyle.borderColor = {100, 100, 100, 255};
    dialogStyle.borderWidth = 2;
    dialogStyle.borderRadius = 0;
    setStyle(ElementState::Normal, dialogStyle);
    
    setClipChildren(false);  // Don't clip children
    
    // Centered on screen - draggable
    setDraggable(true);

    // Create the message label
    int messageY = m_hasTitleBar ? m_titleBarHeight + 10 : 20;
    auto messageLabel = std::make_unique<Label>(manager, 10, messageY, message);
    m_messageLabel = messageLabel.get();
    addChild(std::move(messageLabel));

    // Create the buttons
    int buttonHeight = 35;
    int buttonSpacing = 10;
    int totalButtonsWidth = 0;
    std::vector<int> buttonWidths;

    // Calculate button widths
    for (const auto& label : buttonLabels) {
        int btnWidth = std::max(80, static_cast<int>(label.length() * 12 + 20));
        buttonWidths.push_back(btnWidth);
        totalButtonsWidth += btnWidth;
    }
    totalButtonsWidth += buttonSpacing * static_cast<int>(buttonLabels.size() - 1);

    // Button position
    int startX = (width - totalButtonsWidth) / 2;
    int buttonY = height - buttonHeight - 15;

    for (size_t i = 0; i < buttonLabels.size(); ++i) {
        auto button = std::make_unique<Button>(
            manager, startX, buttonY, buttonWidths[i], buttonHeight, buttonLabels[i]
        );
        
        // Button style
        Style btnStyle;
        btnStyle.backgroundColor = {220, 220, 220, 255};
        btnStyle.borderColor = {150, 150, 150, 255};
        btnStyle.borderWidth = 1;
        btnStyle.borderRadius = 3;
        button->setStyle(ElementState::Normal, btnStyle);
        
        Style hoverStyle;
        hoverStyle.backgroundColor = {230, 230, 230, 255};
        hoverStyle.borderColor = {100, 100, 100, 255};
        button->setStyle(ElementState::Hover, hoverStyle);
        
        Style pressedStyle;
        pressedStyle.backgroundColor = {200, 200, 200, 255};
        button->setStyle(ElementState::Pressed, pressedStyle);

        // Button callback
        int buttonIndex = static_cast<int>(i);
        button->setOnClickCallback([this, buttonIndex](GUIElement*) {
            m_lastClickedButton = buttonIndex;
            if (m_callback) {
                m_callback(buttonIndex);
            }
            close();
        });

        m_buttons.push_back(button.get());
        addChild(std::move(button));

        startX += buttonWidths[i] + buttonSpacing;
    }
}

// ============================================================================
// Method implementations
// ============================================================================

void DialogBox::createTitleBar(std::string_view title) {
    m_hasTitleBar = true;
    
    // Title label
    auto titleLabel = std::make_unique<Label>(m_manager, 10, 5, title);
    m_titleLabel = titleLabel.get();
    
    // Title style
    Style titleStyle;
    titleStyle.textColor = {0, 0, 0, 255};
    titleLabel->setStyle(ElementState::Normal, titleStyle);
    
    addChild(std::move(titleLabel));
    
    // Move the message label
    m_messageLabel->setPosition(10, m_titleBarHeight + 10);
}

void DialogBox::setMessage(std::string_view message) {
    m_message = message;
    m_messageLabel->setText(message);
    markDirty();
}

void DialogBox::setTitle(std::string_view title) {
    if (!m_hasTitleBar) {
        createTitleBar(title);
    } else {
        m_titleLabel->setText(title);
    }
    markDirty();
}

void DialogBox::close() {
    m_isOpen = false;
    markForDeletion();
}

void DialogBox::layoutButtons() {
    if (m_buttons.empty()) return;
    
    int buttonHeight = 35;
    int buttonSpacing = 10;
    int totalButtonsWidth = 0;
    
    for (auto* btn : m_buttons) {
        totalButtonsWidth += btn->getWidth();
    }
    totalButtonsWidth += buttonSpacing * static_cast<int>(m_buttons.size() - 1);
    
    int startX = (m_width - totalButtonsWidth) / 2;
    int buttonY = m_height - buttonHeight - 15;
    
    for (auto* btn : m_buttons) {
        btn->setPosition(startX, buttonY);
        startX += btn->getWidth() + buttonSpacing;
    }
}

void DialogBox::draw(SDL_Renderer* renderer) {
    if (m_hasTitleBar) {
        drawTitleBar(renderer, m_x, m_y, m_width, m_titleBarHeight);
    }
    
    // Draw the panel (background and border)
    Panel::draw(renderer);
}

bool DialogBox::handleEvent(const SDL_Event& e) {
    if (!m_isOpen || !m_visible) return false;
    
    // Handle children (buttons, label)
    if (Panel::handleEvent(e)) return true;
    
    // ESC closes the dialog
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
        m_lastClickedButton = -1;  // Cancelled
        if (m_callback) {
            m_callback(-1);
        }
        close();
        return true;
    }
    
    return false;
}

const char* DialogBox::getComponentType() const {
    return "DialogBox";
}