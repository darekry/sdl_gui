#pragma once


#include <SDL.h>
#include <functional>
#include <vector>


class GUIElement;

struct TimerEvent {
    uint32_t id;
    uint32_t executionTime;
    GUIElement* target;
    std::function<void(GUIElement*)> callback;
    bool singleShot;
    uint32_t interval;
};

class TimerManager {
public:
    uint32_t addTimer(GUIElement* target, uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void removeTimer(uint32_t timerId);
    void update();

private:
    std::vector<TimerEvent> timers;
    uint32_t nextTimerId = 1;
};