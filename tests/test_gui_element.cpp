#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/gui.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/style.hpp"

class TestableElement : public GUIElement {
public:
    TestableElement(GUIManager& manager, int x, int y, int width, int height)
        : GUIElement(manager, x, y, width, height) {}
    
    void draw(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_FRect r = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height)};
        SDL_RenderFillRect(renderer, &r);
    }
    
    ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
    
    bool isDirty() const { return m_isDirty; }
    
    void resetDirty() { m_isDirty = false; }
    
    bool testContains(int x, int y) { return contains(x, y); }
};

TEST_CASE("GUIElement Position and Geometry", "[gui_element][geometry]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("Constructor initializes position and dimensions correctly") {
        TestableElement element(manager, 10, 20, 100, 50);
        REQUIRE(element.getX() == 10);
        REQUIRE(element.getY() == 20);
        REQUIRE(element.getWidth() == 100);
        REQUIRE(element.getHeight() == 50);
    }
    
    SECTION("setSize() updates dimensions") {
        TestableElement element(manager, 0, 0, 50, 50);
        element.setSize(200, 100);
        REQUIRE(element.getWidth() == 200);
        REQUIRE(element.getHeight() == 100);
    }
    
    SECTION("getSize() returns correct values") {
        TestableElement element(manager, 0, 0, 150, 75);
        int w, h;
        element.getSize(w, h);
        REQUIRE(w == 150);
        REQUIRE(h == 75);
    }
    
    SECTION("setPosition() updates position") {
        TestableElement element(manager, 10, 20, 100, 100);
        element.setPosition(50, 60);
        REQUIRE(element.getX() == 50);
        REQUIRE(element.getY() == 60);
    }
    
    SECTION("getRelativePosition() returns local position") {
        TestableElement element(manager, 30, 40, 50, 50);
        SDL_Point pos = element.getRelativePosition();
        REQUIRE(pos.x == 30);
        REQUIRE(pos.y == 40);
    }
    
    SECTION("getAbsolutePosition() returns correct absolute position without parent") {
        TestableElement element(manager, 100, 200, 50, 50);
        SDL_Point pos = element.getAbsolutePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }
    
    SECTION("getAbsolutePosition() returns correct absolute position with parent") {
        auto parent = std::make_unique<Panel>(manager, 50, 60, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 20, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        SDL_Point pos = childPtr->getAbsolutePosition();
        REQUIRE(pos.x == 60);
        REQUIRE(pos.y == 80);
    }
    
    SECTION("getAbsolutePosition() works with nested hierarchy") {
        auto grandparent = std::make_unique<Panel>(manager, 100, 100, 300, 300);
        Panel* grandparentPtr = grandparent.get();
        manager.addElement(std::move(grandparent));
        
        auto parent = std::make_unique<Panel>(manager, 50, 50, 200, 200);
        Panel* parentPtr = parent.get();
        grandparentPtr->addChild(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 25, 25, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        SDL_Point pos = childPtr->getAbsolutePosition();
        REQUIRE(pos.x == 175);
        REQUIRE(pos.y == 175);
    }
    
    SECTION("contains() correctly detects point inside bounds") {
        TestableElement element(manager, 100, 100, 50, 50);
        REQUIRE(element.testContains(100, 100) == true);
        REQUIRE(element.testContains(125, 125) == true);
        REQUIRE(element.testContains(149, 149) == true);
        REQUIRE(element.testContains(99, 100) == false);
        REQUIRE(element.testContains(150, 100) == false);
        REQUIRE(element.testContains(100, 150) == false);
        REQUIRE(element.testContains(200, 200) == false);
    }
    
    SECTION("contains() works with parent offset") {
        auto parent = std::make_unique<Panel>(manager, 50, 50, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        REQUIRE(childPtr->testContains(60, 60) == true);
        REQUIRE(childPtr->testContains(109, 109) == true);
        REQUIRE(childPtr->testContains(50, 50) == false);
        REQUIRE(childPtr->testContains(110, 110) == false);
    }
    
    SECTION("Negative positions work correctly") {
        TestableElement element(manager, -50, -30, 100, 100);
        REQUIRE(element.getX() == -50);
        REQUIRE(element.getY() == -30);
        
        SDL_Point pos = element.getRelativePosition();
        REQUIRE(pos.x == -50);
        REQUIRE(pos.y == -30);
    }
    
    SECTION("Zero dimensions are valid") {
        TestableElement element(manager, 0, 0, 0, 0);
        REQUIRE(element.getWidth() == 0);
        REQUIRE(element.getHeight() == 0);
    }
}

TEST_CASE("GUIElement State Management", "[gui_element][state]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("Initial state is Normal") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.getState() == ElementState::Normal);
    }
    
    SECTION("setState()/getState() for ElementState") {
        TestableElement element(manager, 0, 0, 100, 100);
        
        element.setState(ElementState::Hover);
        REQUIRE(element.getState() == ElementState::Hover);
        
        element.setState(ElementState::Pressed);
        REQUIRE(element.getState() == ElementState::Pressed);
        
        element.setState(ElementState::Disabled);
        REQUIRE(element.getState() == ElementState::Disabled);
        
        element.setState(ElementState::Normal);
        REQUIRE(element.getState() == ElementState::Normal);
    }
    
    SECTION("setVisible()/isVisible() toggling works") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.isVisible() == true);
        
        element.setVisible(false);
        REQUIRE(element.isVisible() == false);
        
        element.setVisible(true);
        REQUIRE(element.isVisible() == true);
    }
    
    SECTION("setEnabled()/isEnabled() toggling works") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.isEnabled() == true);
        
        element.setEnabled(false);
        REQUIRE(element.isEnabled() == false);
        
        element.setEnabled(true);
        REQUIRE(element.isEnabled() == true);
    }
}

