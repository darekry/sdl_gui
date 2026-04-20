#include "button.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "label.hpp"
#include "panel.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;

class Calculator {
public:
    Calculator() : currentValue(0.0), storedValue(0.0), operation('\0'), clearOnNextInput(false) {}

    void inputDigit(int digit) {
        if (clearOnNextInput) {
            display = "";
            clearOnNextInput = false;
        }
        if (display == "0") {
            display = "";
        }
        display += std::to_string(digit);
        currentValue = std::stod(display);
    }

    void inputDecimal() {
        if (clearOnNextInput) {
            display = "0";
            clearOnNextInput = false;
        }
        if (display.find('.') == std::string::npos) {
            display += ".";
        }
    }

    void setOperation(char op) {
        if (display == "Error") {
            return;
        }

        if (operation != '\0') {
            calculate();
            if (display == "Error") {
                return;
            }
        } else {
            storedValue = currentValue;
        }
        operation = op;
        clearOnNextInput = true;
    }

    void calculate() {
        if (operation == '\0') {
            return;
        }

        switch (operation) {
            case '+':
                storedValue += currentValue;
                break;
            case '-':
                storedValue -= currentValue;
                break;
            case '*':
                storedValue *= currentValue;
                break;
            case '/':
                if (currentValue == 0.0) {
                    setErrorState("Error");
                    return;
                }
                storedValue /= currentValue;
                break;
            default:
                break;
        }

        currentValue = storedValue;
        operation = '\0';
        updateDisplay();
        clearOnNextInput = true;
    }

    void clear() {
        reset();
        updateDisplay();
    }

    void backspace() {
        if (display == "Error") {
            return;
        }
        if (!display.empty() && !clearOnNextInput) {
            display.pop_back();
            if (display.empty() || display == "-") {
                display = "0";
            }
            currentValue = display == "0" ? 0.0 : std::stod(display);
        }
    }

    void toggleSign() {
        if (display == "Error") {
            return;
        }
        if (currentValue != 0.0) {
            currentValue = -currentValue;
            updateDisplay();
        }
    }

    std::string getDisplay() const {
        return display;
    }

private:
    void reset() {
        currentValue = 0.0;
        storedValue = 0.0;
        operation = '\0';
        clearOnNextInput = false;
        display = "0";
    }

    void setErrorState(const std::string& errorMsg) {
        display = errorMsg;
        currentValue = 0.0;
        storedValue = 0.0;
        operation = '\0';
        clearOnNextInput = true;
    }

    void updateDisplay() {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(8) << currentValue;
        display = oss.str();
        
        size_t dotPos = display.find('.');
        if (dotPos != std::string::npos) {
            size_t end = display.find_last_not_of('0');
            if (end != std::string::npos && end >= dotPos) {
                if (end == dotPos) {
                    display = display.substr(0, dotPos);
                } else {
                    display = display.substr(0, end + 1);
                }
            }
        }
    }

    double currentValue;
    double storedValue;
    char operation;
    bool clearOnNextInput;
    std::string display = "0";
};

