#pragma once

#include <SDL3/SDL.h>

#include "std.hpp"


class GUIElement;

struct TimerEvent {
    uint32_t id;
    Uint64 executionTime;
    GUIElement* target;
    std::function<void(GUIElement*)> callback;
    bool singleShot;
    uint32_t interval;
};

/**
 * @brief Manages timers for GUI elements.
 * 
 * @warning This class is NOT thread-safe. All methods must be called from the same thread
 *          that owns the GUIManager. Concurrent calls from different threads may cause data races.
 *          If multi-threaded timer management is needed, use external synchronization (e.g., std::mutex).
 */
class TimerManager {
public:
    uint32_t addTimer(GUIElement* target, uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void removeTimer(uint32_t timerId);
    void update();

private:
    std::vector<TimerEvent> timers;
    uint32_t nextTimerId = 1;
};