TEST_CASE("GUIElement Hierarchy", "[gui_element][hierarchy]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("getParent() returns null initially") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.getParent() == nullptr);
    }
    
    SECTION("addChild() adds children correctly") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        REQUIRE(parentPtr->getChildren().size() == 1);
        REQUIRE(childPtr->getParent() == parentPtr);
    }
    
    SECTION("getChildren() returns correct list") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child1 = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        auto child2 = std::make_unique<TestableElement>(manager, 70, 10, 50, 50);
        parentPtr->addChild(std::move(child1));
        parentPtr->addChild(std::move(child2));
        
        const auto& children = parentPtr->getChildren();
        REQUIRE(children.size() == 2);
    }
    
    SECTION("countDescendants() returns correct count") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        REQUIRE(parentPtr->countDescendants() == 0);
        
        auto child1 = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        auto child2 = std::make_unique<TestableElement>(manager, 70, 10, 50, 50);
        parentPtr->addChild(std::move(child1));
        parentPtr->addChild(std::move(child2));
        
        REQUIRE(parentPtr->countDescendants() == 2);
    }
    
    SECTION("Nested children (child with its own children)") {
        auto grandparent = std::make_unique<Panel>(manager, 0, 0, 300, 300);
        Panel* grandparentPtr = grandparent.get();
        manager.addElement(std::move(grandparent));
        
        auto parent = std::make_unique<Panel>(manager, 10, 10, 200, 200);
        Panel* parentPtr = parent.get();
        grandparentPtr->addChild(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        REQUIRE(grandparentPtr->countDescendants() == 2);
        REQUIRE(parentPtr->countDescendants() == 1);
        REQUIRE(parentPtr->getParent() == grandparentPtr);
        REQUIRE(childPtr->getParent() == parentPtr);
    }
    
    SECTION("clearChildren() removes all children") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child1 = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        auto child2 = std::make_unique<TestableElement>(manager, 70, 10, 50, 50);
        auto child3 = std::make_unique<TestableElement>(manager, 130, 10, 50, 50);
        parentPtr->addChild(std::move(child1));
        parentPtr->addChild(std::move(child2));
        parentPtr->addChild(std::move(child3));
        
        REQUIRE(parentPtr->getChildren().size() == 3);
        
        parentPtr->clearChildren();
        
        REQUIRE(parentPtr->getChildren().size() == 0);
        REQUIRE(parentPtr->countDescendants() == 0);
    }
    
    SECTION("Adding same child twice does not duplicate") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        REQUIRE(parentPtr->getChildren().size() == 1);
        REQUIRE(childPtr->getParent() == parentPtr);
    }
}

