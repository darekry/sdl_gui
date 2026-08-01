/**
 * @file 45_gpu_shader_animation.cpp
 * @brief Animated GPU fragment shaders reacting to time and mouse position
 *
 * Left panel: water shader animated by time, driven by AnimationManager.
 * Right panel: glow following the cursor, driven by mouse events.
 *
 * Per-frame data (time, mouse) is passed through SDL_Vertex colors
 * (fragment input location 0), no per-frame hook needed.
 * ESC: quit.
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
        SDLApp app("GPU Shader Animation — ESC: quit", 800, 560, false, GPU_VULKAN);
        LOG_INFO("ShaderAnim", "GPU driver: {}", SDL_GetGPUDeviceDriver(app.getGPUDevice()));

        GUIManager gui(app.getRenderer());

        auto info = std::make_unique<Label>(gui, 20, 10,
            "Left: time-driven water (AnimationManager)   Right: mouse glow   ESC: quit");
        Style is; is.fontSize = 12; is.textColor = SDL_Color{180, 180, 180, 255};
        info->setStyle(ElementState::Normal, is);
        gui.addElement(std::move(info));

        auto titleL = std::make_unique<Label>(gui, 30, 45, "Time (AnimationManager)");
        Style ts; ts.fontSize = 14; ts.textColor = SDL_Color{220, 220, 220, 255};
        titleL->setStyle(ElementState::Normal, ts);
        gui.addElement(std::move(titleL));

        auto titleR = std::make_unique<Label>(gui, 430, 45, "Mouse position");
        titleR->setStyle(ElementState::Normal, ts);
        gui.addElement(std::move(titleR));

        Style panelS;
        panelS.backgroundColor = SDL_Color{25, 25, 35, 255};
        panelS.borderColor = SDL_Color{100, 120, 160, 255};
        panelS.borderWidth = 2;
        panelS.borderRadius = 10;

        auto leftP = std::make_unique<ShaderPanel>(gui, 30, 75, 350, 350);
        leftP->setStyle(ElementState::Normal, panelS);
        leftP->setShader(gpu_shader::time_water_frag, gpu_shader::time_water_frag_size);
        ShaderPanel* leftPtr = leftP.get();
        gui.addElement(std::move(leftP));

        auto rightP = std::make_unique<ShaderPanel>(gui, 420, 75, 350, 350);
        rightP->setStyle(ElementState::Normal, panelS);
        rightP->setShader(gpu_shader::mouse_glow_frag, gpu_shader::mouse_glow_frag_size);
        ShaderPanel* rightPtr = rightP.get();
        gui.addElement(std::move(rightP));

        float time = 0.0f;
        gui.getAnimationManager()->addAnimation(16, [&time, leftPtr]() {
            time += 0.016f;
            if (leftPtr) leftPtr->setUniformTime(time);
        });

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) quit = true;
                if (e.type == SDL_EVENT_MOUSE_MOTION) {
                    float mx = (static_cast<float>(e.motion.x) - 420.0f) / 350.0f;
                    float my = (static_cast<float>(e.motion.y) - 75.0f) / 350.0f;
                    if (rightPtr) rightPtr->setUniformMouse(mx, my);
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
