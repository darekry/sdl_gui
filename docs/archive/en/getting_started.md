# Getting Started with SDL2 GUI Library

This guide will walk you through setting up a basic SDL2 GUI project, from installing dependencies to running a simple "Hello World" application with a button.

## 1. System Requirements and Dependencies

The SDL2 GUI Library relies on SDL2 and its extension libraries. You'll need to install these on your system.

### Installation on Debian/Ubuntu-based systems:

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev clang libc++-dev
```

For other operating systems, please refer to the official SDL2 documentation for installation instructions for `SDL2`, `SDL2_image`, and `SDL2_ttf`. You will also need a C++23 compatible compiler (e.g., Clang or GCC).

## 2. Project Structure

For a minimal project, you'll typically have your source files (e.g., `main.cpp`) and link against the SDL2 GUI Library.

```
your_project/
├── main.cpp
└── (link to sdl_gui library and headers)
```

You will need access to the SDL2 GUI Library header files (from the `src/` directory of the library) and the compiled library file (e.g., `libsdl_gui.a` or `libsdl_gui.so`).

<h2>3. Minimal "Hello World" Example</h2>

Here's a simple example that initializes SDL, creates a window, and displays a button. When clicked, the button will print a message to the console.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "label.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize SDL and create a window
    SDLApp app("Hello SDL2 GUI", 800, 600);

    // Create the GUI manager
    GUIManager guiManager(app.getRenderer());

    // Create a button
    auto button = std::make_unique<Button>(300, 250, 200, 50, "Click Me!");
    button->setOnClickCallback([](GUIElement*) {
        std::cout << "Button clicked!" << std::endl;
    });
    guiManager.addElement(std::move(button));

    // Create a label
    auto label = std::make_unique<Label>(300, 150, "Hello, SDL2 GUI!", 32, SDL_Color{0, 0, 0, 255});
    guiManager.addElement(std::move(label));

    // Main application loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Forward events to the GUI
            guiManager.processEvent(e);
        }

        // Safe removal of elements and upkeep
        guiManager.cleanup();

        SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
        SDL_RenderClear(app.getRenderer());
        guiManager.render();
        SDL_RenderPresent(app.getRenderer());
    }

    return 0;
}
```

## 4. Compiling and Linking

To compile and link your application with the SDL2 GUI Library, you need to provide the path to the header files and the library file, along with the necessary SDL2 dependencies.

Assuming you have compiled the SDL2 GUI Library and have `libsdl_gui.a` (static) or `libsdl_gui.so` (dynamic) and the `src/` headers available.

### Static Linking (`.a`)

When linking with the static library (`libsdl_gui.a`), you need to provide the path to the header files and the library file, along with the necessary SDL2 dependencies.

```bash
g++ your_app.cpp -o your_app -I/path/to/sdl_gui/src -L/path/to/sdl_gui/output -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Replace `/path/to/sdl_gui/src` with the actual path to the SDL2 GUI Library's `src` directory, and `/path/to/sdl_gui/output` with the path where `libsdl_gui.a` is located (usually `output/` in the library's root).

<h3>Dynamic Linking (`.so`)</h3>

When linking with the shared library (`libsdl_gui.so`), ensure the linker can find it at runtime.

```bash
g++ your_app.cpp -o your_app -I/path/to/sdl_gui/src -L/path/to/sdl_gui/output -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Make sure the `.so` file is in a directory known to the dynamic linker (e.g., `/usr/local/lib`) or set the `LD_LIBRARY_PATH` environment variable:

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/sdl_gui/output
./your_app
```

Now you should be able to compile and run your first SDL2 GUI application!