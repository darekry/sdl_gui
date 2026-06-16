/**
 * @file example_window_manager.cpp
 * @brief Demonstracja WindowManager - wiele okien systemowych
 * 
 * Ten przykład pokazuje:
 * - MainWindow - główne okno aplikacji
 * - SettingsWindow - okno ustawień (osobne okno systemowe)
 * - FormWindow - okno z formularzem
 * 
 * Architektura:
 * 1. WindowManager zarządza wieloma oknami SDL
 * 2. Każde Window ma własny SDL_Window, SDL_Renderer i GUIManager
 * 3. Zdarzenia są automatycznie routowane do odpowiednego okna (SDL_WINDOWID)
 * 4. Okna mogą być przesuwane/minimalizowane przez użytkownika
 */

#include "window_manager.hpp"
#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "text_area.hpp"
#include "logger.hpp"

import std.compat;

void setupSettingsWindow(Window* window);

void setupFormWindow(Window* window);

void setupMainWindow(Window* window, WindowManager& manager) {
    GUIManager& gui = window->getGUIManager();
    
    auto panel = std::make_unique<Panel>(gui, 50, 50, 700, 500);
    panel->setBackgroundColor(ElementState::Normal, {250, 250, 255, 255});
    panel->setBorder(ElementState::Normal, {100, 100, 150, 255}, 2);
    panel->setBorderRadius(ElementState::Normal, 10);
    
    auto titleLabel = std::make_unique<Label>(gui, 200, 20, "MAIN APPLICATION WINDOW", 28);
    panel->addChild(std::move(titleLabel));
    
    auto btnSettings = std::make_unique<Button>(gui, 50, 100, 200, 50, "Open Settings Window");
    btnSettings->setOnClickCallback([&manager](GUIElement*) {
        if (manager.getWindowCount() < 2) {
            Window* settingsWindow = manager.createWindow("Settings", 450, 400);
            if (settingsWindow) {
                setupSettingsWindow(settingsWindow);
                settingsWindow->setOnCloseCallback([](Window* w) {
                    w->markForClose();
                });
                LOG_INFO("WindowManager", "Settings window created (ID={})", settingsWindow->getWindowID());
            }
        } else {
            LOG_INFO("WindowManager", "Settings window already open");
        }
    });
    panel->addChild(std::move(btnSettings));
    
    auto btnForm = std::make_unique<Button>(gui, 260, 100, 200, 50, "Open Form Window");
    btnForm->setOnClickCallback([&manager](GUIElement*) {
        bool formExists = false;
        for (size_t i = 0; i < manager.getWindowCount(); ++i) {
            Window* w = manager.getWindow(i);
            if (w && w->getTitle() == "Contact Form") {
                formExists = true;
                w->show();
                break;
            }
        }
        
        if (!formExists) {
            Window* formWindow = manager.createWindow("Contact Form", 400, 500);
            if (formWindow) {
                setupFormWindow(formWindow);
                formWindow->setOnCloseCallback([](Window* w) {
                    LOG_INFO("WindowManager", "Form window closed");
                    w->markForClose();
                });
                LOG_INFO("WindowManager", "Form window created (ID={})", formWindow->getWindowID());
            }
        }
    });
    panel->addChild(std::move(btnForm));
    
    auto btnCloseAll = std::make_unique<Button>(gui, 50, 160, 200, 50, "Close All Secondary");
    btnCloseAll->setOnClickCallback([&manager](GUIElement*) {
        manager.closeSecondaryWindows();
        LOG_INFO("WindowManager", "All secondary windows closed");
    });
    panel->addChild(std::move(btnCloseAll));
    
    auto info1 = std::make_unique<Label>(gui, 50, 250, 
        "This demonstrates multiple SDL windows.", 18);
    panel->addChild(std::move(info1));
    
    auto info2 = std::make_unique<Label>(gui, 50, 330,
        "Windows are independent - move, minimize, close each one.", 16);
    panel->addChild(std::move(info2));
    
    auto info3 = std::make_unique<Label>(gui, 50, 420,
        "Press ESC in secondary windows to close them.", 16);
    panel->addChild(std::move(info3));
    
    gui.addElement(std::move(panel));
    
    window->setOnCloseCallback([&manager](Window*) {
        LOG_INFO("WindowManager", "Main window closed - exiting application");
        manager.requestQuit();
    });
}

