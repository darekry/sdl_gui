#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_editable.hpp"
#include "../src/gui_manager.hpp"

// Minimal concrete subclass exposing TextEditable's protected logic for testing.
class TestEditable : public TextEditable {
public:
    using TextEditable::TextEditable;

    bool copy() { return handleClipboardCopy(); }
    bool paste() { return handleClipboardPaste(); }
    bool cut() { return handleClipboardCut(); }
    bool del() { return handleDeleteWithSelection(); }
    bool backspace() { return handleBackspaceWithSelection(); }
    void type(const char* text) { handleTextInputWithSelection(text); }
    void deleteSelected() { deleteSelection(); }

    void setCursor(size_t pos) { m_cursorPos = pos; }
    size_t getCursor() const { return m_cursorPos; }

    void blink() { updateCursorBlink(); }
    void resetBlink() { resetCursorBlink(); }
    bool isCursorShown() const { return m_showCursor; }

protected:
    void updateTextOffset() override {}
    void refreshTextTexture() override {}
    void markNeedsUpdate() override {}
};

TEST_CASE("TextEditable - text access", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);

    SECTION("initial text is empty") {
        REQUIRE(editable.getText().empty());
    }

    SECTION("setText stores text") {
        editable.setText("hello");
        REQUIRE(editable.getText() == "hello");
    }

    SECTION("setText with same text does not fire callback") {
        editable.setText("abc");
        int calls = 0;
        editable.setOnTextChanged([&](TextEditable*) { ++calls; });
        editable.setText("abc");
        REQUIRE(calls == 0);
    }

    SECTION("setText fires onTextChanged") {
        int calls = 0;
        TextEditable* source = nullptr;
        editable.setOnTextChanged([&](TextEditable* e) { ++calls; source = e; });
        editable.setText("new");
        REQUIRE(calls == 1);
        REQUIRE(source == &editable);
    }

    SECTION("setText clamps cursor to new length") {
        editable.setText("long text");
        editable.setCursor(9);
        editable.setText("ab");
        REQUIRE(editable.getCursor() == 2);
    }

    SECTION("getText returns reference to internal text") {
        editable.setText("x");
        const std::string& ref = editable.getText();
        editable.setText("y");
        REQUIRE(ref == "y");
    }
}

TEST_CASE("TextEditable - selection", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);
    editable.setText("hello world");

    SECTION("no selection initially") {
        REQUIRE_FALSE(editable.hasSelection());
        REQUIRE(editable.getSelection().empty());
    }

    SECTION("setSelection creates selection") {
        editable.setSelection(1, 4);
        REQUIRE(editable.hasSelection());
        REQUIRE(editable.getSelection() == "ell");
    }

    SECTION("reversed selection returns same range") {
        editable.setSelection(4, 1);
        REQUIRE(editable.getSelection() == "ell");
    }

    SECTION("selection is clamped to text length") {
        editable.setSelection(2, 999);
        REQUIRE(editable.getSelection() == "llo world");
    }

    SECTION("single point selection is not a selection") {
        editable.setSelection(3, 3);
        REQUIRE_FALSE(editable.hasSelection());
    }

    SECTION("clearSelection resets") {
        editable.setSelection(0, 5);
        editable.clearSelection();
        REQUIRE_FALSE(editable.hasSelection());
        REQUIRE(editable.getSelection().empty());
    }

    SECTION("UTF-8 selection counts characters not bytes") {
        editable.setText("ząb");
        editable.setSelection(0, 2);
        REQUIRE(editable.getSelection() == "zą");
    }
}

TEST_CASE("TextEditable - deleteSelection", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);
    editable.setText("hello world");

    SECTION("deletes the selected range") {
        editable.setSelection(0, 5);
        editable.deleteSelected();
        REQUIRE(editable.getText() == " world");
        REQUIRE_FALSE(editable.hasSelection());
    }

    SECTION("cursor moves to range start") {
        editable.setCursor(9);
        editable.setSelection(6, 11);
        editable.deleteSelected();
        REQUIRE(editable.getText() == "hello ");
        REQUIRE(editable.getCursor() == 6);
    }

    SECTION("deleteSelection without selection is a no-op") {
        editable.deleteSelected();
        REQUIRE(editable.getText() == "hello world");
    }
}

