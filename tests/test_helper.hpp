#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdexcept>
#include <string>

class TestHelper {
public:
    TestHelper();
    ~TestHelper();

    SDL_Renderer* getRenderer() const { return m_renderer; }
    SDL_Window* getWindow() const { return m_window; }

    SDL_Event create_mouse_event(Uint32 type, Uint8 button, int x, int y);
    SDL_Event create_key_event(Uint32 type, SDL_Keycode key);
SDL_Event create_text_input_event(const char* text);

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};

#endif // TEST_HELPER_HPP
