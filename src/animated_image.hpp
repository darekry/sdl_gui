#pragma once


#include "gui.hpp"
#include "texture_manager.hpp"
#include "animation_manager.hpp"
#include <SDL3/SDL.h>

import std.compat;
class AnimatedImage : public GUIElement {
public:
    enum class ScaleMode {
        Fit,    // Dopasuj obraz do rozmiaru widgetu (domyślnie)
        Center, // Wycentruj obraz, bez skalowania
        None    // Nie skaluj, rysuj w lewym górnym rogu
    };

    // Konstruktor / destruktor
    AnimatedImage(GUIManager& manager, int x, int y, int width, int height);
    ~AnimatedImage() override;

    // Sprite-sheet configuration
    // frameW / frameH: jeśli 0 -> obliczane automatycznie na podstawie rozmiaru tekstury
    void setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0);

    // Zmiana parametrów animacji
    void setFPS(float fps); // ustawia prędkość w klatkach na sekundę
    void setFrameDuration(float secondsPerFrame); // alternatywa
    void setLoop(bool loop);
    void setUseCache(bool useCache); // true = renderToCache (domyślnie true)
    void setScaleMode(ScaleMode mode);
    void setPreserveAspect(bool preserve);

    // Kontrola odtwarzania
    void play();
    void pause();
    void stop(); // zatrzymaj i zresetuj do klatki 0
    void setFrame(int frameIndex); // natychmiastowa zmiana klatki

    // Gettery
    int getCurrentFrame() const;
    int getTotalFrames() const;
    bool isPlaying() const;

    // Callbacki opcjonalne
    void setOnAnimationEnd(std::function<void()> cb);
    void setOnFrameChanged(std::function<void(int)> cb);
    
    // Animacja przez AnimationManager: animuj wartość frame float -> int
    // duration_ms: czas trwania w milisekundach
    // loop: czy powtarzać po zakończeniu
    void animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false);

    // GUIElement overrides
    void draw(SDL_Renderer* renderer) override;
    // Dla renderowania bez cache implementujemy drawDirect (GUIElement::drawDirect)
    bool wantsDirectRender() const override;
    void drawDirect(SDL_Renderer* renderer) override;
    const char* getComponentType() const override { return "AnimatedImage"; }

private:
    // Helperzy
    void ensureTextureLoaded();
    void recalcFrameGeometry(); // oblicza m_frameW/m_frameH/m_cols na podstawie tekstury i ustawień
    void updateSrcRect(); // oblicza SDL_Rect src dla aktualnej klatki
    void stopFrameAnimation();

    // Zasoby / konfiguracja
    std::string m_texturePath;
    SharedTexture m_texture;
    int m_totalFrames = 0;
    int m_rows = 1;
    int m_cols = 0; // wyliczone
    int m_frameW = 0;
    int m_frameH = 0;

    // Stan odtwarzania
    int m_currentFrame = 0;
    float m_frameDuration = 1.0f / 12.0f; // domyślnie 12 FPS
    bool m_isPlaying = false;
    bool m_loop = true;

    // Render / cache
    bool m_useCache = true;
    ScaleMode m_scaleMode = ScaleMode::Fit;
    bool m_preserveAspect = true;
    SDL_Rect m_srcRect{0,0,0,0};

    // Integracja z AnimationManager
    // AnimationManager animates numeric properties by pointer; tutaj użyjemy m_animFrame (float)
    float m_animFrame = 0.0f; // wewnętrzna właściwość animowana
    // Note: AnimationManager nie zwraca identyfikatora animacji w tej wersji.
    // Będziemy także używać TimerManager do kroków odtwarzania i ticków renderujących,
    // których identyfikatory przechowujemy poniżej.
    
    // Callbacks
    std::function<void()> m_onAnimationEnd;
    std::function<void(int)> m_onFrameChanged;

    // Timer IDs (0 = none). Umożliwiają usuwanie timerów kiedy pauzujemy/stopujemy.
    uint32_t m_playTimerId = 0;
    uint32_t m_animTickTimerId = 0;

    // Wskaźnik do managerów (dostęp przez GUIElement::m_manager)
    // Nie trzymamy dodatknego wskaźnika do AnimationManager/TextureManager — pobieramy z m_manager w locie.
};
