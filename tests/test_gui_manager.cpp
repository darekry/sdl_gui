#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/button.hpp"
#include "../src/panel.hpp"
#include "../src/label.hpp"
#include "../src/text_input.hpp"
#include "../src/theme.hpp"
#include "../src/style.hpp"
#include "../src/constants.hpp"

TEST_CASE("GUIManager Element Management", "[gui_manager][elements]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("addElement adds elements and returns pointer") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
        Button* rawPtr = button.get();
        GUIElement* returnedPtr = manager.addElement(std::move(button));
        
        REQUIRE(returnedPtr == rawPtr);
        REQUIRE(returnedPtr != nullptr);
    }

    SECTION("Elements added to manager receive proper initialization") {
        auto button = std::make_unique<Button>(manager, 50, 50, 100, 40, "Init");
        Button* btnPtr = button.get();
        manager.addElement(std::move(button));
        
        REQUIRE(btnPtr->getX() == 50);
        REQUIRE(btnPtr->getY() == 50);
        REQUIRE(btnPtr->getWidth() == 100);
        REQUIRE(btnPtr->getHeight() == 40);
        REQUIRE(btnPtr->isEnabled());
        REQUIRE(btnPtr->isVisible());
    }

    SECTION("Multiple elements can be added") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 80, 30, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 100, 10, 80, 30, "Btn2");
        auto panel = std::make_unique<Panel>(manager, 200, 10, 100, 50);
        
        Button* btn1Ptr = btn1.get();
        Button* btn2Ptr = btn2.get();
        Panel* panelPtr = panel.get();
        
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        manager.addElement(std::move(panel));
        
        REQUIRE(btn1Ptr != nullptr);
        REQUIRE(btn2Ptr != nullptr);
        REQUIRE(panelPtr != nullptr);
        
        REQUIRE(manager.findElementAt(20, 20) == btn1Ptr);
        REQUIRE(manager.findElementAt(120, 20) == btn2Ptr);
        REQUIRE(manager.findElementAt(220, 30) == panelPtr);
    }

    SECTION("Elements can be added in different order") {
        auto btnA = std::make_unique<Button>(manager, 0, 0, 50, 50, "");
        auto btnB = std::make_unique<Button>(manager, 100, 0, 50, 50, "");
        auto btnC = std::make_unique<Button>(manager, 200, 0, 50, 50, "");
        
        Button* ptrA = btnA.get();
        Button* ptrB = btnB.get();
        Button* ptrC = btnC.get();
        
        manager.addElement(std::move(btnC));
        manager.addElement(std::move(btnA));
        manager.addElement(std::move(btnB));
        
        REQUIRE(manager.findElementAt(25, 25) == ptrA);
        REQUIRE(manager.findElementAt(125, 25) == ptrB);
        REQUIRE(manager.findElementAt(225, 25) == ptrC);
    }
}

TEST_CASE("GUIManager Event Processing - Mouse Motion", "[gui_manager][events][mouse]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("processEvent handles mouse motion events") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Hover");
        Button* btn = button.get();
        manager.addElement(std::move(button));
        
        REQUIRE(btn->getState() == ElementState::Normal);
        
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);
        
        manager.processEvent(helper.createMouseMotion(400, 400));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Mouse motion events forwarded to appropriate elements based on position") {
        auto btn1 = std::make_unique<Button>(manager, 0, 0, 100, 50, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 150, 0, 100, 50, "Btn2");
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        manager.processEvent(helper.createMouseMotion(50, 25));
        REQUIRE(ptr1->getState() == ElementState::Hover);
        REQUIRE(ptr2->getState() == ElementState::Normal);
        
        manager.processEvent(helper.createMouseMotion(200, 25));
        REQUIRE(ptr1->getState() == ElementState::Normal);
        REQUIRE(ptr2->getState() == ElementState::Hover);
    }
}

TEST_CASE("GUIManager Event Processing - Mouse Button", "[gui_manager][events][mouse]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("processEvent handles mouse button events") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Click");
        Button* btn = button.get();
        int clicks = 0;
        button->setOnClickCallback([&](GUIElement*) { ++clicks; });
        manager.addElement(std::move(button));
        
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(clicks == 1);
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Mouse button events forwarded to element at position") {
        auto btn1 = std::make_unique<Button>(manager, 0, 0, 100, 50, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 150, 0, 100, 50, "Btn2");
        int clicks1 = 0;
        int clicks2 = 0;
        btn1->setOnClickCallback([&](GUIElement*) { ++clicks1; });
        btn2->setOnClickCallback([&](GUIElement*) { ++clicks2; });
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 50, 25));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 50, 25));
        REQUIRE(clicks1 == 1);
        REQUIRE(clicks2 == 0);
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 200, 25));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 200, 25));
        REQUIRE(clicks1 == 1);
        REQUIRE(clicks2 == 1);
    }
}

