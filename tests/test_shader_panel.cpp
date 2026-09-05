#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/shader_panel.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ShaderPanel - construction", "[shader_panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("constructor creates a panel-like element") {
        ShaderPanel panel(manager, 10, 20, 200, 100);
        REQUIRE(panel.getX() == 10);
        REQUIRE(panel.getY() == 20);
        REQUIRE(panel.getWidth() == 200);
        REQUIRE(panel.getHeight() == 100);
    }

    SECTION("getComponentTypeIdId returns ShaderPanel") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        REQUIRE(panel.getComponentTypeId() == ComponentType::ShaderPanel);
    }

    SECTION("wantsDirectRender is true") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        REQUIRE(panel.wantsDirectRender());
    }
}

TEST_CASE("ShaderPanel - shader state", "[shader_panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("shader enabled by default") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        REQUIRE(panel.isShaderEnabled());
    }

    SECTION("setShaderEnabled toggles") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        panel.setShaderEnabled(false);
        REQUIRE_FALSE(panel.isShaderEnabled());
        panel.setShaderEnabled(true);
        REQUIRE(panel.isShaderEnabled());
    }

    SECTION("uniforms can be set") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        panel.setUniformTime(0.5f);
        panel.setUniformMouse(12.0f, 34.0f);
        panel.setUniformTime(1.0f);
        panel.setUniformMouse(-1.0f, -1.0f);
    }

    SECTION("setShader with null data on CPU context is a safe no-op") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        panel.setShader(nullptr, 0);
        panel.setShaderEnabled(true);
        REQUIRE(panel.isShaderEnabled());
    }

    SECTION("setShader with garbage data on CPU context is a safe no-op") {
        ShaderPanel panel(manager, 0, 0, 100, 50);
        uint8_t fakeSpirv[16] = {0x03, 0x02, 0x23, 0x07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        panel.setShader(fakeSpirv, sizeof(fakeSpirv));
        REQUIRE(panel.isShaderEnabled());
    }
}

TEST_CASE("ShaderPanel - render on CPU context", "[shader_panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("renders as a plain panel without a GPU device") {
        auto panel = std::make_unique<ShaderPanel>(manager, 10, 10, 200, 100);
        manager.addElement(std::move(panel));
        manager.render();
    }

    SECTION("render with shader enabled and set uniform time") {
        auto panel = std::make_unique<ShaderPanel>(manager, 10, 10, 200, 100);
        panel->setUniformTime(0.33f);
        panel->setUniformMouse(50.0f, 25.0f);
        manager.addElement(std::move(panel));
        manager.render();
    }

    SECTION("render with shader disabled") {
        auto panel = std::make_unique<ShaderPanel>(manager, 10, 10, 200, 100);
        panel->setShaderEnabled(false);
        manager.addElement(std::move(panel));
        manager.render();
    }

    SECTION("render at zero size is safe") {
        auto panel = std::make_unique<ShaderPanel>(manager, 0, 0, 0, 0);
        manager.addElement(std::move(panel));
        manager.render();
    }
}
