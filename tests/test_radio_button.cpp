#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/radio_button.hpp"
#include "../src/radio_group.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("RadioButton standalone behaviour", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initial state is unselected") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE_FALSE(rb.isSelected());
    }

    SECTION("Programmatically setting selection invokes callback") {
        RadioButton rb(manager, 0, 0, 20, 20);
        bool changed = false;
        rb.setOnChange([&](RadioButton*, bool) { changed = true; });

        rb.setSelected(true);
        REQUIRE(rb.isSelected());
        REQUIRE(changed);

        changed = false;
        rb.setSelected(true);
        REQUIRE(rb.isSelected());
        REQUIRE_FALSE(changed);

        rb.setSelected(false);
        REQUIRE_FALSE(rb.isSelected());
        REQUIRE(changed);
    }

    SECTION("Position and dimensions") {
        RadioButton rb(manager, 10, 20, 100, 50);
        REQUIRE(rb.getX() == 10);
        REQUIRE(rb.getY() == 20);
        REQUIRE(rb.getWidth() == 100);
        REQUIRE(rb.getHeight() == 50);
    }

    SECTION("Component type identifier") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE(std::string(rb.getComponentType()) == "RadioButton");
    }

    SECTION("Enabled state affects selection") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        manager.addElement(std::move(rb));

        REQUIRE(rb_ptr->isEnabled());

        rb_ptr->setEnabled(false);
        REQUIRE_FALSE(rb_ptr->isEnabled());

        // Programowe ustawienie selekcji na disabled RadioButton powinno działać
        // (setEnabled wpływa tylko na obsługę zdarzeń)
        rb_ptr->setSelected(true);
        REQUIRE(rb_ptr->isSelected());
    }

    SECTION("Hidden state does not prevent programmatic selection") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        manager.addElement(std::move(rb));

        REQUIRE(rb_ptr->isVisible());

        rb_ptr->setVisible(false);
        REQUIRE_FALSE(rb_ptr->isVisible());

        // Programowe ustawienie selekcji na hidden RadioButton powinno działać
        rb_ptr->setSelected(true);
        REQUIRE(rb_ptr->isSelected());
    }
}

TEST_CASE("RadioButton event handling limitations", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    // UWAGA: Testy symulacji kliknięcia są ograniczone przez implementację GUIElement::handleEvent,
    // która używa SDL_GetMouseState() zamiast pozycji ze zdarzenia.
    // W środowisku testowym (bez prawdziwej myszy) SDL_GetMouseState() zwraca (0, 0),
    // więc m_isHovered nigdy nie jest ustawiane na true, co uniemożliwia przejście
    // Pressed -> Hover po MOUSEBUTTONUP.
    //
    // Poniższe testy weryfikują zachowanie przy programowej selekcji.

    SECTION("Click outside RadioButton does not affect selection") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        manager.addElement(std::move(rb));

        // Kliknięcie poza obszarem RadioButton (pozycja 50, 50)
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        // RadioButton powinien pozostać niezaznaczony
        REQUIRE_FALSE(rb_ptr->isSelected());
    }

    SECTION("Programmatic selection works regardless of mouse state") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        bool changed = false;
        rb_ptr->setOnChange([&](RadioButton*, bool) { changed = true; });
        manager.addElement(std::move(rb));

        // Programowa selekcja powinna działać niezależnie od stanu myszy
        rb_ptr->setSelected(true);
        REQUIRE(rb_ptr->isSelected());
        REQUIRE(changed);
    }

    SECTION("Already selected RadioButton stays selected on click attempt") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        rb_ptr->setSelected(true);
        REQUIRE(rb_ptr->isSelected());

        int callbackCount = 0;
        rb_ptr->setOnChange([&](RadioButton*, bool) { callbackCount++; });
        manager.addElement(std::move(rb));

        // Próba kliknięcia (która nie działa w testach) nie powinna zmienić stanu
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        // Stan powinien pozostać niezmieniony
        REQUIRE(rb_ptr->isSelected());
        REQUIRE(callbackCount == 0);
    }
}

