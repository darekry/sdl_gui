#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "../src/animation_manager.hpp"
#include "../src/easing.hpp"
#include <SDL2/SDL.h>

TEST_CASE("AnimationManager functionality", "[animation_manager]") {
    AnimationManager animManager;

    SECTION("Animation of int property") {
        int value = 0;
        animManager.createAnimation(&value, 0.0f, 100.0f, 120, Easing::linear);
        
        REQUIRE(value == 0);
        
        SDL_Delay(60);
        animManager.update();
        REQUIRE(value >= 40);
        REQUIRE(value <= 80);
        
        SDL_Delay(80);
        animManager.update();
        REQUIRE(value == 100);
    }

    SECTION("Animation of float property") {
        float value = 0.0f;
        animManager.createAnimation(&value, 0.0f, 1.0f, 120, Easing::linear);
        
        REQUIRE(value == Catch::Approx(0.0f));
        
        SDL_Delay(60);
        animManager.update();
        REQUIRE(value >= 0.3f);
        REQUIRE(value <= 0.8f);
        
        SDL_Delay(80);
        animManager.update();
        REQUIRE(value == Catch::Approx(1.0f).margin(0.01f));
    }

    SECTION("Animation completion callback is invoked") {
        int value = 0;
        bool callbackInvoked = false;
        
        animManager.createAnimation(&value, 0.0f, 100.0f, 60, Easing::linear, [&]() {
            callbackInvoked = true;
        });
        
        REQUIRE_FALSE(callbackInvoked);
        
        SDL_Delay(80);
        animManager.update();
        REQUIRE(callbackInvoked);
        REQUIRE(value == 100);
    }

    SECTION("Multiple animations can run simultaneously") {
        int value1 = 0;
        int value2 = 0;
        
        animManager.createAnimation(&value1, 0.0f, 100.0f, 120, Easing::linear);
        animManager.createAnimation(&value2, 0.0f, 50.0f, 120, Easing::linear);
        
        SDL_Delay(60);
        animManager.update();
        
        REQUIRE(value1 >= 40);
        REQUIRE(value1 <= 80);
        REQUIRE(value2 >= 20);
        REQUIRE(value2 <= 40);
        
        SDL_Delay(80);
        animManager.update();
        
        REQUIRE(value1 == 100);
        REQUIRE(value2 == 50);
    }

    SECTION("Animation with easeInOut easing") {
        float value = 0.0f;
        animManager.createAnimation(&value, 0.0f, 100.0f, 120, Easing::easeInOutQuad);
        
        SDL_Delay(40);
        animManager.update();
        float early = value;
        
        SDL_Delay(40);
        animManager.update();
        float mid = value;
        
        SDL_Delay(80);
        animManager.update();
        
        REQUIRE(value == Catch::Approx(100.0f).margin(0.01f));
        REQUIRE(mid > early);
        REQUIRE(mid < 80.0f);
    }

    SECTION("Finished animations are removed") {
        int value = 0;
        int completionCount = 0;
        
        animManager.createAnimation(&value, 0.0f, 100.0f, 60, Easing::linear, [&]() {
            ++completionCount;
        });
        
        SDL_Delay(80);
        animManager.update();
        REQUIRE(completionCount == 1);
        
        SDL_Delay(100);
        animManager.update();
        REQUIRE(completionCount == 1); // Should not be called again
    }

    SECTION("Animation updates property immediately to start value") {
        int value = 50;
        animManager.createAnimation(&value, 10.0f, 100.0f, 120, Easing::linear);
        REQUIRE(value == 10);
    }
}