TEST_CASE("GUIManager Event Processing - Keyboard", "[gui_manager][events][keyboard]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("processEvent handles keyboard events when element has focus") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(inputPtr->hasKeyboardFocus());
        
        manager.processEvent(helper.createTextInputEvent("H"));
        REQUIRE(inputPtr->getText() == "H");
        
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));
        REQUIRE(inputPtr->getText().empty());
    }

    SECTION("Keyboard events ignored when no element has focus") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        REQUIRE_FALSE(inputPtr->hasKeyboardFocus());
        
        manager.processEvent(helper.createTextInputEvent("A"));
        REQUIRE(inputPtr->getText().empty());
        
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));
        REQUIRE(inputPtr->getText().empty());
    }
}

TEST_CASE("GUIManager Event Processing - Mouse Wheel", "[gui_manager][events][wheel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("processEvent handles mouse wheel events") {
        auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));
        
        manager.processEvent(helper.createMouseWheel(1, 0));
        
        SDL_Event wheelDown = helper.createMouseWheel(-1, 0);
        manager.processEvent(wheelDown);
    }

    SECTION("Mouse wheel events are processed by event loop") {
        SDL_Event wheelEvent = helper.createMouseWheel(5, 0);
        bool processed = manager.processEvent(wheelEvent);
        REQUIRE_FALSE(processed);
    }
}

TEST_CASE("GUIManager Rendering", "[gui_manager][render]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("render renders all visible elements") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "Btn2");
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        ptr1->setVisible(true);
        ptr2->setVisible(true);
        
        manager.render();
        
        REQUIRE(ptr1->isVisible());
        REQUIRE(ptr2->isVisible());
    }

    SECTION("render handles hidden elements correctly") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Visible");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "Hidden");
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        ptr2->setVisible(false);
        
        manager.render();
        
        REQUIRE(ptr1->isVisible());
        REQUIRE_FALSE(ptr2->isVisible());
    }

    SECTION("update updates manager state") {
        auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
        manager.addElement(std::move(btn));
        
        manager.update();
        
        REQUIRE(manager.getTimerManager() != nullptr);
        REQUIRE(manager.getAnimationManager() != nullptr);
    }
}

TEST_CASE("GUIManager Focus Management", "[gui_manager][focus]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setKeyboardFocus sets focused element") {
        auto input1 = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        auto input2 = std::make_unique<TextInput>(manager, 10, 50, 200, 30);
        TextInput* ptr1 = input1.get();
        TextInput* ptr2 = input2.get();
        manager.addElement(std::move(input1));
        manager.addElement(std::move(input2));
        
        manager.setKeyboardFocus(ptr1);
        REQUIRE(ptr1->hasKeyboardFocus());
        REQUIRE_FALSE(ptr2->hasKeyboardFocus());
        
        manager.setKeyboardFocus(ptr2);
        REQUIRE_FALSE(ptr1->hasKeyboardFocus());
        REQUIRE(ptr2->hasKeyboardFocus());
    }

    SECTION("getKeyboardFocus returns focused element") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));
        
        REQUIRE(manager.getKeyboardFocus() == nullptr);
        
        manager.setKeyboardFocus(ptr);
        REQUIRE(manager.getKeyboardFocus() == ptr);
        
        manager.setKeyboardFocus(nullptr);
        REQUIRE(manager.getKeyboardFocus() == nullptr);
    }

    SECTION("Keyboard focus affects which element receives key events") {
        auto input1 = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        auto input2 = std::make_unique<TextInput>(manager, 10, 50, 200, 30);
        TextInput* ptr1 = input1.get();
        TextInput* ptr2 = input2.get();
        manager.addElement(std::move(input1));
        manager.addElement(std::move(input2));
        
        manager.setKeyboardFocus(ptr1);
        manager.processEvent(helper.createTextInputEvent("A"));
        REQUIRE(ptr1->getText() == "A");
        REQUIRE(ptr2->getText().empty());
        
        manager.setKeyboardFocus(ptr2);
        manager.processEvent(helper.createTextInputEvent("B"));
        REQUIRE(ptr1->getText() == "A");
        REQUIRE(ptr2->getText() == "B");
    }

    SECTION("Click on focusable element gives it keyboard focus") {
        auto input = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));
        
        REQUIRE_FALSE(ptr->hasKeyboardFocus());
        
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55));
        
        REQUIRE(ptr->hasKeyboardFocus());
        REQUIRE(manager.getKeyboardFocus() == ptr);
    }

    SECTION("Focus lost and gained callbacks are invoked") {
        class FocusTestElement : public GUIElement {
        public:
            int focusGainedCount = 0;
            int focusLostCount = 0;
            
            FocusTestElement(GUIManager& m, int x, int y, int w, int h)
                : GUIElement(m, x, y, w, h) {
                setCanGetKeyboardFocus(true);
            }
            
            void onFocusGained() override { ++focusGainedCount; }
            void onFocusLost() override { ++focusLostCount; }
            
            ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
            void draw(SDL_Renderer*) override {}
        };
        
        auto elem = std::make_unique<FocusTestElement>(manager, 10, 10, 100, 30);
        FocusTestElement* ptr = elem.get();
        manager.addElement(std::move(elem));
        
        REQUIRE(ptr->focusGainedCount == 0);
        REQUIRE(ptr->focusLostCount == 0);
        
        manager.setKeyboardFocus(ptr);
        REQUIRE(ptr->focusGainedCount == 1);
        REQUIRE(ptr->focusLostCount == 0);
        
        manager.setKeyboardFocus(nullptr);
        REQUIRE(ptr->focusGainedCount == 1);
        REQUIRE(ptr->focusLostCount == 1);
        
        manager.setKeyboardFocus(ptr);
        REQUIRE(ptr->focusGainedCount == 2);
        REQUIRE(ptr->focusLostCount == 1);
    }
}

