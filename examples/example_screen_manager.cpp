/**
 * @file example_screen_manager.cpp
 * @brief Demonstracja ScreenManager - przełączanie ekranów w jednym oknie
 * 
 * Ten przykład pokazuje:
 * - MenuScreen - ekran główny z wyborem trybów gry
 * - GameScreen - ekran "rozgrywki" z kontrolkami
 * - PauseScreen - overlay ekranu pauzy (pushScreen/popScreen)
 * 
 * Architektura:
 * 1. ScreenManager zarządza ekranami w jednym oknie
 * 2. Każdy ekran (Screen) ma onEnter/onExit lifecycle
 * 3. PushScreen tworzy overlay (np. pauza nad grą)
 * 4. PopScreen wraca do poprzedniego ekranu
 */

#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "screen_manager.hpp"
#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "checkbox.hpp"

import std.compat;

class MenuScreen : public Screen {
public:
    std::string getName() const override { return "MenuScreen"; }
    
    void onEnter(GUIManager& manager) override {
        auto panel = std::make_unique<Panel>(manager, 200, 100, 400, 400);
        panel->setBackgroundColor(ElementState::Normal, {240, 240, 250, 255});
        panel->setBorder(ElementState::Normal, {100, 100, 150, 255}, 2);
        panel->setBorderRadius(ElementState::Normal, 10);
        m_panel = panel.get();
        
        auto titleLabel = std::make_unique<Label>(manager, 100, 30, "MAIN MENU", 36);
        panel->addChild(std::move(titleLabel));
        
        auto btnStart = std::make_unique<Button>(manager, 100, 100, 200, 50, "Start Game");
        btnStart->setOnClickCallback([this](GUIElement*) {
            if (m_onStartGame) m_onStartGame();
        });
        m_btnStart = btnStart.get();
        panel->addChild(std::move(btnStart));
        
        auto btnSettings = std::make_unique<Button>(manager, 100, 160, 200, 50, "Settings");
        btnSettings->setOnClickCallback([](GUIElement*) {
            std::cout << "Settings screen not implemented in this demo\n";
        });
        panel->addChild(std::move(btnSettings));
        
        auto btnQuit = std::make_unique<Button>(manager, 100, 220, 200, 50, "Quit Game");
        btnQuit->setOnClickCallback([this](GUIElement*) {
            if (m_onQuit) m_onQuit();
        });
        panel->addChild(std::move(btnQuit));
        
        auto infoLabel = std::make_unique<Label>(manager, 50, 300, 
            "Click 'Start Game' to switch to game screen. ESC returns to menu.", 16);
        panel->addChild(std::move(infoLabel));
        
        manager.addElement(std::move(panel));
    }
    
    void onExit(GUIManager&) override {
        if (m_panel) {
            m_panel->markForDeletion();
        }
        m_panel = nullptr;
        m_btnStart = nullptr;
    }
    
    bool handleEvent(GUIManager&, const SDL_Event&) override {
        return false;
    }
    
    void update(GUIManager&) override {}
    
    void render(GUIManager&, SDL_Renderer*) override {}
    
    void setOnStartGame(std::function<void()> callback) { m_onStartGame = callback; }
    void setOnQuit(std::function<void()> callback) { m_onQuit = callback; }
    
private:
    Panel* m_panel = nullptr;
    Button* m_btnStart = nullptr;
    std::function<void()> m_onStartGame;
    std::function<void()> m_onQuit;
};

class GameScreen : public Screen {
public:
    std::string getName() const override { return "GameScreen"; }
    
    void onEnter(GUIManager& manager) override {
        auto panel = std::make_unique<Panel>(manager, 50, 50, 700, 500);
        panel->setBackgroundColor(ElementState::Normal, {20, 30, 40, 255});
        panel->setBorder(ElementState::Normal, {50, 70, 90, 255}, 2);
        m_panel = panel.get();
        
        auto titleLabel = std::make_unique<Label>(manager, 250, 20, "GAME SCREEN", 32);
        panel->addChild(std::move(titleLabel));
        
        auto volumeLabel = std::make_unique<Label>(manager, 50, 80, "Volume:", 20);
        panel->addChild(std::move(volumeLabel));
        
        auto volumeSlider = std::make_unique<Slider>(manager, 150, 85, 200, 20, 0, 100, 70, Orientation::Horizontal);
        m_volumeSlider = volumeSlider.get();
        volumeSlider->setOnChangeCallback([](GUIElement* el) {
            Slider* s = static_cast<Slider*>(el);
            std::cout << "Volume: " << s->getValue() << "%\n";
        });
        panel->addChild(std::move(volumeSlider));
        
        auto diffCheckbox = std::make_unique<Checkbox>(manager, 50, 130, 20, 20);
        m_diffCheckbox = diffCheckbox.get();
        diffCheckbox->setChecked(false);
        diffCheckbox->setOnChange([](Checkbox* cb, bool checked) {
            std::cout << "Hard Mode: " << (checked ? "ON" : "OFF") << "\n";
        });
        panel->addChild(std::move(diffCheckbox));
        
        auto diffLabel = std::make_unique<Label>(manager, 80, 130, "Hard Mode", 20);
        panel->addChild(std::move(diffLabel));
        
        auto btnPause = std::make_unique<Button>(manager, 50, 200, 150, 40, "Pause (ESC)");
        btnPause->setOnClickCallback([this](GUIElement*) {
            if (m_onPause) m_onPause();
        });
        panel->addChild(std::move(btnPause));
        
        auto btnReturn = std::make_unique<Button>(manager, 220, 200, 150, 40, "Back to Menu");
        btnReturn->setOnClickCallback([this](GUIElement*) {
            if (m_onReturn) m_onReturn();
        });
        panel->addChild(std::move(btnReturn));
        
        auto gameAreaLabel = std::make_unique<Label>(manager, 100, 280, 
            "This would be the actual game area.", 16);
        panel->addChild(std::move(gameAreaLabel));
        
        manager.addElement(std::move(panel));
    }
    
