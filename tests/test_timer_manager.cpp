#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/timer_manager.hpp"
#include "../src/gui.hpp"
#include "../src/panel.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TimerManager functionality", "[timer_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TimerManager* timerManager = manager.getTimerManager();

    SECTION("Single-shot timer executes once") {
        int callCount = 0;
        timerManager->addTimer(nullptr, 10, true, [&](GUIElement*) {
            ++callCount;
        });

        REQUIRE(callCount == 0);
        
        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount == 1);

        SDL_Delay(20);
        timerManager->update();
        REQUIRE(callCount == 1); // Should still be 1
    }

    SECTION("Repeating timer executes multiple times") {
        int callCount = 0;
        uint32_t timerId = timerManager->addTimer(nullptr, 10, false, [&](GUIElement*) {
            ++callCount;
        });

        REQUIRE(callCount == 0);

        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount == 1);

        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount == 2);

        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount == 3);

        timerManager->removeTimer(timerId);
    }

    SECTION("Timer can be removed") {
        int callCount = 0;
        uint32_t timerId = timerManager->addTimer(nullptr, 10, false, [&](GUIElement*) {
            ++callCount;
        });

        timerManager->removeTimer(timerId);
        
        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount == 0);
    }

    SECTION("Multiple timers can run independently") {
        int callCount1 = 0;
        int callCount2 = 0;

        uint32_t timerId1 = timerManager->addTimer(nullptr, 10, false, [&](GUIElement*) {
            ++callCount1;
        });

        uint32_t timerId2 = timerManager->addTimer(nullptr, 20, false, [&](GUIElement*) {
            ++callCount2;
        });

        SDL_Delay(15);
        timerManager->update();
        REQUIRE(callCount1 == 1);
        REQUIRE(callCount2 == 0);

        SDL_Delay(10);
        timerManager->update();
        REQUIRE(callCount1 == 2);
        REQUIRE(callCount2 == 1);

        timerManager->removeTimer(timerId1);
        timerManager->removeTimer(timerId2);
    }

    SECTION("Timer passes target element to callback") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 100);
        GUIElement* target = panel.get();
        manager.addElement(std::move(panel));

        GUIElement* receivedTarget = nullptr;
        timerManager->addTimer(target, 10, true, [&](GUIElement* element) {
            receivedTarget = element;
        });

        SDL_Delay(15);
        timerManager->update();
        REQUIRE(receivedTarget == target);
    }
}
