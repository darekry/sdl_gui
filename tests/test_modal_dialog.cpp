#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/composite/dialog_box.hpp"
#include "../src/composite/file_dialog.hpp"
#include "../src/text_input.hpp"
#include "../src/gui_manager.hpp"
#include "../src/theme.hpp"

#include "std.hpp"

namespace fs = std::filesystem;

static TextInput* findFilenameInput(FileDialog* dlg) {
    for (const auto& child : dlg->getChildren()) {
        if (auto* ti = dynamic_cast<TextInput*>(child.get())) {
            return ti;
        }
    }
    return nullptr;
}

TEST_CASE("ModalDialog - DialogBox Esc cancels with callback(-1)", "[modal][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("custom (int index): Esc reports -1") {
        int cbValue = 42;
        auto dialog = DialogBox::createCustom(manager, "Sure?", {"Tak", "Nie"},
            [&cbValue](int idx) { cbValue = idx; });
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));

        REQUIRE(raw->isOpen());
        manager.setKeyboardFocus(raw);
        bool consumed = manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));

        REQUIRE(consumed);
        REQUIRE(cbValue == -1);
        REQUIRE(raw->getLastClickedButton() == -1);
        REQUIRE(!raw->isOpen());
        REQUIRE(raw->isMarkedForDeletion());
    }

    SECTION("confirm (bool): Esc reports false") {
        bool cbValue = true;
        auto dialog = DialogBox::createConfirm(manager, "Sure?", "Tak", "Nie",
            [&cbValue](bool confirmed) { cbValue = confirmed; });
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));

        manager.setKeyboardFocus(raw);
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));

        REQUIRE(!cbValue);
        REQUIRE(raw->getLastClickedButton() == -1);
        REQUIRE(!raw->isOpen());
    }
}

TEST_CASE("ModalDialog - DialogBox Enter is a no-op (buttons keep semantics)", "[modal][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    bool cbFired = false;
    auto dialog = DialogBox::createAlert(manager, "Hi!", "OK",
        [&cbFired](int) { cbFired = true; });
    DialogBox* raw = dialog.get();
    manager.addElement(std::move(dialog));

    manager.setKeyboardFocus(raw);
    bool consumed = manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN));

    // Consumed by the modal (no leak to elements behind), but no action.
    REQUIRE(consumed);
    REQUIRE(!cbFired);
    REQUIRE(raw->isOpen());
    REQUIRE(!raw->isMarkedForDeletion());
}

TEST_CASE("ModalDialog - FileDialog Esc closes without callback", "[modal][filedialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    fs::path tmp = fs::temp_directory_path() / "modal_esc_test";
    fs::create_directories(tmp);

    bool cbFired = false;
    FileDialog* raw = FileDialog::createSave(manager, "Save", [&cbFired](const std::string&) {
        cbFired = true;
    }, tmp.string());
    REQUIRE(raw != nullptr);
    REQUIRE(raw->isOpen());

    manager.setKeyboardFocus(raw);
    bool consumed = manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));

    REQUIRE(consumed);
    REQUIRE(!cbFired);
    REQUIRE(!raw->isOpen());
    REQUIRE(raw->isMarkedForDeletion());

    manager.setKeyboardFocus(nullptr);
    manager.cleanup();
    fs::remove_all(tmp);
}

TEST_CASE("ModalDialog - FileDialog Enter confirms save", "[modal][filedialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    fs::path tmp = fs::temp_directory_path() / "modal_enter_test";
    fs::create_directories(tmp);

    std::string got;
    FileDialog* raw = FileDialog::createSave(manager, "Save", [&got](const std::string& p) {
        got = p;
    }, tmp.string());
    REQUIRE(raw != nullptr);

    TextInput* input = findFilenameInput(raw);
    REQUIRE(input != nullptr);
    input->setText(std::string{"out.txt"});

    // Focus the dialog itself (not the input): the input would consume
    // Enter on its own, the confirm path fires when no child takes it.
    manager.setKeyboardFocus(raw);
    bool consumed = manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN));

    REQUIRE(consumed);
    REQUIRE(got == (tmp / "out.txt").string());
    REQUIRE(!raw->isOpen());
    REQUIRE(raw->isMarkedForDeletion());

    manager.setKeyboardFocus(nullptr);
    manager.cleanup();
    fs::remove_all(tmp);
}

TEST_CASE("ModalDialog - overlay, close and centerInViewport", "[modal][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("both dialogs are overlays") {
        auto confirm = DialogBox::createConfirm(manager, "Sure?");
        REQUIRE(confirm->isOverlay());
        fs::path tmp = fs::temp_directory_path() / "modal_overlay_test";
        fs::create_directories(tmp);
        FileDialog* fd = FileDialog::createOpen(manager, "Open", [](const std::string&) {}, tmp.string());
        REQUIRE(fd->isOverlay());
        manager.setKeyboardFocus(nullptr);
        manager.cleanup();
        fs::remove_all(tmp);
    }

    SECTION("close() marks for deletion") {
        auto dialog = DialogBox::createAlert(manager, "Hi!");
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));
        REQUIRE(raw->isOpen());
        raw->close();
        REQUIRE(!raw->isOpen());
        REQUIRE(raw->isMarkedForDeletion());
    }

    SECTION("centerInViewport centers in the manager viewport") {
        int winW = 0, winH = 0;
        manager.getWindowSize(winW, winH);
        auto dialog = DialogBox::createAlert(manager, "Hi!", "OK", nullptr, 350, 120);
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));
        raw->centerInViewport();
        REQUIRE(raw->getX() == (winW - 350) / 2);
        REQUIRE(raw->getY() == (winH - 120) / 2);
    }
}

TEST_CASE("ModalDialog - createConfirm API unchanged", "[modal][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto dialog = DialogBox::createConfirm(manager, "Sure?", "Tak", "Nie");
    DialogBox* raw = dialog.get();
    manager.addElement(std::move(dialog));

    REQUIRE(raw->getDialogType() == DialogBox::DialogType::Confirm);
    REQUIRE(raw->isOpen());
    REQUIRE(raw->getLastClickedButton() == -1);
    // Message label + 2 buttons (StackLayout strip, pinned by test_anchor).
    REQUIRE(raw->getChildren().size() == 3);
}

TEST_CASE("ModalDialog - overlay pass actually paints the dialog", "[modal][dialog][pixel]") {
    // Regression: after the focus-outline fix made the base renderOverlay()
    // a no-op, modals hit-tested but stayed invisible (smoke tests only
    // check for crashes). The ModalDialog override must paint them.
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        Uint8* p = static_cast<Uint8*>(surf->pixels);
        auto result = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
        SDL_DestroySurface(surf);
        return result;
    };

    auto dialog = DialogBox::createAlert(manager, "Hi!", "OK", nullptr, 400, 150);
    DialogBox* raw = dialog.get();
    manager.addElement(std::move(dialog));

    manager.update();
    manager.cleanup();
    manager.render();

    // createAlert centers 400x150 in 800x600 -> (200,225); sample well
    // inside the dialog face, away from the message label and buttons.
    REQUIRE(raw->getX() == 200);
    REQUIRE(raw->getY() == 225);
    auto px = readPixel(210, 235);
    REQUIRE(px[3] > 200);  // opaque, not the transparent backbuffer
    REQUIRE(px[0] == 240);
    REQUIRE(px[1] == 240);
    REQUIRE(px[2] == 240);
}