TEST_CASE("GUIManager Mouse Capture", "[gui_manager][capture]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("captureMouse captures mouse for element") {
        class CaptureTestElement : public GUIElement {
        public:
            int captureGainedCount = 0;
            int captureLostCount = 0;
            
            CaptureTestElement(GUIManager& m, int x, int y, int w, int h)
                : GUIElement(m, x, y, w, h) {}
            
            void onMouseCaptureGained() override { ++captureGainedCount; }
            void onMouseCaptureLost() override { ++captureLostCount; }
            
            ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
            void draw(SDL_Renderer*) override {}
        };
        
        auto elem = std::make_unique<CaptureTestElement>(manager, 10, 10, 100, 30);
        CaptureTestElement* ptr = elem.get();
        manager.addElement(std::move(elem));
        
        REQUIRE(ptr->captureGainedCount == 0);
        REQUIRE(ptr->captureLostCount == 0);
        
        manager.captureMouse(ptr);
        REQUIRE(ptr->captureGainedCount == 1);
        REQUIRE(ptr->captureLostCount == 0);
    }

    SECTION("releaseMouse releases mouse capture") {
        class CaptureTestElement : public GUIElement {
        public:
            int captureGainedCount = 0;
            int captureLostCount = 0;
            
            CaptureTestElement(GUIManager& m, int x, int y, int w, int h)
                : GUIElement(m, x, y, w, h) {}
            
            void onMouseCaptureGained() override { ++captureGainedCount; }
            void onMouseCaptureLost() override { ++captureLostCount; }
            
            ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
            void draw(SDL_Renderer*) override {}
        };
        
        auto elem = std::make_unique<CaptureTestElement>(manager, 10, 10, 100, 30);
        CaptureTestElement* ptr = elem.get();
        manager.addElement(std::move(elem));
        
        manager.captureMouse(ptr);
        REQUIRE(ptr->captureGainedCount == 1);
        
        manager.releaseMouse();
        REQUIRE(ptr->captureLostCount == 1);
    }

    SECTION("Mouse capture affects event routing") {
        class CaptureTestElement : public GUIElement {
        public:
            std::vector<Uint32> receivedEvents;
            
            CaptureTestElement(GUIManager& m, int x, int y, int w, int h)
                : GUIElement(m, x, y, w, h) {}
            
            bool handleEvent(const SDL_Event& e) override {
                receivedEvents.push_back(e.type);
                return true;
            }
            
            ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
            void draw(SDL_Renderer*) override {}
        };
        
        auto capturing = std::make_unique<CaptureTestElement>(manager, 10, 10, 100, 100);
        auto other = std::make_unique<CaptureTestElement>(manager, 150, 10, 100, 100);
        CaptureTestElement* capturePtr = capturing.get();
        CaptureTestElement* otherPtr = other.get();
        manager.addElement(std::move(capturing));
        manager.addElement(std::move(other));
        
        manager.captureMouse(capturePtr);
        
        manager.processEvent(helper.createMouseMotion(200, 50));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 200, 50));
        
        REQUIRE(capturePtr->receivedEvents.size() == 2);
        REQUIRE(otherPtr->receivedEvents.empty());
        
        manager.releaseMouse();
        
        manager.processEvent(helper.createMouseMotion(200, 50));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 200, 50));
        
        REQUIRE(otherPtr->receivedEvents.size() >= 1);
    }
}

