#include "animation_manager.hpp"
import std.compat;
#include <SDL.h>

void AnimationManager::createAnimation(
    float* target_property,
    float start_value,
    float end_value,
    Uint32 duration,
    std::function<float(float)> easing,
    Animation::CompleteCallback on_complete,
    std::function<void()> on_update
) {
    if (!target_property) {
        std::cerr << "AnimationManager Error: target_property cannot be null." << std::endl;
        return;
    }
    
    // Ustawienie wartości początkowej
    *target_property = start_value;

    active_animations.emplace_back(
        Animation{
            .target_property = target_property,
            .start_value = start_value,
            .end_value = end_value,
            .start_time = SDL_GetTicks64(),
            .duration_ms = duration,
            .easing_function = easing,
            .on_complete_callback = on_complete,
            .on_update = on_update,
            .is_finished = false
        }
    );
}

void AnimationManager::update() {
    if (active_animations.empty()) {
        return;
    }

    Uint64 current_time = SDL_GetTicks64();
    std::vector<Animation::CompleteCallback> completed_callbacks;

    for (auto& anim : active_animations) {
        if (anim.is_finished) continue;

        float raw_progress = static_cast<float>(current_time - anim.start_time) / static_cast<float>(anim.duration_ms);
        
        if (raw_progress >= 1.0f) {
            raw_progress = 1.0f;
            anim.is_finished = true;
        }

        float eased_progress = anim.easing_function(raw_progress);
        
        // Zastosuj interpolację do właściwości docelowej
        if (anim.target_property) {
            *anim.target_property = std::lerp(anim.start_value, anim.end_value, eased_progress);
        }

        // Wywołaj callback on_update, jeśli istnieje
        if (anim.on_update) {
            anim.on_update();
        }

        if (anim.is_finished && anim.on_complete_callback) {
            completed_callbacks.push_back(anim.on_complete_callback);
        }
    }
    
    // Usuń zakończone animacje
    std::erase_if(active_animations, [](const auto& anim) {
        return anim.is_finished;
    });

    // Wywołaj callbacki po usunięciu, aby uniknąć problemów z modyfikacją kolekcji
    for (const auto& callback : completed_callbacks) {
        callback();
    }
}