int main(int, char**) {
    try {
        SDLApp app("Calculator", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        Calculator calc;

        auto displayPanel = std::make_unique<Panel>(guiManager, 20, 20, 360, 80);
        displayPanel->setBackgroundColor(ElementState::Normal, {40, 40, 40, 255});
        auto* displayPanelPtr = displayPanel.get();
        guiManager.addElement(std::move(displayPanel));

        auto displayLabel = std::make_unique<Label>(guiManager, 30, 35, "0", 36);
        displayLabel->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        auto* displayLabelPtr = displayLabel.get();
        displayPanelPtr->addChild(std::move(displayLabel));

        const int buttonWidth = 80;
        const int buttonHeight = 70;
        const int padding = 10;
        const int startX = 20;
        const int startY = 120;

        auto updateDisplayLabel = [&]() {
            displayLabelPtr->setText(calc.getDisplay());
            const int displayPadding = 15;
            const int panelWidth = displayPanelPtr->getWidth();
            const int panelHeight = displayPanelPtr->getHeight();
            const int labelWidth = displayLabelPtr->getWidth();
            const int labelHeight = displayLabelPtr->getHeight();
            const int x = std::max(displayPadding, panelWidth - labelWidth - displayPadding);
            const int y = std::max(displayPadding, panelHeight - labelHeight - displayPadding);
            displayLabelPtr->setPosition(x, y);
        };
        updateDisplayLabel();

        auto createButton = [&](int col, int row, const std::string& label, SDL_Color normalColor, SDL_Color hoverColor, SDL_Color pressedColor, std::function<void()> callback, int colSpan = 1) {
            int x = startX + col * (buttonWidth + padding);
            int y = startY + row * (buttonHeight + padding);
            int width = buttonWidth * colSpan + padding * (colSpan - 1);

            auto button = std::make_unique<Button>(guiManager, x, y, width, buttonHeight, label);
            button->setBackgroundColor(ElementState::Normal, normalColor);
            button->setBackgroundColor(ElementState::Hover, hoverColor);
            button->setBackgroundColor(ElementState::Pressed, pressedColor);
            button->setTextColor(ElementState::Normal, {255, 255, 255, 255});
            button->setOnClickCallback([callback, &updateDisplayLabel](GUIElement*) {
                callback();
                updateDisplayLabel();
            });
            guiManager.addElement(std::move(button));
        };

        SDL_Color digitColor = {70, 70, 70, 255};
        SDL_Color digitHover = {90, 90, 90, 255};
        SDL_Color digitPressed = {50, 50, 50, 255};

        SDL_Color operatorColor = {255, 149, 0, 255};
        SDL_Color operatorHover = {255, 179, 64, 255};
        SDL_Color operatorPressed = {200, 120, 0, 255};

        SDL_Color specialColor = {160, 160, 160, 255};
        SDL_Color specialHover = {180, 180, 180, 255};
        SDL_Color specialPressed = {140, 140, 140, 255};

        SDL_Color equalsColor = {76, 217, 100, 255};
        SDL_Color equalsHover = {106, 237, 130, 255};
        SDL_Color equalsPressed = {56, 180, 74, 255};

        createButton(0, 0, "C", specialColor, specialHover, specialPressed, [&calc]() { calc.clear(); });
        createButton(1, 0, "+/-", specialColor, specialHover, specialPressed, [&calc]() { calc.toggleSign(); });
        createButton(2, 0, "←", specialColor, specialHover, specialPressed, [&calc]() { calc.backspace(); });
        createButton(3, 0, "/", operatorColor, operatorHover, operatorPressed, [&calc]() { calc.setOperation('/'); });

        createButton(0, 1, "7", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(7); });
        createButton(1, 1, "8", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(8); });
        createButton(2, 1, "9", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(9); });
        createButton(3, 1, "*", operatorColor, operatorHover, operatorPressed, [&calc]() { calc.setOperation('*'); });

        createButton(0, 2, "4", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(4); });
        createButton(1, 2, "5", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(5); });
        createButton(2, 2, "6", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(6); });
        createButton(3, 2, "-", operatorColor, operatorHover, operatorPressed, [&calc]() { calc.setOperation('-'); });

        createButton(0, 3, "1", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(1); });
        createButton(1, 3, "2", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(2); });
        createButton(2, 3, "3", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(3); });
        createButton(3, 3, "+", operatorColor, operatorHover, operatorPressed, [&calc]() { calc.setOperation('+'); });

        createButton(0, 4, "0", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDigit(0); }, 2);
        createButton(2, 4, ".", digitColor, digitHover, digitPressed, [&calc]() { calc.inputDecimal(); });
        createButton(3, 4, "=", equalsColor, equalsHover, equalsPressed, [&calc]() { calc.calculate(); });

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