TEST_CASE("GUIManager Theme", "[gui_manager][theme]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setTheme sets custom theme") {
        Theme customTheme;
        Style btnStyle;
        btnStyle.backgroundColor = SDL_Color{100, 100, 100, 255};
        customTheme.setStyle(ComponentType::Button, btnStyle);
        
        manager.setTheme(customTheme);
        
        REQUIRE(manager.getTheme().getStyle(ComponentType::Button).backgroundColor.has_value());
        REQUIRE(manager.getTheme().getStyle(ComponentType::Button).backgroundColor->r == 100);
    }

    SECTION("getTheme returns current theme") {
        Theme& theme = manager.getTheme();
        REQUIRE_NOTHROW(theme.getStyle(ComponentType::Button));
        REQUIRE_NOTHROW(theme.getDefaultStyle());
    }

    SECTION("Setting theme marks all elements dirty") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "Btn2");
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        Theme newTheme = Theme::createDefaultTheme();
        manager.setTheme(newTheme);
        
        manager.render();
    }
}

TEST_CASE("GUIManager Resource Managers", "[gui_manager][resources]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getFontManager returns valid FontManager") {
        FontManager& fm = manager.getFontManager();
        REQUIRE(fm.isInitialized());
        REQUIRE(fm.getDefaultFont() != nullptr);
    }

    SECTION("getTextureManager returns valid TextureManager") {
        TextureManager& tm = manager.getTextureManager();
        REQUIRE(tm.getCacheSize() >= 0);
    }

    SECTION("getTimerManager returns valid TimerManager") {
        TimerManager* tm = manager.getTimerManager();
        REQUIRE(tm != nullptr);
    }

    SECTION("getAnimationManager returns valid AnimationManager") {
        AnimationManager* am = manager.getAnimationManager();
        REQUIRE(am != nullptr);
    }

    SECTION("getRenderer returns the SDL renderer") {
        SDL_Renderer* renderer = manager.getRenderer();
        REQUIRE(renderer != nullptr);
        REQUIRE(renderer == helper.getRenderer());
    }
}

TEST_CASE("GUIManager Tooltip", "[gui_manager][tooltip]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("showTooltip displays tooltip") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
        Button* btn = button.get();
        manager.addElement(std::move(button));
        
        manager.showTooltip(btn, "Tooltip text");
        
        manager.render();
    }

    SECTION("hideTooltip hides tooltip") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
        Button* btn = button.get();
        manager.addElement(std::move(button));
        
        manager.showTooltip(btn, "Tooltip text");
        manager.hideTooltip();
        
        manager.render();
    }

    SECTION("Tooltip can be shown multiple times") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Btn1");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "Btn2");
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        manager.showTooltip(ptr1, "First tooltip");
        manager.hideTooltip();
        manager.showTooltip(ptr2, "Second tooltip");
        manager.hideTooltip();
    }

    SECTION("Multi-line tooltip panel fits all lines") {
        auto font = manager.getFontManager().loadFont(constants::kDefaultFontPath, constants::kTooltipFontSize);
        REQUIRE(font != nullptr);
        int lineHeight = TTF_GetFontHeight(font.get());

        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        Label widestRef(manager, 0, 0, "Second much longer line", constants::kTooltipFontSize);
        manager.showTooltip(btn, "First\nSecond much longer line\nThird");

        GUIElement* tip = manager.getActiveTooltip();
        REQUIRE(tip != nullptr);
        REQUIRE(tip->getWidth() == widestRef.getWidth() + 2 * constants::kTooltipPadding);
        REQUIRE(tip->getHeight() == 3 * lineHeight + 2 * constants::kTooltipPadding);

        manager.render();

        SECTION("Single-line tooltip shrinks back") {
            Label singleRef(manager, 0, 0, "Short", constants::kTooltipFontSize);
            manager.showTooltip(btn, "Short");
            tip = manager.getActiveTooltip();
            REQUIRE(tip != nullptr);
            REQUIRE(tip->getWidth() == singleRef.getWidth() + 2 * constants::kTooltipPadding);
            REQUIRE(tip->getHeight() == singleRef.getHeight() + 2 * constants::kTooltipPadding);
        }
    }
}

