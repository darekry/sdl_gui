/**
 * @file 40_gpu_shader.cpp
 * @brief GPU fragment shader on GUI element via ShaderPanel widget
 *
 * Demonstrates ShaderPanel — a Panel subclass that applies a GPU
 * fragment shader automatically when blitting its cached texture
 * to screen. No manual pipeline management needed.
 *
 * Uses SDLApp GPU constructor + ShaderPanel.
 * Controls: ENTER = toggle shader, ESC = quit.
 */

#include "std.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "shader_panel.hpp"
#include "label.hpp"
#include "style.hpp"
#include "../output/gpu_shader_spirv.hpp"

int main(int, char**) {
    try {
        Uint64 startupStart = SDL_GetTicks();
        SDLApp app("GPU Shader Demo — ENTER: toggle shader, ESC: quit",
                    700, 450, false, GPU_VULKAN);
        LOG_INFO("GpuShader", "[Main] SDLApp GPU init total: {}ms", SDL_GetTicks() - startupStart);

        GUIManager gui(app.getRenderer(), Viewport{700, 450});

        auto info = std::make_unique<Label>(gui, 20, 10,
            "ENTER: toggle shader on right panel   ESC: quit");
        Style is; is.fontSize = 12; is.textColor = SDL_Color{180, 180, 180, 255};
        info->setStyle(ElementState::Normal, is);
        gui.addElement(std::move(info));

        // Left panel — normal Panel
        Style leftS;
        leftS.backgroundColor = SDL_Color{50, 70, 120, 255};
        leftS.borderColor = SDL_Color{100, 130, 200, 255};
        leftS.borderWidth = 2; leftS.borderRadius = 10;
        auto leftP = std::make_unique<Panel>(gui, 30, 50, 250, 180);
        leftP->setStyle(ElementState::Normal, leftS);
        auto lt = std::make_unique<Label>(gui, 15, 15, "Normal Panel\n(default shader)");
        Style lts; lts.fontSize = 14; lts.textColor = SDL_Color{255, 255, 255, 255};
        lt->setStyle(ElementState::Normal, lts);
        leftP->addChild(std::move(lt));
        gui.addElement(std::move(leftP));

        // Right panel — ShaderPanel with desaturate fragment shader
        Style rightS;
        rightS.backgroundColor = SDL_Color{50, 50, 60, 255};
        rightS.borderColor = SDL_Color{120, 120, 140, 255};
        rightS.borderWidth = 2; rightS.borderRadius = 10;
        auto rightP = std::make_unique<ShaderPanel>(gui, 310, 50, 320, 250);
        rightP->setStyle(ElementState::Normal, rightS);
        rightP->setShader(gpu_shader::desaturate_frag, gpu_shader::desaturate_frag_size);
        rightP->setShaderEnabled(true);

        auto rt = std::make_unique<Label>(gui, 15, 15, "ShaderPanel");
        Style rts; rts.fontSize = 14; rts.textColor = SDL_Color{255, 255, 255, 255};
        rt->setStyle(ElementState::Normal, rts);
        rightP->addChild(std::move(rt));
        auto rl = std::make_unique<Label>(gui, 15, 60,
            "Fragment shader is\napplied when blitting\nthe cached texture\nto screen.");
        Style rls; rls.fontSize = 11; rls.textColor = SDL_Color{180, 200, 180, 255};
        rl->setStyle(ElementState::Normal, rls);
        rightP->addChild(std::move(rl));

        ShaderPanel* rightPtr = rightP.get();
        gui.addElement(std::move(rightP));

        auto stat = std::make_unique<Label>(gui, 20, 330, "Shader: ON (press ENTER)");
        Style ss; ss.fontSize = 14; ss.textColor = SDL_Color{220, 220, 220, 255};
        stat->setStyle(ElementState::Normal, ss);
        Label* statPtr = stat.get();
        gui.addElement(std::move(stat));

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                if (e.type == SDL_EVENT_KEY_DOWN) {
                    if (e.key.key == SDLK_RETURN) {
                        bool enabled = !rightPtr->isShaderEnabled();
                        rightPtr->setShaderEnabled(enabled);
                        if (statPtr) statPtr->setText(
                            enabled ? "Shader: ON (press ENTER)" : "Shader: OFF (press ENTER)");
                    }
                    if (e.key.key == SDLK_ESCAPE) quit = true;
                }
                gui.processEvent(e);
            }
            gui.update();

            SDL_SetRenderDrawColor(app.getRenderer(), 20, 20, 30, 255);
            SDL_RenderClear(app.getRenderer());
            gui.render();
            SDL_RenderPresent(app.getRenderer());
        }

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