    void onExit(GUIManager&) override {
        if (m_panel) {
            m_panel->markForDeletion();
        }
        m_panel = nullptr;
        m_volumeSlider = nullptr;
        m_diffCheckbox = nullptr;
    }
    
    bool handleEvent(GUIManager&, const SDL_Event& e) override {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (m_onPause) m_onPause();
            return true;
        }
        return false;
    }
    
    void update(GUIManager&) override {}
    
    void render(GUIManager&, SDL_Renderer*) override {}
    
    bool wantsPreProcessEvent() const override { return true; }
    
    void setOnPause(std::function<void()> callback) { m_onPause = callback; }
    void setOnReturn(std::function<void()> callback) { m_onReturn = callback; }
    
private:
    Panel* m_panel = nullptr;
    Slider* m_volumeSlider = nullptr;
    Checkbox* m_diffCheckbox = nullptr;
    std::function<void()> m_onPause;
    std::function<void()> m_onReturn;
};

class PauseScreen : public Screen {
public:
    std::string getName() const override { return "PauseScreen"; }
    
    void onEnter(GUIManager& manager) override {
        auto panel = std::make_unique<Panel>(manager, 250, 150, 300, 300);
        panel->setBackgroundColor(ElementState::Normal, {50, 50, 50, 230});
        panel->setBorder(ElementState::Normal, {255, 255, 255, 255}, 2);
        panel->setBorderRadius(ElementState::Normal, 8);
        m_panel = panel.get();
        
        auto titleLabel = std::make_unique<Label>(manager, 100, 30, "PAUSED", 28);
        panel->addChild(std::move(titleLabel));
        
        auto btnResume = std::make_unique<Button>(manager, 50, 100, 200, 50, "Resume");
        btnResume->setOnClickCallback([this](GUIElement*) {
            if (m_onResume) m_onResume();
        });
        panel->addChild(std::move(btnResume));
        
        auto btnMenu = std::make_unique<Button>(manager, 50, 160, 200, 50, "Return to Menu");
        btnMenu->setOnClickCallback([this](GUIElement*) {
            if (m_onMenu) m_onMenu();
        });
        panel->addChild(std::move(btnMenu));
        
        manager.addElement(std::move(panel));
    }
    
    void onExit(GUIManager&) override {
        if (m_panel) {
            m_panel->markForDeletion();
        }
        m_panel = nullptr;
    }
    
    bool handleEvent(GUIManager&, const SDL_Event& e) override {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (m_onResume) m_onResume();
            return true;
        }
        return false;
    }
    
    void update(GUIManager&) override {}
    void render(GUIManager&, SDL_Renderer*) override {}
    
    bool wantsPreProcessEvent() const override { return true; }
    
    void setOnResume(std::function<void()> callback) { m_onResume = callback; }
    void setOnMenu(std::function<void()> callback) { m_onMenu = callback; }
    
private:
    Panel* m_panel = nullptr;
    std::function<void()> m_onResume;
    std::function<void()> m_onMenu;
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Screen Manager Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        ScreenManager screenManager(guiManager);
        
        auto menuScreen = std::make_unique<MenuScreen>();
        auto gameScreen = std::make_unique<GameScreen>();
        auto pauseScreen = std::make_unique<PauseScreen>();
        
        MenuScreen* menuPtr = menuScreen.get();
        GameScreen* gamePtr = gameScreen.get();
        PauseScreen* pausePtr = pauseScreen.get();
        
        menuPtr->setOnStartGame([&]() {
            screenManager.changeScreen("game");
        });
        menuPtr->setOnQuit([&]() {
            std::cout << "Quit requested\n";
        });
        
        gamePtr->setOnPause([&]() {
            screenManager.pushScreen("pause");
        });
        gamePtr->setOnReturn([&]() {
            screenManager.changeScreen("menu");
        });
        
        pausePtr->setOnResume([&]() {
            screenManager.popScreen();
        });
        pausePtr->setOnMenu([&]() {
            screenManager.popScreen();
            screenManager.changeScreen("menu");
        });
        
        screenManager.addScreen("menu", std::move(menuScreen));
        screenManager.addScreen("game", std::move(gameScreen));
        screenManager.addScreen("pause", std::move(pauseScreen));
        
        screenManager.changeScreen("menu");
        
        bool quit = false;
        SDL_Event e;
        
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else {
                    screenManager.handleEvent(e);
                }
            }
            
            SDL_SetRenderDrawColor(renderer, 200, 200, 210, 255);
            SDL_RenderClear(renderer);
            
            screenManager.update();
            screenManager.render(renderer);
            screenManager.cleanup();
            
            SDL_RenderPresent(renderer);
        }
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}