TEST_CASE("GUIManager Element Finding", "[gui_manager][finding]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("findElementAt finds element at coordinates") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "");
        auto panel = std::make_unique<Panel>(manager, 10, 100, 200, 100);
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        manager.addElement(std::move(panel));
        
        REQUIRE(manager.findElementAt(50, 25) == ptr1);
        REQUIRE(manager.findElementAt(175, 25) == ptr2);
        REQUIRE(manager.findElementAt(50, 150) == panelPtr);
    }

    SECTION("findElementAt returns nullptr for empty areas") {
        auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "");
        manager.addElement(std::move(btn));
        
        REQUIRE(manager.findElementAt(500, 500) == nullptr);
        REQUIRE(manager.findElementAt(200, 200) == nullptr);
        REQUIRE(manager.findElementAt(0, 0) == nullptr);
        REQUIRE(manager.findElementAt(150, 25) == nullptr);
    }

    SECTION("findElementAt finds child elements") {
        auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
        Panel* panelPtr = panel.get();
        
        auto childBtn = std::make_unique<Button>(manager, 10, 10, 80, 40, "");
        Button* childPtr = childBtn.get();
        panelPtr->addChild(std::move(childBtn));
        
        manager.addElement(std::move(panel));
        
        int childX = 50 + 10 + 40;
        int childY = 50 + 10 + 20;
        REQUIRE(manager.findElementAt(childX, childY) == childPtr);
        
        REQUIRE(manager.findElementAt(50 + 50, 50 + 100) == panelPtr);
    }

    SECTION("findElementAt returns nullptr when elements are hidden") {
        auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "Hidden");
        Button* ptr = btn.get();
        manager.addElement(std::move(btn));
        
        ptr->setVisible(false);
        
        REQUIRE(manager.findElementAt(50, 25) == nullptr);
    }
}

TEST_CASE("GUIManager Cleanup", "[gui_manager][cleanup]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("cleanup removes elements marked for deletion") {
        auto btn1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "");
        auto btn2 = std::make_unique<Button>(manager, 150, 10, 100, 40, "");
        Button* ptr1 = btn1.get();
        Button* ptr2 = btn2.get();
        manager.addElement(std::move(btn1));
        manager.addElement(std::move(btn2));
        
        ptr2->markForDeletion();
        
        manager.cleanup();
        
        REQUIRE(manager.findElementAt(50, 25) == ptr1);
        REQUIRE(manager.findElementAt(175, 25) == nullptr);
    }

    SECTION("cleanup clears focus when focused element is deleted") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));
        
        manager.setKeyboardFocus(ptr);
        REQUIRE(manager.getKeyboardFocus() == ptr);
        
        ptr->markForDeletion();
        manager.cleanup();
        
        REQUIRE(manager.getKeyboardFocus() == nullptr);
    }

    SECTION("cleanup releases mouse capture when captured element is deleted") {
        class CaptureTestElement : public GUIElement {
        public:
            CaptureTestElement(GUIManager& m, int x, int y, int w, int h)
                : GUIElement(m, x, y, w, h) {}
            ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
            void draw(SDL_Renderer*) override {}
        };
        
        auto elem = std::make_unique<CaptureTestElement>(manager, 10, 10, 100, 30);
        CaptureTestElement* ptr = elem.get();
        manager.addElement(std::move(elem));
        
        manager.captureMouse(ptr);
        
        ptr->markForDeletion();
        manager.cleanup();
    }
}

TEST_CASE("GUIManager Constructor", "[gui_manager][constructor]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();

    SECTION("GUIManager constructor accepts SDL_Renderer pointer") {
        GUIManager manager(renderer);
        REQUIRE(manager.getRenderer() == renderer);
    }

    SECTION("GUIManager initializes resource managers") {
        GUIManager manager(renderer);
        REQUIRE(manager.getFontManager().isInitialized());
        REQUIRE(manager.getTimerManager() != nullptr);
        REQUIRE(manager.getAnimationManager() != nullptr);
    }

    SECTION("GUIManager initializes default theme") {
        GUIManager manager(renderer);
        Theme& theme = manager.getTheme();
        REQUIRE_NOTHROW(theme.getStyle(ComponentType::Button));
        REQUIRE_NOTHROW(theme.getDefaultStyle());
    }
}