#pragma once

#include <SDL3/SDL.h>
#include "logger.hpp"
#include "easing.hpp"

#include "std.hpp"

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

struct LoopingAnimation {
    uint32_t id = 0;
    std::function<void()> callback;
    uint32_t interval_ms = 0;
    uint64_t last_execution_time = 0;
};

/**
 * @brief Manages animations for GUI elements.
 * 
 * @warning This class is NOT thread-safe. All methods must be called from the same thread
 *          that owns the GUIManager. Concurrent calls from different threads may cause data races.
 *          
 * @warning Animation callbacks that modify external data should be carefully managed.
 *          The Animation struct stores raw pointers to target properties. If the target object
 *          is destroyed before the animation completes, undefined behavior will occur.
 *          Consider using std::shared_ptr or ensuring animations complete before object destruction.
 */
class AnimationManager {
public:
    AnimationManager() = default;

    uint32_t addAnimation(uint32_t interval_ms, std::function<void()> callback) {
        uint32_t id = next_looping_animation_id++;
        looping_animations.emplace_back(
            LoopingAnimation{
                .id = id,
                .callback = callback,
                .interval_ms = interval_ms,
                .last_execution_time = SDL_GetTicks()
            }
        );
        return id;
    }

    void removeAnimation(uint32_t id) {
        looping_animations.erase(
            std::remove_if(looping_animations.begin(), looping_animations.end(),
                           [id](const auto& anim) { return anim.id == id; }),
            looping_animations.end()
        );
    }

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
            LOG_ERROR("AnimationManager", "target_property cannot be null.");
            return;
        }
        *target_property = static_cast<T>(start_value);
        active_animations.emplace_back(
            Animation{
                .target_property = target_property,
                .start_value = start_value,
                .end_value = end_value,
                .start_time = SDL_GetTicks(),
                .duration_ms = duration,
                .easing_function = easing,
                .on_complete_callback = on_complete,
                .is_finished = false
            }
        );
    }

    void update() {
        Uint64 current_time = SDL_GetTicks();
        
        if (!active_animations.empty()) {
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

        for (auto& anim : looping_animations) {
            if (current_time - anim.last_execution_time >= anim.interval_ms) {
                if (anim.callback) {
                    anim.callback();
                }
                anim.last_execution_time = current_time;
            }
        }
    }

private:
    std::vector<Animation> active_animations;
    std::vector<LoopingAnimation> looping_animations;
    uint32_t next_looping_animation_id = 0;
};
