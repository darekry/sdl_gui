#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/utf8_utils.hpp"
#include "../src/text_input.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("UTF8 helpers", "[utf8]") {
    SECTION("charCount vs byte size") {
        std::string polish = "zażółć";
        REQUIRE(utf8::charCount(polish) == 6);
        REQUIRE(polish.size() == 10);
    }
    
    SECTION("charToByteIndex") {
        std::string text = "zażółć";
        REQUIRE(utf8::charToByteIndex(text, 0) == 0);
        REQUIRE(utf8::charToByteIndex(text, 1) == 1);
        REQUIRE(utf8::charToByteIndex(text, 2) == 2);
        REQUIRE(utf8::charToByteIndex(text, 3) == 4);
        REQUIRE(utf8::charToByteIndex(text, 4) == 6);
        REQUIRE(utf8::charToByteIndex(text, 5) == 8);
        REQUIRE(utf8::charToByteIndex(text, 6) == 10);
    }
    
    SECTION("substrChars Polish") {
        std::string text = "zażółć";
        REQUIRE(utf8::substrChars(text, 0, 2) == "za");
        REQUIRE(utf8::substrChars(text, 2, 2) == "żó");
        REQUIRE(utf8::substrChars(text, 0, 6) == "zażółć");
    }
    
    SECTION("charByteLength") {
        REQUIRE(utf8::charByteLength('z') == 1);
        REQUIRE(utf8::charByteLength(0xC5) == 2);
    }
    
    SECTION("prevCharBytePos Polish") {
        std::string text = "zażółć";
        REQUIRE(utf8::prevCharBytePos(text, 1) == 0);
        REQUIRE(utf8::prevCharBytePos(text, 4) == 2);
        REQUIRE(utf8::prevCharBytePos(text, 6) == 4);
        REQUIRE(utf8::prevCharBytePos(text, 10) == 8);
    }
    
    SECTION("nextCharBytePos Polish") {
        std::string text = "zażółć";
        REQUIRE(utf8::nextCharBytePos(text, 0) == 1);
        REQUIRE(utf8::nextCharBytePos(text, 2) == 4);
        REQUIRE(utf8::nextCharBytePos(text, 4) == 6);
        REQUIRE(utf8::nextCharBytePos(text, 8) == 10);
    }
    
    SECTION("byteIndexToCharIndex Polish") {
        std::string text = "zażółć";
        REQUIRE(utf8::byteIndexToCharIndex(text, 0) == 0);
        REQUIRE(utf8::byteIndexToCharIndex(text, 1) == 1);
        REQUIRE(utf8::byteIndexToCharIndex(text, 2) == 2);
        REQUIRE(utf8::byteIndexToCharIndex(text, 4) == 3);
        REQUIRE(utf8::byteIndexToCharIndex(text, 6) == 4);
        REQUIRE(utf8::byteIndexToCharIndex(text, 8) == 5);
        REQUIRE(utf8::byteIndexToCharIndex(text, 10) == 6);
    }
}

TEST_CASE("UTF8 edge cases", "[utf8]") {
    SECTION("Empty string") {
        REQUIRE(utf8::charCount("") == 0);
        REQUIRE(utf8::charToByteIndex("", 0) == 0);
        REQUIRE(utf8::substrChars("", 0, 0) == "");
    }
    
    SECTION("ASCII only") {
        std::string ascii = "hello";
        REQUIRE(utf8::charCount(ascii) == 5);
        REQUIRE(ascii.size() == 5);
        REQUIRE(utf8::substrChars(ascii, 0, 3) == "hel");
    }
    
    SECTION("Mixed ASCII and Polish") {
        std::string mixed = "aśćb";
        REQUIRE(utf8::charCount(mixed) == 4);
        REQUIRE(mixed.size() == 6);
        REQUIRE(utf8::substrChars(mixed, 0, 2) == "aś");
        REQUIRE(utf8::substrChars(mixed, 2, 2) == "ćb");
    }
    
    SECTION("All Polish special chars") {
        std::string all = "ąćęłńóśźż";
        REQUIRE(utf8::charCount(all) == 9);
        REQUIRE(all.size() == 18);
    }
}

