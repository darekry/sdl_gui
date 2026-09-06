#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "tab_control.hpp"
#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"

#include "std.hpp"

const int SCREEN_W = 1920;
const int SCREEN_H = 1080;

struct TabData {
    GUIManager* manager = nullptr;            // Pointer to GUIManager (needed to construct widgets)
    Panel* container = nullptr;               // Panel where generated items are added
    std::vector<GUIElement*> items;           // Raw pointers to items (owned by container)
    int nextIndex = 0;                        // Next index used to label items
};

static void create_n_items(TabData* tab, int n) {
    if (!tab || !tab->container) return;

    const int btnW = 100;
    const int btnH = 30;
    const int spacing = 8;
    const int margin = 10;

    int containerW = tab->container->getWidth();

    int cols = std::max(1, (containerW - 2*margin + spacing) / (btnW + spacing));
    // compute starting index
    for (int i = 0; i < n; ++i) {
        int idx = tab->nextIndex++;
        int overall = static_cast<int>(tab->items.size()) + i;
        int col = overall % cols;
        int row = overall / cols;

        int x = margin + col * (btnW + spacing);
        int y = margin + row * (btnH + spacing);

        // Create button with label "Btn N"
        std::string lbl = std::string("Btn ") + std::to_string(idx);
        auto btn = std::make_unique<Button>(*tab->manager, x, y, btnW, btnH, lbl);
        // simple onclick to print its index
        btn->setOnClickCallback([idx](GUIElement*) {
            LOG_INFO("Performance", "[Perf] Button clicked: {}", idx);
        });

        // capture raw pointer before moving
        GUIElement* raw = btn.get();
        tab->container->addChild(std::move(btn));
        tab->items.push_back(raw);
    }
}

static void remove_n_items(TabData* tab, int n) {
    if (!tab || !tab->container) return;
    for (int i = 0; i < n && !tab->items.empty(); ++i) {
        GUIElement* last = tab->items.back();
        if (last) {
            last->markForDeletion();
        }
        tab->items.pop_back();
    }
}

int main(int, char**) {
    try {
        SDLApp app("Performance Example", SCREEN_W, SCREEN_H);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_W, SCREEN_H});

        // Create TabControl filling most of the window
        auto tabControl = std::make_unique<TabControl>(guiManager, 10, 10, SCREEN_W - 20, SCREEN_H - 20);
        Panel* tabA = tabControl->addTab("Buttons A");
        Panel* tabB = tabControl->addTab("Buttons B");
        Panel* tabC = tabControl->addTab("Mixed");

        // We'll create a container inside each tab to host generated items so we keep control buttons separate.
        // Content area starts a bit lower to leave space for control buttons.
        const int ctrl_h = 48;
        auto make_content_panel = [&](Panel* tabPanel) -> Panel* {
            // Create a transparent panel that hosts generated items
            auto content = std::make_unique<Panel>(guiManager, 0, ctrl_h, tabPanel->getWidth(), tabPanel->getHeight() - ctrl_h);
            Panel* contentPtr = content.get();
            tabPanel->addChild(std::move(content));
            return contentPtr;
        };

        Panel* contentA = make_content_panel(tabA);
        Panel* contentB = make_content_panel(tabB);
        Panel* contentC = make_content_panel(tabC);

        // Create TabData for each tab
        TabData dataA; dataA.container = contentA; dataA.manager = &guiManager;
        TabData dataB; dataB.container = contentB; dataB.manager = &guiManager;
        TabData dataC; dataC.container = contentC; dataC.manager = &guiManager;

        // Helper to add control buttons (Create 100 / Remove 100) into tab panel (top area)
        auto add_controls = [&](Panel* tabPanel, TabData* data) {
            // Create "Create 100" button
            auto createBtn = std::make_unique<Button>(guiManager, 10, 10, 200, 36, "Utwórz 100 nowych obiektów");
            createBtn->setOnClickCallback([data](GUIElement*) {
                create_n_items(data, 100);
            });

            // Create "Remove 100" button
            auto removeBtn = std::make_unique<Button>(guiManager, 220, 10, 200, 36, "Usuń 100 ostatnich obiektów");
            removeBtn->setOnClickCallback([data](GUIElement*) {
                remove_n_items(data, 100);
            });

            // Add to the tabPanel (these are positioned at top of tab area)
            tabPanel->addChild(std::move(createBtn));
            tabPanel->addChild(std::move(removeBtn));
        };

        add_controls(tabA, &dataA);
        add_controls(tabB, &dataB);
        add_controls(tabC, &dataC);

        // Add the TabControl as a top-level element
        guiManager.addElement(std::move(tabControl));

        // FPS / frame time label as top-level element (overlay)
        auto fpsLabel = std::make_unique<Label>(guiManager, SCREEN_W - 300, 10, "FPS: 0 | Frame: 0.00 ms", 20);
        Label* fpsLabelPtr = fpsLabel.get();
        guiManager.addElement(std::move(fpsLabel));

        // Timing variables
        using clock = std::chrono::high_resolution_clock;
        auto last = clock::now();
        double fps = 0.0;
        double frameTimeMs = 0.0;
        int frames = 0;
        double accumTime = 0.0;

        bool quit = false;
        SDL_Event e;

        // Main loop
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            // Update timers and remove marked elements
            guiManager.cleanup();

            // Timing
            auto now = clock::now();
            std::chrono::duration<double> diff = now - last;
            last = now;
            frameTimeMs = diff.count() * 1000.0;
            accumTime += diff.count();
            frames++;
            if (accumTime >= 10) { // update FPS ~4 times per second for stability
                fps = frames / accumTime;
                frames = 0;
                accumTime = 0.0;

                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2);
                ss << "FPS: " << fps << " | Frame: " << frameTimeMs << " ms | Total elements: ";
                // simple estimate: sum items in our three tabs
                size_t total = dataA.items.size() + dataB.items.size() + dataC.items.size();
                ss << total;
                fpsLabelPtr->setText(ss.str());
            }

            // Render
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }

    } catch (const std::runtime_error& ex) {
        std::cerr << "Runtime error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}