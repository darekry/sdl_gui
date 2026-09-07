/**
 * @file 63_mixer_beeps.cpp
 * @brief Third-party: SDL3_mixer plays procedural beeps (3rd-party example).
 *
 * Three sine tones (C/E/G) are synthesized into in-memory WAVs and loaded
 * with MIX_LoadAudio_IO() - no audio files needed. Buttons or keys 1/2/3
 * trigger MIX_PlayAudio(); a slider drives MIX_SetMixerGain().
 * Link flags come from `pkg-config sdl3-mixer` (see the mixer hook in nob.c).
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"
#include <SDL3_mixer/SDL_mixer.h>

namespace {

// Minimal 16-bit mono WAV with exponential decay envelope.
std::vector<Uint8> makeToneWav(float freqHz, float seconds) {
    const int rate = 22050;
    const int n = static_cast<int>(rate * seconds);
    std::vector<Uint8> wav;
    wav.reserve(44 + static_cast<size_t>(n) * 2);
    auto push32 = [&](uint32_t v) {
        for (int i = 0; i < 4; ++i) wav.push_back(static_cast<Uint8>(v >> (8 * i)));
    };
    auto push16 = [&](uint16_t v) {
        wav.push_back(static_cast<Uint8>(v & 0xFF));
        wav.push_back(static_cast<Uint8>(v >> 8));
    };
    wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
    push32(static_cast<uint32_t>(36 + n * 2));
    wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    push32(16);
    push16(1);
    push16(1);
    push32(static_cast<uint32_t>(rate));
    push32(static_cast<uint32_t>(rate * 2));
    push16(2);
    push16(16);
    wav.insert(wav.end(), {'d', 'a', 't', 'a'});
    push32(static_cast<uint32_t>(n * 2));
    for (int i = 0; i < n; ++i) {
        float t = i / static_cast<float>(rate);
        float env = std::exp(-3.0f * t / seconds);
        int16_t s = static_cast<int16_t>(28000.0f * env * std::sin(6.2831853f * freqHz * t));
        push16(static_cast<uint16_t>(s));
    }
    return wav;
}

} // namespace

int main(int, char**) {
    int rc = 0;
    MIX_Mixer* mixer = nullptr;
    bool mixInit = false;
    try {
        SDLApp app("Mixer Beeps - SDL3_mixer + procedural WAV", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto status = std::make_unique<Label>(guiManager, 30, 25, "", 17);
        auto statusRef = guiManager.makeRef(status.get());
        guiManager.addElement(std::move(status));

        if (!MIX_Init()) throw std::runtime_error("MIX_Init failed");
        mixInit = true;
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (!mixer) throw std::runtime_error(std::string("no mixer: ") + SDL_GetError());

        // Sample bytes must outlive the MIX_Audio objects (predecode=false).
        std::vector<std::vector<Uint8>> samples;
        auto loadTone = [&](float freq) -> MIX_Audio* {
            samples.push_back(makeToneWav(freq, 0.25f));
            SDL_IOStream* io = SDL_IOFromConstMem(samples.back().data(), samples.back().size());
            if (!io) return nullptr;
            return MIX_LoadAudio_IO(mixer, io, false, true);
        };
        MIX_Audio* toneC = loadTone(261.63f);
        MIX_Audio* toneE = loadTone(329.63f);
        MIX_Audio* toneG = loadTone(392.00f);
        if (!toneC || !toneE || !toneG) throw std::runtime_error("MIX_LoadAudio_IO failed");

        auto play = [&](MIX_Audio* a, const char* name) {
            if (MIX_PlayAudio(mixer, a)) {
                if (statusRef) statusRef->setText(std::string("playing: ") + name);
            } else if (statusRef) {
                statusRef->setText("play failed");
            }
        };

        const char* names[3] = {"C (261 Hz)", "E (329 Hz)", "G (392 Hz)"};
        MIX_Audio* tones[3] = {toneC, toneE, toneG};
        for (int i = 0; i < 3; ++i) {
            auto btn = std::make_unique<Button>(guiManager, 30 + i * 170, 120, 150, 60, names[i]);
            btn->setOnClickCallback([play, tones, i](GUIElement*) { play(tones[i], "tone"); });
            guiManager.addElement(std::move(btn));
        }

        auto volSlider = std::make_unique<Slider>(guiManager, 30, 240, 480, 36,
                                                  0, 100, 80, Orientation::Horizontal);
        volSlider->setTooltip("Mixer gain");
        volSlider->setOnChangeCallback([mixer](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s) MIX_SetMixerGain(mixer, s->getValue() / 100.0f);
        });
        guiManager.addElement(std::move(volSlider));
        MIX_SetMixerGain(mixer, 0.8f);

        auto info = std::make_unique<Label>(guiManager, 30, 290,
            "keys 1/2/3 also play - WAVs are synthesized, no audio files shipped", 16);
        guiManager.addElement(std::move(info));
        if (statusRef) statusRef->setText("SDL3_mixer ready");

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                } else if (e.type == SDL_EVENT_KEY_DOWN) {
                    if (e.key.key == SDLK_1) play(toneC, "C");
                    if (e.key.key == SDLK_2) play(toneE, "E");
                    if (e.key.key == SDLK_3) play(toneG, "G");
                }
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
        // Tear down the mixer BEFORE SDLApp dies (its dtor calls SDL_Quit,
        // after which MIX_DestroyMixer touches a dead audio subsystem).
        MIX_DestroyMixer(mixer);
        mixer = nullptr;
        MIX_Quit();
        mixInit = false;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        rc = 1;
    }
    if (mixer) MIX_DestroyMixer(mixer);
    if (mixInit) MIX_Quit();
    return rc;
}