TEST_CASE("Label with Polish text", "[utf8][label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("Polish text display") {
        auto label = std::make_unique<Label>(manager, 10, 10, "Żółw", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));
        
        REQUIRE(labelPtr->getWidth() > 0);
        REQUIRE(labelPtr->getHeight() > 0);
        
        labelPtr->setText("zażółć gęśl jaźń");
        REQUIRE(labelPtr->getWidth() > 0);
        REQUIRE(labelPtr->getHeight() > 0);
        
        labelPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("Long Polish text") {
        auto label = std::make_unique<Label>(manager, 10, 10, "ĄąĆćĘęŁłŃńÓóŚśŹźŻż", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));
        
        REQUIRE(utf8::charCount("ĄąĆćĘęŁłŃńÓóŚśŹźŻż") == 18);
        REQUIRE(labelPtr->getWidth() > 0);
        
        labelPtr->markForDeletion();
        manager.cleanup();
    }
}

TEST_CASE("TextInput with Polish text - cursor movement", "[utf8][textinput]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("TextInput Polish text input") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        REQUIRE(inputPtr->getText() == "zażółć");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 6);
        REQUIRE(inputPtr->getText().size() == 10);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("TextInput backspace removes full Polish char") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        REQUIRE(inputPtr->getText() == "zażółć");
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        
        REQUIRE(inputPtr->getText() == "zażół");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 5);
        REQUIRE(inputPtr->getText().size() == 8);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("TextInput multiple backspace on Polish text") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(inputPtr->getText() == "zażół");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 5);
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(inputPtr->getText() == "zażó");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 4);
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(inputPtr->getText() == "zaż");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 3);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("TextInput delete removes full Polish char") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DELETE));
        
        REQUIRE(inputPtr->getText() == "zażół");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 5);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("TextInput arrow left moves by char not byte") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zaż"));
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createTextInputEvent("X"));
        
        REQUIRE(inputPtr->getText() == "zaXż");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 4);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("TextInput arrow keys with Polish chars") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("ąę"));
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createTextInputEvent("X"));
        
        REQUIRE(inputPtr->getText() == "Xąę");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 3);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
}

TEST_CASE("UTF8 cursor positioning", "[utf8][cursor]") {
    SECTION("Cursor at char boundaries") {
        std::string text = "zażółć";
        
        size_t cursorByte3 = utf8::charToByteIndex(text, 3);
        REQUIRE(cursorByte3 == 4);
        REQUIRE(utf8::byteIndexToCharIndex(text, cursorByte3) == 3);
        
        size_t cursorByte5 = utf8::charToByteIndex(text, 5);
        REQUIRE(cursorByte5 == 8);
        REQUIRE(utf8::byteIndexToCharIndex(text, cursorByte5) == 5);
    }
    
    SECTION("Cursor navigation consistency") {
        std::string text = "ąćęłń";
        
        for (size_t i = 0; i <= utf8::charCount(text); i++) {
            size_t bytePos = utf8::charToByteIndex(text, i);
            size_t charPos = utf8::byteIndexToCharIndex(text, bytePos);
            REQUIRE(charPos == i);
        }
    }
}

TEST_CASE("TextInput selection with Polish text", "[utf8][selection]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("Selection on Polish text") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        inputPtr->setSelection(2, 4);
        REQUIRE(inputPtr->hasSelection());
        REQUIRE(inputPtr->getSelection() == "żó");
        REQUIRE(utf8::charCount(inputPtr->getSelection()) == 2);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("Backspace with selection removes correct chars") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("zażółć"));
        
        inputPtr->setSelection(2, 4);
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        
        REQUIRE(inputPtr->getText() == "załć");
        REQUIRE(utf8::charCount(inputPtr->getText()) == 4);
        REQUIRE_FALSE(inputPtr->hasSelection());
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
    
    SECTION("Shift+Arrow creates selection on Polish text") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* inputPtr = input.get();
        manager.addElement(std::move(input));
        
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("ąćęł"));
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT, KMOD_SHIFT));
        REQUIRE(inputPtr->hasSelection());
        REQUIRE(utf8::charCount(inputPtr->getSelection()) == 1);
        
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT, KMOD_SHIFT));
        REQUIRE(utf8::charCount(inputPtr->getSelection()) == 2);
        
        inputPtr->markForDeletion();
        manager.cleanup();
    }
}