void setupSettingsWindow(Window* window) {
    GUIManager& gui = window->getGUIManager();
    gui.setWindowSize(450, 400);
    
    auto panel = std::make_unique<Panel>(gui, 20, 20, 410, 360);
    panel->setBackgroundColor(ElementState::Normal, {240, 245, 250, 255});
    panel->setBorder(ElementState::Normal, {80, 80, 100, 255}, 1);
    
    auto titleLabel = std::make_unique<Label>(gui, 130, 20, "SETTINGS", 24);
    panel->addChild(std::move(titleLabel));
    
    auto volumeLabel = std::make_unique<Label>(gui, 20, 70, "Volume:", 18);
    panel->addChild(std::move(volumeLabel));
    
    auto volumeSlider = std::make_unique<Slider>(gui, 100, 75, 150, 20, 0, 100, 75, Orientation::Horizontal);
    volumeSlider->setOnChangeCallback([](GUIElement* el) {
        Slider* s = static_cast<Slider*>(el);
        LOG_INFO("WindowManager", "Volume changed to {}%", s->getValue());
    });
    panel->addChild(std::move(volumeSlider));
    
    auto volumeValueLabel = std::make_unique<Label>(gui, 260, 70, "75%", 18);
    panel->addChild(std::move(volumeValueLabel));
    
    auto musicCheckbox = std::make_unique<Checkbox>(gui, 20, 120, 20, 20);
    musicCheckbox->setChecked(true);
    musicCheckbox->setOnChange([](Checkbox*, bool checked) {
        LOG_INFO("WindowManager", "Music: {}", checked ? "ON" : "OFF");
    });
    panel->addChild(std::move(musicCheckbox));
    
    auto musicLabel = std::make_unique<Label>(gui, 50, 120, "Enable Music", 18);
    panel->addChild(std::move(musicLabel));
    
    auto sfxCheckbox = std::make_unique<Checkbox>(gui, 20, 160, 20, 20);
    sfxCheckbox->setChecked(true);
    sfxCheckbox->setOnChange([](Checkbox*, bool checked) {
        LOG_INFO("WindowManager", "Sound Effects: {}", checked ? "ON" : "OFF");
    });
    panel->addChild(std::move(sfxCheckbox));
    
    auto sfxLabel = std::make_unique<Label>(gui, 50, 160, "Enable Sound Effects", 18);
    panel->addChild(std::move(sfxLabel));
    
    auto diffLabel = std::make_unique<Label>(gui, 20, 210, "Difficulty: Easy | Medium | Hard", 18);
    panel->addChild(std::move(diffLabel));
    
    auto btnClose = std::make_unique<Button>(gui, 150, 280, 110, 40, "Close");
    btnClose->setOnClickCallback([window](GUIElement*) {
        if (window) window->markForClose();
    });
    panel->addChild(std::move(btnClose));
    
    gui.addElement(std::move(panel));
}

void setupFormWindow(Window* window) {
    GUIManager& gui = window->getGUIManager();
    gui.setWindowSize(400, 500);
    
    auto panel = std::make_unique<Panel>(gui, 20, 20, 360, 460);
    panel->setBackgroundColor(ElementState::Normal, {255, 255, 255, 255});
    panel->setBorder(ElementState::Normal, {100, 100, 100, 255}, 1);
    
    auto titleLabel = std::make_unique<Label>(gui, 100, 20, "Contact Form", 24);
    panel->addChild(std::move(titleLabel));
    
    auto nameLabel = std::make_unique<Label>(gui, 20, 70, "Name:", 18);
    panel->addChild(std::move(nameLabel));
    
    auto nameInput = std::make_unique<TextInput>(gui, 100, 70, 240, 30);
    panel->addChild(std::move(nameInput));
    
    auto emailLabel = std::make_unique<Label>(gui, 20, 110, "Email:", 18);
    panel->addChild(std::move(emailLabel));
    
    auto emailInput = std::make_unique<TextInput>(gui, 100, 110, 240, 30);
    panel->addChild(std::move(emailInput));
    
    auto msgLabel = std::make_unique<Label>(gui, 20, 160, "Message:", 18);
    panel->addChild(std::move(msgLabel));
    
    auto msgArea = std::make_unique<TextArea>(gui, 20, 190, 320, 150, "assets/fonts/font.ttf", 16);
    panel->addChild(std::move(msgArea));
    
    auto btnSubmit = std::make_unique<Button>(gui, 120, 360, 120, 40, "Submit");
    btnSubmit->setOnClickCallback([window](GUIElement*) {
        LOG_INFO("WindowManager", "Form submitted!");
        if (window) window->markForClose();
    });
    panel->addChild(std::move(btnSubmit));
    
    auto btnCancel = std::make_unique<Button>(gui, 20, 360, 90, 40, "Cancel");
    btnCancel->setOnClickCallback([window](GUIElement*) {
        if (window) window->markForClose();
    });
    panel->addChild(std::move(btnCancel));
    
    gui.addElement(std::move(panel));
}

int main(int, char**) {
    try {
        WindowManager windowManager;
        
        Window* mainWindow = windowManager.createWindow("Main Application", 800, 600, true);
        if (!mainWindow) {
            std::cerr << "Failed to create main window\n";
            return 1;
        }
        
        setupMainWindow(mainWindow, windowManager);
        
        LOG_INFO("WindowManager", "Main window created (ID={})", mainWindow->getWindowID());
        LOG_INFO("WindowManager", "Click buttons to open secondary windows.");
        
        while (!windowManager.shouldQuit()) {
            windowManager.processEvents();
            windowManager.updateAll();
            windowManager.renderAll();
            windowManager.cleanupAll();
            
            SDL_Delay(16);
        }
        
        LOG_INFO("WindowManager", "Application exiting");
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}