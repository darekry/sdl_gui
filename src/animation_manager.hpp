#pragma once

#include <SDL2/SDL.h>
#include <functional>
#include <vector>
#include <variant>
#include <iostream>
#include <numeric>
#include <cmath>
#include "easing.hpp"

struct Animation {
    using CompleteCallback = std::function<void()>;
    std::variant<int*, float*> target_property;
    float start_value = 0.0f;
    float end_value = 0.0f;
    uint64_t start_time = 0;
    uint32_t duration_ms = 0;
    std::function<float(float)> easing_function = Easing::linear;
    CompleteCallback on_complete_callback = nullptr;
    bool is_finished = false;
};

class AnimationManager {
public:
    AnimationManager() = default;

    template <typename T>
    void createAnimation(
        T* target_property,
        float start_value,
        float end_value,
        uint32_t duration,
        std::function<float(float)> easing = Easing::linear,
        Animation::CompleteCallback on_complete = nullptr
    ) {
        if (!target_property) {
            std::cerr << "AnimationManager Error: target_property cannot be null." << std::endl;
            return;
        }
        *target_property = static_cast<T>(start_value);
        active_animations.emplace_back(
            Animation{
                .target_property = target_property,
                .start_value = start_value,
                .end_value = end_value,
                .start_time = SDL_GetTicks64(),
                .duration_ms = duration,
                .easing_function = easing,
                .on_complete_callback = on_complete,
                .is_finished = false
            }
        );
    }

    void update() {
        if (active_animations.empty()) {
            return;
        }
        Uint64 current_time = SDL_GetTicks64();
        std::vector<Animation::CompleteCallback> completed_callbacks;
        for (auto& anim : active_animations) {
            if (anim.is_finished) { continue; }
            float raw_progress = static_cast<float>(current_time - anim.start_time) / static_cast<float>(anim.duration_ms);
            if (raw_progress >= 1.0f) {
                raw_progress = 1.0f;
                anim.is_finished = true;
            }
            float eased_progress = anim.easing_function(raw_progress);
            float current_value = anim.start_value + eased_progress * (anim.end_value - anim.start_value);
            std::visit([current_value](auto* ptr) {
                *ptr = static_cast<std::remove_pointer_t<decltype(ptr)>>(current_value);
            }, anim.target_property);
            if (anim.is_finished && anim.on_complete_callback) {
                completed_callbacks.push_back(anim.on_complete_callback);
            }
        }
        active_animations.erase(
            std::remove_if(active_animations.begin(), active_animations.end(),
                           [](const auto& anim) { return anim.is_finished; }),
            active_animations.end()
        );
        for (const auto& callback : completed_callbacks) {
            callback();
        }
    }

private:
    std::vector<Animation> active_animations;
};
