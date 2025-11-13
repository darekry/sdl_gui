#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
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
    SDL_Event createMouseButton(Uint32 type, Uint8 button, int x, int y);
    SDL_Event createMouseMotion(int x, int y, Uint32 state = 0);
    SDL_Event createKeyEvent(Uint32 type, SDL_Keycode key);
    SDL_Event createTextInputEvent(const char* text);

    // Backward-compat alias (camelCase name expected by spec)
    SDL_Event createMouseEvent(Uint32 type, Uint8 button, int x, int y);

    // Legacy snake_case (kept for compatibility with existing tests)
    SDL_Event create_mouse_event(Uint32 type, Uint8 button, int x, int y);
    SDL_Event create_key_event(Uint32 type, SDL_Keycode key);
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
