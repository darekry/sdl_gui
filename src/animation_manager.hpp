#pragma once
import std.compat;
#include "easing.hpp"
#include <SDL.h>

struct Animation {
    using CompleteCallback = std::function<void()>;

    float* target_property = nullptr; // Wskaźnik na animowaną właściwość
    float start_value = 0.0f;
    float end_value = 0.0f;
    
    Uint64 start_time = 0;
    Uint32 duration_ms = 0;
    
    std::function<float(float)> easing_function = Easing::linear;
    CompleteCallback on_complete_callback = nullptr;
    std::function<void()> on_update = nullptr;

    bool is_finished = false;
};

class AnimationManager {
public:
    AnimationManager() = default;

    /**
     * @brief Tworzy nową animację dla dowolnej właściwości typu float.
     * @param target_property Wskaźnik na właściwość do animowania (np. &element->position.x).
     * @param start_value Wartość początkowa animacji.
     * @param end_value Wartość końcowa animacji.
     * @param duration Czas trwania animacji w milisekundach.
     * @param easing Funkcja easing do zastosowania (domyślnie liniowa).
     * @param on_complete Opcjonalny callback wywołyany po zakończeniu animacji.
     */
    void createAnimation(
        float* target_property,
        float start_value,
         float end_value,
         Uint32 duration,
         std::function<float(float)> easing = Easing::linear,
         Animation::CompleteCallback on_complete = nullptr,
         std::function<void()> on_update = nullptr
     );

    /**
     * @brief Aktualizuje stan wszystkich aktywnych animacji.
     * Metoda ta powinna być wywoływana w każdej klatce głównej pętli aplikacji.
     */
    void update();

private:
    std::vector<Animation> active_animations;
};