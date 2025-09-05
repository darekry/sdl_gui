#include "test_helper.hpp"
#include <cstring> // std::strncpy
#include "../src/sdl_deleters.hpp"
#include "../src/gui_manager.hpp"
#include "../src/font_manager.hpp"
#include "../src/texture_manager.hpp"

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

    m_guiManager = std::make_unique<GUIManager>(m_renderer);
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

// ---- Accessors ----
GUIManager& TestHelper::getManager() {
    return *m_guiManager;
}

// ---- New unified API implementations ----
SDL_Event TestHelper::createMouseButton(Uint32 type, Uint8 button, int x, int y) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = type;
    e.button.type = type;
    e.button.button = button;
    e.button.state = (type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED);
    e.button.clicks = 1;
    e.button.x = x;
    e.button.y = y;
    return e;
}

SDL_Event TestHelper::createMouseMotion(int x, int y, Uint32 state) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = SDL_MOUSEMOTION;
    e.motion.type = SDL_MOUSEMOTION;
    e.motion.x = x;
    e.motion.y = y;
    e.motion.state = state;
    return e;
}

SDL_Event TestHelper::createKeyEvent(Uint32 type, SDL_Keycode key) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = type;
    e.key.type = type;
    e.key.keysym.sym = key;
    e.key.state = (type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED);
    return e;
}

SDL_Event TestHelper::createTextInputEvent(const char* text) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = SDL_TEXTINPUT;
    if (text) {
        std::strncpy(e.text.text, text, SDL_TEXTINPUTEVENT_TEXT_SIZE - 1);
        e.text.text[SDL_TEXTINPUTEVENT_TEXT_SIZE - 1] = '\0';
    }
    return e;
}

// ---- Backward-compat alias ----
SDL_Event TestHelper::createMouseEvent(Uint32 type, Uint8 button, int x, int y) {
    if (type == SDL_MOUSEMOTION) {
        return createMouseMotion(x, y, 0);
    }
    return createMouseButton(type, button, x, y);
}

// ---- Legacy snake_case wrappers (backward compatibility) ----
SDL_Event TestHelper::create_mouse_event(Uint32 type, Uint8 button, int x, int y) {
    return createMouseEvent(type, button, x, y);
}

SDL_Event TestHelper::create_key_event(Uint32 type, SDL_Keycode key) {
    return createKeyEvent(type, key);
}

SDL_Event TestHelper::create_text_input_event(const char* text) {
    return createTextInputEvent(text);
}

// ---- Stub resources ----
SharedTexture TestHelper::makeStubTexture(int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!m_renderer) return nullptr;
    SDL_Texture* raw = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!raw) {
        return nullptr;
    }
    SDL_SetTextureBlendMode(raw, SDL_BLENDMODE_BLEND);

    // Fill with color to make it visible in tests
    SDL_Texture* prevTarget = SDL_GetRenderTarget(m_renderer);
    SDL_SetRenderTarget(m_renderer, raw);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderClear(m_renderer);
    SDL_SetRenderTarget(m_renderer, prevTarget);

    return SharedTexture(raw, SDLTextureDeleter());
}

void TestHelper::primeTextureAs(std::string_view key, SharedTexture tex) {
    if (!m_guiManager || !tex) return;
    m_guiManager->getTextureManager().addTexture(key, tex);
}