TEST_CASE("TextEditable - delete and backspace", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);
    editable.setText("hello");

    SECTION("delete removes char at cursor") {
        editable.setCursor(1);
        REQUIRE(editable.del());
        REQUIRE(editable.getText() == "hllo");
        REQUIRE(editable.getCursor() == 1);
    }

    SECTION("delete at end returns false") {
        editable.setCursor(5);
        REQUIRE_FALSE(editable.del());
    }

    SECTION("delete with selection removes selection") {
        editable.setSelection(1, 3);
        REQUIRE(editable.del());
        REQUIRE(editable.getText() == "hlo");
    }

    SECTION("backspace removes char before cursor") {
        editable.setCursor(2);
        REQUIRE(editable.backspace());
        REQUIRE(editable.getText() == "hllo");
        REQUIRE(editable.getCursor() == 1);
    }

    SECTION("backspace at start returns false") {
        editable.setCursor(0);
        REQUIRE_FALSE(editable.backspace());
    }

    SECTION("backspace with selection removes selection") {
        editable.setSelection(1, 4);
        REQUIRE(editable.backspace());
        REQUIRE(editable.getText() == "ho");
    }

    SECTION("backspace handles UTF-8 characters") {
        editable.setText("ąęść");
        editable.setCursor(2);
        REQUIRE(editable.backspace());
        REQUIRE(editable.getText() == "ąść");
    }
}

TEST_CASE("TextEditable - typing with selection", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);
    editable.setText("hello world");

    SECTION("type replaces selection") {
        editable.setSelection(6, 11);
        editable.type("there");
        REQUIRE(editable.getText() == "hello there");
        REQUIRE(editable.getCursor() == 11);
    }

    SECTION("type inserts at cursor without selection") {
        editable.setCursor(5);
        editable.type(",");
        REQUIRE(editable.getText() == "hello, world");
    }
}

TEST_CASE("TextEditable - clipboard", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);

    SECTION("copy with selection puts text on clipboard") {
        editable.setText("copy me");
        editable.setSelection(0, 4);
        REQUIRE(editable.copy());

        char* clip = SDL_GetClipboardText();
        REQUIRE(clip != nullptr);
        REQUIRE(std::string(clip) == "copy");
        SDL_free(clip);
    }

    SECTION("copy without selection is a no-op") {
        REQUIRE_FALSE(editable.copy());
    }

    SECTION("paste inserts clipboard text at cursor") {
        editable.setText("ab");
        editable.setCursor(1);
        SDL_SetClipboardText("XY");
        REQUIRE(editable.paste());
        REQUIRE(editable.getText() == "aXYb");
        REQUIRE(editable.getCursor() == 3);
    }

    SECTION("paste replaces selection") {
        editable.setText("hello world");
        editable.setSelection(6, 11);
        SDL_SetClipboardText("there");
        REQUIRE(editable.paste());
        REQUIRE(editable.getText() == "hello there");
    }

    SECTION("cut removes selection and copies it") {
        editable.setText("hello world");
        editable.setSelection(0, 5);
        REQUIRE(editable.cut());
        REQUIRE(editable.getText() == " world");

        char* clip = SDL_GetClipboardText();
        REQUIRE(clip != nullptr);
        REQUIRE(std::string(clip) == "hello");
        SDL_free(clip);
    }

    SECTION("cut without selection is a no-op") {
        REQUIRE_FALSE(editable.cut());
        REQUIRE(editable.getText().empty());
    }
}

TEST_CASE("TextEditable - cursor blink", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TestEditable editable(manager, 0, 0, 200, 30);

    SECTION("resetCursorBlink shows cursor") {
        editable.resetBlink();
        REQUIRE(editable.isCursorShown());
    }

    SECTION("cursor does not toggle before 500ms") {
        editable.resetBlink();
        REQUIRE(editable.isCursorShown());
        editable.blink();
        REQUIRE(editable.isCursorShown()); // < 500ms elapsed
    }
}

TEST_CASE("TextEditable - charIndexAtX", "[text_editable]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SharedFont font = manager.getFontManager().loadFont("assets/fonts/DejaVuSansMono.ttf", 16);
    REQUIRE(font != nullptr);

    SECTION("x <= 0 returns 0") {
        REQUIRE(TextEditable::charIndexAtX("hello", font.get(), 0) == 0);
        REQUIRE(TextEditable::charIndexAtX("hello", font.get(), -5) == 0);
    }

    SECTION("empty text returns 0") {
        REQUIRE(TextEditable::charIndexAtX("", font.get(), 100) == 0);
    }

    SECTION("large x returns end of text") {
        REQUIRE(TextEditable::charIndexAtX("hello", font.get(), 100000) == 5);
    }

    SECTION("x past some characters selects the right index") {
        size_t idx = TextEditable::charIndexAtX("hello", font.get(), 100);
        REQUIRE(idx > 0);
        REQUIRE(idx <= 5);
    }
}
