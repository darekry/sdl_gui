#include "timer_manager.hpp"
#include "gui.hpp"


uint32_t TimerManager::addTimer(GUIElement* target, uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback) {
    TimerEvent newTimer;
    newTimer.id = nextTimerId;
    newTimer.target = target;
    newTimer.executionTime = SDL_GetTicks() + delay;
    newTimer.callback = callback;
    newTimer.singleShot = singleShot;
    newTimer.interval = delay;

    timers.push_back(newTimer);
    return nextTimerId++;
}

void TimerManager::removeTimer(uint32_t timerId) {
    auto new_end = std::remove_if(timers.begin(), timers.end(), [timerId](const TimerEvent& timer) {
        return timer.id == timerId;
    });
    timers.erase(new_end, timers.end());
}

void TimerManager::update() {
    uint32_t currentTime = SDL_GetTicks();

    for (auto& timer : timers) {
        if (currentTime >= timer.executionTime) {
            if (timer.callback) {
                timer.callback(timer.target);
            }
            if (!timer.singleShot) {
                timer.executionTime = currentTime + timer.interval;
            }
        }
    }

    auto new_end = std::remove_if(timers.begin(), timers.end(), [currentTime](const TimerEvent& timer) {
        return timer.singleShot && currentTime >= timer.executionTime;
    });
    timers.erase(new_end, timers.end());
}