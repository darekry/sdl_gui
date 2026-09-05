#pragma once

#include "text_editable.hpp"
#include "style.hpp"
#include "sdl_deleters.hpp"

class TextInput : public TextEditable {
public:
    TextInput(GUIManager& manager, int x, int y, int w, int h);
    ~TextInput() = default;

    // TextInput-specific callbacks
    void setOnTextChanged(const std::function<void(TextInput*)>& callback);
    void setOnEnterPressed(const std::function<void(TextInput*)>& callback);
    
    // Locked state (override shared TextEditable implementation)
    void setLocked(bool locked) override;
    bool isLocked() const override;

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;
    void renderOverlay(SDL_Renderer* renderer) override;

protected:
    void draw(SDL_Renderer* renderer) override;
    
    // Override TextEditable virtual methods
    void updateTextOffset() override;
    void refreshTextTexture() override;
    void markNeedsUpdate() override;

private:
    SharedTexture m_textTexture;
    int m_text_offset_x = 0;
    
    std::function<void(TextInput*)> m_onTextChanged;
    std::function<void(TextInput*)> m_onEnterPressed;
};