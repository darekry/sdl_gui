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
    Uint64 currentTime = SDL_GetTicks();

    // Collect timers that need to execute, to avoid iterator invalidation
    // if callbacks modify the timers vector
    std::vector<size_t> timers_to_execute;
    timers_to_execute.reserve(timers.size());
    for (size_t i = 0; i < timers.size(); ++i) {
        if (currentTime >= timers[i].executionTime) {
            timers_to_execute.push_back(i);
        }
    }

    // Execute callbacks for triggered timers
    for (size_t idx : timers_to_execute) {
        if (idx < timers.size() && timers[idx].callback) {
            timers[idx].callback(timers[idx].target);
        }
    }

    // Update repeating timers and remove single-shot timers
    // Use erase-remove with proper predicate
    auto new_end = std::remove_if(timers.begin(), timers.end(), 
        [currentTime](const TimerEvent& timer) {
            if (currentTime >= timer.executionTime) {
                return timer.singleShot; // Remove single-shot timers that executed
            }
            return false;
        });
    timers.erase(new_end, timers.end());

    // Update execution time for repeating timers
    for (auto& timer : timers) {
        if (!timer.singleShot && currentTime >= timer.executionTime) {
            timer.executionTime = currentTime + timer.interval;
        }
    }
}