TEST_CASE("GUIElement ID and Tooltip", "[gui_element][id_tooltip]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("setID()/getID() work correctly") {
        TestableElement element(manager, 0, 0, 100, 100);
        
        element.setID("test_element");
        REQUIRE(element.getID() == "test_element");
        
        element.setID("another_id");
        REQUIRE(element.getID() == "another_id");
    }
    
    SECTION("Empty ID by default") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.getID() == "");
    }
    
    SECTION("setTooltip() can be set without error") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE_NOTHROW(element.setTooltip("This is a tooltip"));
        REQUIRE_NOTHROW(element.setTooltip(""));
    }
}

TEST_CASE("GUIElement Deletion Marking", "[gui_element][deletion]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("isMarkedForDeletion() returns false initially") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.isMarkedForDeletion() == false);
    }
    
    SECTION("markForDeletion() marks element") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.markForDeletion();
        REQUIRE(element.isMarkedForDeletion() == true);
    }
    
    SECTION("cleanup() removes marked children") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child1 = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        auto child2 = std::make_unique<TestableElement>(manager, 70, 10, 50, 50);
        TestableElement* child1Ptr = child1.get();
        TestableElement* child2Ptr = child2.get();
        parentPtr->addChild(std::move(child1));
        parentPtr->addChild(std::move(child2));
        
        REQUIRE(parentPtr->getChildren().size() == 2);
        
        child1Ptr->markForDeletion();
        parentPtr->cleanup();
        
        REQUIRE(parentPtr->getChildren().size() == 1);
        REQUIRE(child2Ptr->isMarkedForDeletion() == false);
    }
    
    SECTION("cleanup() preserves unmarked children") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        parentPtr->addChild(std::move(child));
        
        parentPtr->cleanup();
        
        REQUIRE(parentPtr->getChildren().size() == 1);
    }
}

TEST_CASE("GUIElement Keyboard Focus", "[gui_element][focus]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("canGetKeyboardFocus() returns false initially") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.canGetKeyboardFocus() == false);
    }
    
    SECTION("setCanGetKeyboardFocus(true) enables focus capability") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.setCanGetKeyboardFocus(true);
        REQUIRE(element.canGetKeyboardFocus() == true);
        
        element.setCanGetKeyboardFocus(false);
        REQUIRE(element.canGetKeyboardFocus() == false);
    }
    
    SECTION("hasKeyboardFocus() returns correct state") {
        auto element = std::make_unique<TestableElement>(manager, 0, 0, 100, 100);
        TestableElement* elementPtr = element.get();
        elementPtr->setCanGetKeyboardFocus(true);
        manager.addElement(std::move(element));
        
        REQUIRE(elementPtr->hasKeyboardFocus() == false);
        
        manager.setKeyboardFocus(elementPtr);
        REQUIRE(elementPtr->hasKeyboardFocus() == true);
        
        manager.setKeyboardFocus(nullptr);
        REQUIRE(elementPtr->hasKeyboardFocus() == false);
    }
}

