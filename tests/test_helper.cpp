#include "test_helper.hpp"
#include "../src/font_manager.hpp"
#include "../src/texture_manager.hpp"
#include "../src/gui_manager.hpp"
#include <memory>

TestHelper::TestHelper() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
    }
    if (TTF_Init() == -1) {
        throw std::runtime_error("SDL_ttf could not initialize! SDL_ttf Error: " + std::string(TTF_GetError()));
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        throw std::runtime_error("SDL_image could not initialize! SDL_image Error: " + std::string(IMG_GetError()));
    }

    m_window = SDL_CreateWindow("Test Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_HIDDEN);
    if (!m_window) {
        throw std::runtime_error("Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        throw std::runtime_error("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()));
    }

    m_fontManager = std::make_unique<FontManager>();
    m_textureManager = std::make_unique<TextureManager>(m_renderer);
    m_guiManager = std::make_unique<GUIManager>(m_renderer, m_fontManager.get(), m_textureManager.get());
}

TestHelper::~TestHelper() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

SDL_Event TestHelper::create_mouse_event(Uint32 type, Uint8 button, int x, int y) {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event)); // Użyj SDL_memset do poprawnego wyzerowania
    event.button.type = type;
    event.button.timestamp = SDL_GetTicks();
    event.button.windowID = SDL_GetWindowID(m_window);
    event.button.which = 0; // Użyj standardowego ID myszy
    event.button.button = button;
    event.button.state = SDL_PRESSED;
    event.button.clicks = 1;
    event.button.x = x;
    event.button.y = y;
    event.type = type;
    return event;
}

SDL_Event TestHelper::create_key_event(Uint32 type, SDL_Keycode key) {
    SDL_Event event;
    event.type = type;
    event.key.keysym.sym = key;
    return event;
}
SDL_Event TestHelper::create_text_input_event(const char* text) {
    SDL_Event event;
    event.type = SDL_TEXTINPUT;
    strncpy(event.text.text, text, sizeof(event.text.text));
    return event;
}