# Calculator Example

## Overview

The calculator example demonstrates how to build a functional calculator application using the SDL2 GUI Library. It showcases the use of buttons, labels, panels, and event callbacks to create an interactive desktop calculator.

## Features

- Basic arithmetic operations: addition, subtraction, multiplication, and division
- Decimal point support
- Clear and backspace functionality
- Sign toggle (+/-)
- Auto-aligned, right-justified display values
- Double-width zero key for quick entry
- Visual feedback with hover and pressed states
- iOS-inspired color scheme

## Architecture

The calculator is built using several key components:

### Calculator Class

A self-contained calculator logic class that manages:
- Current and stored values
- Active operation (+, -, *, /)
- Display state
- Number formatting and error handling

### GUI Components

1. **Display Panel**: A dark panel that contains the calculator's display
2. **Display Label**: Shows the current value or result
3. **Button Grid**: A 4x5 layout of buttons for digits and operations with a double-width zero key on the bottom row

## Layout

The calculator uses a grid layout with the following structure:

```
┌────────────────────────────┐
│       Display Panel        │
│        (360x80)           │
└────────────────────────────┘
┌──────┬──────┬──────┬──────┐
│  C   │ +/-  │  ←   │  /   │
├──────┼──────┼──────┼──────┤
│  7   │  8   │  9   │  *   │
├──────┼──────┼──────┼──────┤
│  4   │  5   │  6   │  -   │
├──────┼──────┼──────┼──────┤
│  1   │  2   │  3   │  +   │
├──────┼──────┼──────┼──────┤
│    0       │  .   │  =   │
└────────────┴──────┴──────┘
```

The bottom row features a double-width zero key to mirror common handheld calculators.

### Button Types and Colors

- **Digit buttons (0-9)**: Dark gray (#464646)
- **Operator buttons (+, -, *, /)**: Orange (#FF9500)
- **Special buttons (C, +/-, ←)**: Light gray (#A0A0A0)
- **Equals button (=)**: Green (#4CD964)

## Code Structure

### Main Components

```cpp
// Calculator logic
class Calculator {
    // Handles all calculation operations
    // Manages display state
    // Formats output
};

// GUI setup
int main() {
    // Initialize SDL and create window
    SDLApp app("Calculator", 400, 600);
    GUIManager guiManager(app.getRenderer());
    
    // Create calculator instance
    Calculator calc;
    
    // Create display panel and label
    // Create button grid with callbacks
    
    // Main event loop
}
```

### Button Callback Pattern

Each button uses a lambda callback that:
1. Calls the appropriate calculator method
2. Updates the display label with the new value

```cpp
button->setOnClickCallback([callback, displayLabelPtr, &calc](GUIElement*) {
    callback();
    displayLabelPtr->setText(calc.getDisplay());
});
```

## Building and Running

### Compilation

```bash
cd /path/to/sdl_gui_library
make output/example_calculator
```

### Execution

```bash
./output/example_calculator
```

## Usage

1. **Number Input**: Click digit buttons (0-9) to enter numbers
2. **Decimal Point**: Click the "." button to add a decimal point
3. **Operations**: Click +, -, *, or / to perform operations
4. **Equals**: Click "=" to calculate the result
5. **Clear**: Click "C" to reset the calculator
6. **Backspace**: Click "←" to delete the last digit
7. **Sign Toggle**: Click "+/-" to change the sign of the current number

## Key Features Demonstrated

### GUI Library Features

- **Panel**: Used for the display background
- **Label**: Displays the current value/result
- **Button**: All calculator buttons with custom styling
- **Event Callbacks**: Each button responds to clicks
- **State Management**: Buttons show visual feedback (normal, hover, pressed)
- **Color Customization**: Custom colors for different button types

### C++ Features

- **Lambda Captures**: Callbacks capture calculator reference and display pointer
- **String Formatting**: Custom number formatting with precision control
- **State Management**: Finite state machine for calculator operations
- **Error Handling**: Division by zero detection

## Customization

You can customize the calculator by modifying:

- **Window size**: Change `SCREEN_WIDTH` and `SCREEN_HEIGHT`
- **Button colors**: Modify the SDL_Color definitions
- **Button size**: Adjust `buttonWidth`, `buttonHeight`, and `padding`
- **Font size**: Change the font size parameter in the Label constructor
- **Number precision**: Modify the `setprecision` value in `updateDisplay()`

## Known Limitations

- No keyboard input support (only mouse clicks)
- Limited to basic arithmetic operations
- No memory functions (M+, M-, MR, MC)
- No scientific calculator features
- Display width is limited (long numbers may overflow)

## Future Enhancements

Possible improvements to the calculator example:

1. Add keyboard support for faster input
2. Implement memory functions
3. Add a history panel showing previous calculations
4. Support for more advanced operations (square root, percentage, etc.)
5. Keyboard shortcuts for operations
6. Scientific calculator mode with additional functions
7. Resize support with responsive layout

## Related Examples

- `example_button.cpp`: Basic button usage
- `example_panel.cpp`: Panel containers
- `example_label.cpp`: Text display
- `example_window.cpp`: Window management

## File Location

- **Source**: `examples/example_calculator.cpp`
- **Documentation**: `docs/calculator_example.md`
