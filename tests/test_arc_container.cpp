#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/arc_container.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"

#include <cmath>

TEST_CASE("ArcContainer - construction", "[arc_container]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("constructor derives rect from center and radius") {
        ArcContainer arc(manager, 200, 150, 100);
        REQUIRE(arc.getX() == 100);
        REQUIRE(arc.getY() == 50);
        REQUIRE(arc.getWidth() == 200);
        REQUIRE(arc.getHeight() == 200);
    }

    SECTION("constructor with arc range") {
        ArcContainer arc(manager, 100, 100, 50, 0.0f, 180.0f);
        REQUIRE(arc.getWidth() == 100);
        REQUIRE(arc.getHeight() == 100);
    }

    SECTION("getComponentType returns ArcContainer") {
        ArcContainer arc(manager, 0, 0, 50);
        REQUIRE(std::string(arc.getComponentType()) == "ArcContainer");
    }
}

TEST_CASE("ArcContainer - contains", "[arc_container]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ArcContainer arc(manager, 200, 200, 100); // inner radius 50

    SECTION("center is outside inner radius") {
        REQUIRE_FALSE(arc.contains(200, 200));
    }

    SECTION("point on the ring at 0 degrees") {
        REQUIRE(arc.contains(300, 200));
    }

    SECTION("point on the ring at 90 degrees") {
        REQUIRE(arc.contains(200, 100));
    }

    SECTION("point on the ring at 180 degrees") {
        REQUIRE(arc.contains(100, 200));
    }

    SECTION("point on the ring at 270 degrees") {
        REQUIRE(arc.contains(200, 300));
    }

    SECTION("point beyond outer radius") {
        REQUIRE_FALSE(arc.contains(350, 200));
        REQUIRE_FALSE(arc.contains(200, 20));
    }

    SECTION("point inside inner radius") {
        REQUIRE_FALSE(arc.contains(220, 200));
        REQUIRE_FALSE(arc.contains(200, 230));
    }

    SECTION("point on the ring at a diagonal") {
        float d = 100.0f / std::sqrt(2.0f);
        int px = 200 + static_cast<int>(d);
        int py = 200 - static_cast<int>(d);
        REQUIRE(arc.contains(px, py));
    }
}

TEST_CASE("ArcContainer - arc range restricts contains", "[arc_container]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ArcContainer arc(manager, 200, 200, 100, 0.0f, 180.0f);

    SECTION("point inside 0..180 range") {
        REQUIRE(arc.contains(300, 200)); // 0 degrees (east)
        REQUIRE(arc.contains(200, 300)); // 90 degrees (south, screen coords)
        REQUIRE(arc.contains(100, 200)); // 180 degrees (west)
    }

    SECTION("point outside 0..180 range") {
        REQUIRE_FALSE(arc.contains(200, 100)); // 270 degrees (north)
    }

    SECTION("setArcRange restricts to new range") {
        arc.setArcRange(90.0f, 270.0f);
        REQUIRE(arc.contains(200, 300)); // 90 degrees
        REQUIRE(arc.contains(200, 100)); // 270 degrees
        REQUIRE_FALSE(arc.contains(300, 200)); // 0 degrees
    }
}

TEST_CASE("ArcContainer - setRadius", "[arc_container]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ArcContainer arc(manager, 100, 100, 50);

    arc.setRadius(80);
    REQUIRE(arc.getWidth() == 160);
    REQUIRE(arc.getHeight() == 160);
    REQUIRE(arc.getX() == 100 - 80);
    REQUIRE(arc.getY() == 100 - 80);

    // point now on the ring at 0 degrees
    REQUIRE(arc.contains(180, 100));
}

TEST_CASE("ArcContainer - addChildAtAngle", "[arc_container]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ArcContainer arc(manager, 200, 200, 100);

    SECTION("child at 0 degrees sits right of center") {
        auto child = std::make_unique<Button>(manager, 0, 0, 40, 20, "x");
        Button* childPtr = child.get();
        arc.addChildAtAngle(std::move(child), 0.0f);
        REQUIRE(childPtr->getParent() == &arc);

        // x = cx + cos(0)*r - w/2 = 200 + 100 - 20 = 280 (relative to window),
        // but children are relative to the parent: parent at (100, 100)
        REQUIRE(childPtr->getX() == 280 - arc.getX());
        REQUIRE(childPtr->getY() == 200 - arc.getY() - 10);
    }

    SECTION("child at 90 degrees sits south of center") {
        auto child = std::make_unique<Button>(manager, 0, 0, 40, 20, "x");
        Button* childPtr = child.get();
        arc.addChildAtAngle(std::move(child), 90.0f);
        // y = cy + sin(90)*r - h/2 = 200 + 100 - 10 (relative to parent)
        REQUIRE(childPtr->getX() == 200 - arc.getX() - 20);
        REQUIRE(childPtr->getY() == 200 + 100 - 10 - arc.getY());
    }

    SECTION("rotateChild rotates the child") {
        auto child = std::make_unique<Button>(manager, 0, 0, 40, 20, "x");
        Button* childPtr = child.get();
        arc.addChildAtAngle(std::move(child), 45.0f, true);
        REQUIRE(childPtr->getRotation() == 135.0); // angle + 90
    }

    SECTION("rotateChild=false leaves rotation at zero") {
        auto child = std::make_unique<Button>(manager, 0, 0, 40, 20, "x");
        Button* childPtr = child.get();
        arc.addChildAtAngle(std::move(child), 45.0f, false);
        REQUIRE(childPtr->getRotation() == 0.0);
    }

    SECTION("offset moves child outward") {
        auto child = std::make_unique<Button>(manager, 0, 0, 40, 20, "x");
        Button* childPtr = child.get();
        arc.addChildAtAngle(std::move(child), 0.0f, true, 30);
        REQUIRE(childPtr->getX() == 200 + 130 - 20 - arc.getX());
    }
}