TEST_CASE("GUIElement Styles", "[gui_element][styles]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("setBackgroundColor() sets style correctly") {
        TestableElement element(manager, 0, 0, 100, 100);
        SDL_Color color = {255, 0, 0, 255};
        REQUIRE_NOTHROW(element.setBackgroundColor(ElementState::Normal, color));
    }
    
    SECTION("setTextColor() sets style correctly") {
        TestableElement element(manager, 0, 0, 100, 100);
        SDL_Color color = {0, 255, 0, 255};
        REQUIRE_NOTHROW(element.setTextColor(ElementState::Hover, color));
    }
    
    SECTION("setBorder() sets style correctly") {
        TestableElement element(manager, 0, 0, 100, 100);
        SDL_Color color = {0, 0, 255, 255};
        REQUIRE_NOTHROW(element.setBorder(ElementState::Pressed, color, 2));
    }
    
    SECTION("setTexture() sets style correctly") {
        TestableElement element(manager, 0, 0, 100, 100);
        auto texture = helper.makeStubTexture(50, 50);
        REQUIRE_NOTHROW(element.setTexture(ElementState::Normal, texture));
    }
    
    SECTION("setStyle() for different ElementStates") {
        TestableElement element(manager, 0, 0, 100, 100);
        
        Style normalStyle;
        normalStyle.backgroundColor = SDL_Color{100, 100, 100, 255};
        
        Style hoverStyle;
        hoverStyle.backgroundColor = SDL_Color{150, 150, 150, 255};
        
        Style pressedStyle;
        pressedStyle.backgroundColor = SDL_Color{80, 80, 80, 255};
        
        REQUIRE_NOTHROW(element.setStyle(ElementState::Normal, normalStyle));
        REQUIRE_NOTHROW(element.setStyle(ElementState::Hover, hoverStyle));
        REQUIRE_NOTHROW(element.setStyle(ElementState::Pressed, pressedStyle));
        REQUIRE_NOTHROW(element.setStyle(ElementState::Disabled, Style{}));
    }
    
    }

TEST_CASE("GUIElement Clipping", "[gui_element][clipping]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("setClipChildren() works") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.setClipChildren(false);
        element.setClipChildren(true);
    }
}

TEST_CASE("GUIElement Dirty Flag", "[gui_element][dirty]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("Element is initially dirty") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.isDirty() == true);
    }
    
    SECTION("markDirty() marks element dirty") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.resetDirty();
        REQUIRE(element.isDirty() == false);
        
        element.markDirty();
        REQUIRE(element.isDirty() == true);
    }
    
    SECTION("setPosition() marks element dirty") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.resetDirty();
        element.setPosition(50, 50);
        REQUIRE(element.isDirty() == true);
    }
    
    SECTION("setSize() marks element dirty") {
        TestableElement element(manager, 0, 0, 100, 100);
        element.resetDirty();
        element.setSize(200, 200);
        REQUIRE(element.isDirty() == true);
    }
    
    SECTION("markDirtyRecursively() marks element and all children") {
        auto parent = std::make_unique<TestableElement>(manager, 0, 0, 200, 200);
        TestableElement* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        parentPtr->resetDirty();
        childPtr->resetDirty();
        
        parentPtr->markDirtyRecursively();
        
        REQUIRE(parentPtr->isDirty() == true);
        REQUIRE(childPtr->isDirty() == true);
    }
}

TEST_CASE("GUIElement Overlay", "[gui_element][overlay]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("isOverlay() returns false by default") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.isOverlay() == false);
    }
}