TEST_CASE("RadioButton with RadioGroup - mutual exclusivity", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Selecting one button deselects others in group") {
        // Tworzymy grupę
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* group_ptr = group.get();

        // Tworzymy przyciski i dodajemy do grupy
        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 40, 10, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 70, 10, 20, 20);

        RadioButton* rb1_ptr = rb1.get();
        RadioButton* rb2_ptr = rb2.get();
        RadioButton* rb3_ptr = rb3.get();

        // Dodajemy przyciski jako dzieci grupy
        group->addChild(std::move(rb1));
        group->addChild(std::move(rb2));
        group->addChild(std::move(rb3));

        manager.addElement(std::move(group));

        // Początkowo żaden nie jest wybrany
        REQUIRE_FALSE(rb1_ptr->isSelected());
        REQUIRE_FALSE(rb2_ptr->isSelected());
        REQUIRE_FALSE(rb3_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == nullptr);

        // Wybieramy pierwszy programowo
        rb1_ptr->setSelected(true);
        REQUIRE(rb1_ptr->isSelected());
        REQUIRE_FALSE(rb2_ptr->isSelected());
        REQUIRE_FALSE(rb3_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == rb1_ptr);

        // Wybieramy drugi programowo - pierwszy powinien być odznaczony
        rb2_ptr->setSelected(true);
        REQUIRE_FALSE(rb1_ptr->isSelected());
        REQUIRE(rb2_ptr->isSelected());
        REQUIRE_FALSE(rb3_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == rb2_ptr);

        // Wybieramy trzeci programowo - drugi powinien być odznaczony
        rb3_ptr->setSelected(true);
        REQUIRE_FALSE(rb1_ptr->isSelected());
        REQUIRE_FALSE(rb2_ptr->isSelected());
        REQUIRE(rb3_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == rb3_ptr);
    }

    SECTION("getSelectedButton returns nullptr when nothing selected") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* group_ptr = group.get();

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        group->addChild(std::move(rb1));

        manager.addElement(std::move(group));

        REQUIRE(group_ptr->getSelectedButton() == nullptr);
    }

    SECTION("Deselecting selected button via setSelected(false)") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* group_ptr = group.get();

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb1_ptr = rb1.get();
        group->addChild(std::move(rb1));

        manager.addElement(std::move(group));

        rb1_ptr->setSelected(true);
        REQUIRE(rb1_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == rb1_ptr);

        // Odznaczamy programowo
        rb1_ptr->setSelected(false);
        REQUIRE_FALSE(rb1_ptr->isSelected());
        REQUIRE(group_ptr->getSelectedButton() == nullptr);
    }
}

TEST_CASE("RadioButton callback details", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Callback receives pointer to self") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        RadioButton* callbackSender = nullptr;

        rb_ptr->setOnChange([&](RadioButton* sender, bool) {
            callbackSender = sender;
        });
        manager.addElement(std::move(rb));

        rb_ptr->setSelected(true);
        REQUIRE(callbackSender == rb_ptr);
    }

    SECTION("Callback receives correct selection state") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        
        bool lastState = false;
        int callCount = 0;
        
        rb_ptr->setOnChange([&](RadioButton*, bool selected) {
            lastState = selected;
            callCount++;
        });
        manager.addElement(std::move(rb));

        rb_ptr->setSelected(true);
        REQUIRE(callCount == 1);
        REQUIRE(lastState == true);

        rb_ptr->setSelected(false);
        REQUIRE(callCount == 2);
        REQUIRE(lastState == false);
    }

    SECTION("Multiple callbacks - last one wins") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();

        int callback1Count = 0;
        int callback2Count = 0;

        rb_ptr->setOnChange([&](RadioButton*, bool) { callback1Count++; });
        rb_ptr->setOnChange([&](RadioButton*, bool) { callback2Count++; });

        manager.addElement(std::move(rb));

        rb_ptr->setSelected(true);
        REQUIRE(callback1Count == 0);
        REQUIRE(callback2Count == 1);
    }

    SECTION("Empty callback does not crash") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        manager.addElement(std::move(rb));

        // Nie ustawiamy callbacka - nie powinno crashować
        REQUIRE_NOTHROW(rb_ptr->setSelected(true));
        REQUIRE(rb_ptr->isSelected());
    }
}

TEST_CASE("RadioButton group callback integration", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Callback receives correct selection state when switching buttons") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 40, 10, 20, 20);

        RadioButton* rb1_ptr = rb1.get();
        RadioButton* rb2_ptr = rb2.get();

        int rb1CallbackCount = 0;
        int rb2CallbackCount = 0;
        bool lastRb1State = false;
        bool lastRb2State = false;

        rb1_ptr->setOnChange([&](RadioButton*, bool selected) {
            rb1CallbackCount++;
            lastRb1State = selected;
        });
        rb2_ptr->setOnChange([&](RadioButton*, bool selected) {
            rb2CallbackCount++;
            lastRb2State = selected;
        });

        group->addChild(std::move(rb1));
        group->addChild(std::move(rb2));

        manager.addElement(std::move(group));

        // Wybieramy rb1
        rb1_ptr->setSelected(true);
        REQUIRE(rb1CallbackCount == 1);
        REQUIRE(lastRb1State == true);
        REQUIRE(rb2CallbackCount == 0);

        // Wybieramy rb2 - rb1 powinien zostać odznaczony z callbackiem
        rb2_ptr->setSelected(true);
        REQUIRE(rb1CallbackCount == 2); // Drugi callback z false
        REQUIRE(lastRb1State == false);
        REQUIRE(rb2CallbackCount == 1);
        REQUIRE(lastRb2State == true);
    }
}

TEST_CASE("RadioButton ID and identification", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("RadioButton can have ID") {
        RadioButton rb(manager, 0, 0, 20, 20);
        rb.setID("option1");
        REQUIRE(rb.getID() == "option1");
    }

    SECTION("Default ID is empty") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE(rb.getID().empty());
    }
}

TEST_CASE("RadioButton selection persistence", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Selected state persists across multiple operations") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb_ptr = rb.get();
        manager.addElement(std::move(rb));

        rb_ptr->setSelected(true);
        REQUIRE(rb_ptr->isSelected());

        // Wykonaj kilka operacji, które nie powinny wpłynąć na selekcję
        rb_ptr->setPosition(20, 20);
        REQUIRE(rb_ptr->isSelected());

        rb_ptr->setSize(30, 30);
        REQUIRE(rb_ptr->isSelected());

        rb_ptr->setID("test_radio");
        REQUIRE(rb_ptr->isSelected());
    }
}
