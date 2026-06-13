#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <memory>

#include "../src/texture_manager.hpp" // SharedTexture alias

// Forward declarations (keep pointers compatible)
class GUIManager;
class FontManager;
class TextureManager;

class TestHelper {
public:
    TestHelper();
    ~TestHelper();

    // Accessors
    SDL_Renderer* getRenderer() const { return m_renderer; }
    SDL_Window* getWindow() const { return m_window; }
    GUIManager* getGUIManager() const { return m_guiManager.get(); }
    FontManager* getFontManager() const { return m_fontManager.get(); }
    TextureManager* getTextureManager() const { return m_textureManager.get(); }

    // New unified API (camelCase)
    GUIManager& getManager();

    // Mouse/keyboard/text events
    SDL_Event createMouseButton(SDL_EventType type, Uint8 button, int x, int y);
    SDL_Event createMouseMotion(int x, int y, Uint32 state = 0);
    SDL_Event createMouseWheel(Sint32 y, Sint32 x = 0);
    SDL_Event createKeyEvent(SDL_EventType type, SDL_Keycode key);
    SDL_Event createKeyEvent(SDL_EventType type, SDL_Keycode key, Uint16 mod);
    SDL_Event createTextInputEvent(const char* text);

    SDL_Event createMouseEvent(SDL_EventType type, Uint8 button, int x, int y);

    SDL_Event create_mouse_event(SDL_EventType type, Uint8 button, int x, int y);
    SDL_Event create_key_event(SDL_EventType type, SDL_Keycode key);
    SDL_Event create_text_input_event(const char* text);

    // Stub resources for AnimatedImage/tests
    SharedTexture makeStubTexture(int w, int h, Uint8 r=0, Uint8 g=0, Uint8 b=0, Uint8 a=255);
    void primeTextureAs(std::string_view key, SharedTexture tex);

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    std::unique_ptr<FontManager> m_fontManager;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<GUIManager> m_guiManager;
};

#endif // TEST_HELPER_HPP