TEST_CASE("GUIElement findElementAt", "[gui_element][find]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("findElementAt() finds element at position") {
        auto element = std::make_unique<TestableElement>(manager, 100, 100, 50, 50);
        TestableElement* elementPtr = element.get();
        manager.addElement(std::move(element));
        
        REQUIRE(elementPtr->findElementAt(100, 100) == elementPtr);
        REQUIRE(elementPtr->findElementAt(125, 125) == elementPtr);
        REQUIRE(elementPtr->findElementAt(50, 50) == nullptr);
    }
    
    SECTION("findElementAt() with invisible element returns nullptr") {
        auto element = std::make_unique<TestableElement>(manager, 100, 100, 50, 50);
        TestableElement* elementPtr = element.get();
        elementPtr->setVisible(false);
        manager.addElement(std::move(element));
        
        REQUIRE(elementPtr->findElementAt(100, 100) == nullptr);
    }
    
    SECTION("findElementAt() finds child elements") {
        auto parent = std::make_unique<Panel>(manager, 50, 50, 200, 200);
        Panel* parentPtr = parent.get();
        manager.addElement(std::move(parent));
        
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 50, 50);
        TestableElement* childPtr = child.get();
        parentPtr->addChild(std::move(child));
        
        REQUIRE(parentPtr->findElementAt(60, 60) == childPtr);
        REQUIRE(parentPtr->findElementAt(55, 55) == parentPtr);
    }
}

TEST_CASE("GUIElement ComponentType ID", "[gui_element][type]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("getComponentTypeIdId() returns Unknown for test stubs") {
        TestableElement element(manager, 0, 0, 100, 100);
        REQUIRE(element.getComponentTypeId() == ComponentType::Unknown);
    }
    
    SECTION("Panel component type ID") {
        Panel panel(manager, 0, 0, 100, 100);
        REQUIRE(panel.getComponentTypeId() == ComponentType::Panel);
    }
}

TEST_CASE("GUIElement Right Click", "[gui_element][right_click]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestableElement element(manager, 100, 100, 50, 40);

    SECTION("callback fires with click position and consumes the event") {
        int calls = 0;
        float rx = -1.0f, ry = -1.0f;
        element.setOnRightClickCallback([&](GUIElement* e, float x, float y) {
            ++calls;
            rx = x;
            ry = y;
            REQUIRE(e == &element);
        });

        bool consumed = element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 120, 110));

        REQUIRE(consumed);
        REQUIRE(calls == 1);
        REQUIRE(rx == 120.0f);
        REQUIRE(ry == 110.0f);
    }

    SECTION("without callback the event is not consumed") {
        REQUIRE_FALSE(element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 120, 110)));
    }

    SECTION("RMB outside the element does not fire") {
        int calls = 0;
        element.setOnRightClickCallback([&](GUIElement*, float, float) { ++calls; });

        REQUIRE_FALSE(element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 10, 10)));
        REQUIRE(calls == 0);
    }

    SECTION("LMB does not fire the right-click callback") {
        int calls = 0;
        element.setOnRightClickCallback([&](GUIElement*, float, float) { ++calls; });

        element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 120, 110));
        element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 120, 110));

        REQUIRE(calls == 0);
    }

    SECTION("disabled element does not fire") {
        int calls = 0;
        element.setOnRightClickCallback([&](GUIElement*, float, float) { ++calls; });
        element.setEnabled(false);

        element.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 120, 110));

        REQUIRE(calls == 0);
    }

    SECTION("nearest element with callback wins (child consumed, parent skipped)") {
        TestableElement parent(manager, 200, 200, 100, 100);
        auto child = std::make_unique<TestableElement>(manager, 10, 10, 30, 30);
        TestableElement* childPtr = child.get();
        parent.addChild(std::move(child));

        int parentCalls = 0, childCalls = 0;
        parent.setOnRightClickCallback([&](GUIElement*, float, float) { ++parentCalls; });
        childPtr->setOnRightClickCallback([&](GUIElement*, float, float) { ++childCalls; });

        // Click inside the child (child coords are relative to parent)
        REQUIRE(childPtr->handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 215, 215)));
        REQUIRE(childCalls == 1);
        REQUIRE(parentCalls == 0);

        // Click inside the parent but outside the child: parent fires
        REQUIRE(parent.handleEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 280, 280)));
        REQUIRE(parentCalls == 1);
    }
}