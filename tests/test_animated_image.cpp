#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/animated_image.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("AnimatedImage functionality", "[animated_image]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("AnimatedImage can be created") {
        AnimatedImage img(manager, 10, 20, 100, 100);
        REQUIRE(img.getX() == 10);
        REQUIRE(img.getY() == 20);
        REQUIRE(img.getWidth() == 100);
        REQUIRE(img.getHeight() == 100);
    }

    SECTION("Initial state is stopped at frame 0") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        REQUIRE(img.getCurrentFrame() == 0);
        REQUIRE_FALSE(img.isPlaying());
    }

    SECTION("setSpriteSheet configures animation") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(160, 160);
        helper.primeTextureAs("test_sprite.png", tex);
        
        img.setSpriteSheet("test_sprite.png", 4, 2, 80, 80);
        REQUIRE(img.getTotalFrames() == 4);
        REQUIRE(img.getCurrentFrame() == 0);
    }

    SECTION("setFrame changes current frame") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(160, 160);
        helper.primeTextureAs("frames.png", tex);
        
        img.setSpriteSheet("frames.png", 8, 2);
        img.setFrame(3);
        REQUIRE(img.getCurrentFrame() == 3);
        
        img.setFrame(7);
        REQUIRE(img.getCurrentFrame() == 7);
    }

    SECTION("setFrame clamps to valid range") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(160, 80);
        helper.primeTextureAs("test.png", tex);
        
        img.setSpriteSheet("test.png", 4, 1);
        
        img.setFrame(-1);
        REQUIRE(img.getCurrentFrame() == 0);
        
        img.setFrame(100);
        REQUIRE(img.getCurrentFrame() == 3);
    }

    SECTION("play and pause control playback") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(200, 100);
        helper.primeTextureAs("anim.png", tex);
        
        img.setSpriteSheet("anim.png", 4, 1);
        
        REQUIRE_FALSE(img.isPlaying());
        
        img.play();
        REQUIRE(img.isPlaying());
        
        img.pause();
        REQUIRE_FALSE(img.isPlaying());
    }

    SECTION("stop resets to frame 0") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(200, 100);
        helper.primeTextureAs("anim2.png", tex);
        
        img.setSpriteSheet("anim2.png", 4, 1);
        img.setFrame(2);
        REQUIRE(img.getCurrentFrame() == 2);
        
        img.stop();
        REQUIRE(img.getCurrentFrame() == 0);
        REQUIRE_FALSE(img.isPlaying());
    }

    SECTION("setFPS sets frame duration") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setFPS(30.0f);
    }

    SECTION("setFrameDuration updates duration") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setFrameDuration(0.1f);
    }

    SECTION("setLoop controls looping behavior") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setLoop(true);
        img.setLoop(false);
    }

    SECTION("setUseCache controls rendering mode") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setUseCache(true);
        REQUIRE_FALSE(img.wantsDirectRender());
        
        img.setUseCache(false);
        REQUIRE(img.wantsDirectRender());
    }

    SECTION("ScaleMode can be set") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setScaleMode(AnimatedImage::ScaleMode::Fit);
        img.setScaleMode(AnimatedImage::ScaleMode::Center);
        img.setScaleMode(AnimatedImage::ScaleMode::None);
    }

    SECTION("setPreserveAspect controls aspect ratio") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        img.setPreserveAspect(true);
        img.setPreserveAspect(false);
    }

    SECTION("onFrameChanged callback is invoked") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(200, 100);
        helper.primeTextureAs("callback.png", tex);
        
        img.setSpriteSheet("callback.png", 4, 1);
        
        int callbackFrame = -1;
        img.setOnFrameChanged([&](int frame) {
            callbackFrame = frame;
        });
        
        img.setFrame(2);
        REQUIRE(callbackFrame == 2);
    }

    SECTION("onAnimationEnd callback can be set") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        bool endCalled = false;
        img.setOnAnimationEnd([&]() {
            endCalled = true;
        });
    }

    SECTION("getTotalFrames returns frame count") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(300, 100);
        helper.primeTextureAs("total.png", tex);
        
        img.setSpriteSheet("total.png", 6, 2);
        REQUIRE(img.getTotalFrames() == 6);
    }

    SECTION("Component type is correct") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        REQUIRE(img.getComponentTypeId() == ComponentType::AnimatedImage);
    }

    SECTION("AnimatedImage can be added to manager") {
        auto img = std::make_unique<AnimatedImage>(manager, 50, 50, 150, 150);
        AnimatedImage* imgPtr = img.get();
        manager.addElement(std::move(img));
        
        REQUIRE(imgPtr->getX() == 50);
        REQUIRE(imgPtr->getY() == 50);
    }

    SECTION("setSpriteSheet with automatic frame calculation") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(160, 160);
        helper.primeTextureAs("auto_frames.png", tex);
        
        img.setSpriteSheet("auto_frames.png", 8, 2, 0, 0);
        REQUIRE(img.getTotalFrames() == 8);
    }

    SECTION("Frame doesn't change when setting the same frame") {
        AnimatedImage img(manager, 0, 0, 100, 100);
        
        auto tex = helper.makeStubTexture(200, 100);
        helper.primeTextureAs("same.png", tex);
        
        img.setSpriteSheet("same.png", 4, 1);
        
        int callbackCount = 0;
        img.setOnFrameChanged([&](int) { ++callbackCount; });
        
        img.setFrame(1);
        REQUIRE(callbackCount == 1);
        
        img.setFrame(1);
        REQUIRE(callbackCount == 1); // Should not increment
    }
}
