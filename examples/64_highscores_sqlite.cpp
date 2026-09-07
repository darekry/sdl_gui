/**
 * @file 64_highscores_sqlite.cpp
 * @brief Third-party: SQLite highscore table behind a tiny GUI (3rd-party example).
 *
 * Uses the system sqlite3 (dev headers already installed, -lsqlite3 added
 * only for *sqlite* examples - see the hook in nob.c). Name comes from a
 * TextInput, score from a Slider, top-5 renders into a multiline Label.
 * The database file `highscores.db` lands in the working directory.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "label.hpp"
#include "button.hpp"
#include "text_input.hpp"

#include "std.hpp"
#include <sqlite3.h>

int main(int, char**) {
    sqlite3* db = nullptr;
    try {
        SDLApp app("Highscores - SQLite leaderboard", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        if (sqlite3_open("highscores.db", &db) != SQLITE_OK)
            throw std::runtime_error("cannot open highscores.db");
        const char* ddl = "CREATE TABLE IF NOT EXISTS scores("
                          "id INTEGER PRIMARY KEY, name TEXT NOT NULL, score INTEGER NOT NULL);";
        if (sqlite3_exec(db, ddl, nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("cannot create table");

        auto info = std::make_unique<Label>(guiManager, 30, 20,
            "Type a name, pick a score, Save - top 5 persists in highscores.db", 16);
        guiManager.addElement(std::move(info));

        auto nameLbl = std::make_unique<Label>(guiManager, 30, 70, "Name:", 17);
        guiManager.addElement(std::move(nameLbl));
        auto nameInput = std::make_unique<TextInput>(guiManager, 110, 65, 220, 36);
        auto nameRef = guiManager.makeRef(nameInput.get());
        guiManager.addElement(std::move(nameInput));

        auto scoreLbl = std::make_unique<Label>(guiManager, 30, 130, "Score: 500", 17);
        auto scoreRef = guiManager.makeRef(scoreLbl.get());
        guiManager.addElement(std::move(scoreLbl));

        int picked = 500;
        auto scoreSlider = std::make_unique<Slider>(guiManager, 30, 165, 420, 34,
                                                    0, 1000, 500, Orientation::Horizontal);
        scoreSlider->setOnChangeCallback([&picked, scoreRef](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            picked = s->getValue();
            if (scoreRef) scoreRef->setText("Score: " + std::to_string(picked));
        });
        guiManager.addElement(std::move(scoreSlider));

        auto board = std::make_unique<Label>(guiManager, 30, 250, "", 19);
        auto boardRef = guiManager.makeRef(board.get());
        guiManager.addElement(std::move(board));

        auto status = std::make_unique<Label>(guiManager, 30, 500, "", 16);
        auto statusRef = guiManager.makeRef(status.get());
        guiManager.addElement(std::move(status));

        std::function<void()> refresh = [&]() {
            sqlite3_stmt* st = nullptr;
            const char* q = "SELECT name, score FROM scores ORDER BY score DESC LIMIT 5;";
            std::string text;
            if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) == SQLITE_OK) {
                int rank = 1;
                while (sqlite3_step(st) == SQLITE_ROW) {
                    const char* name = reinterpret_cast<const char*>(sqlite3_column_text(st, 0));
                    int score = sqlite3_column_int(st, 1);
                    text += std::to_string(rank++) + ". " + (name ? name : "?") +
                            " - " + std::to_string(score) + "\n";
                }
            }
            sqlite3_finalize(st);
            if (text.empty()) text = "(no scores yet)";
            if (boardRef) boardRef->setText("TOP 5\n" + text);
        };

        auto saveBtn = std::make_unique<Button>(guiManager, 470, 120, 150, 44, "Save");
        saveBtn->setOnClickCallback([&](GUIElement*) {
            std::string name = (nameRef && !nameRef->getText().empty()) ? nameRef->getText() : "Anon";
            sqlite3_stmt* st = nullptr;
            const char* ins = "INSERT INTO scores(name, score) VALUES(?, ?);";
            bool ok = false;
            if (sqlite3_prepare_v2(db, ins, -1, &st, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(st, 1, name.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(st, 2, picked);
                ok = sqlite3_step(st) == SQLITE_DONE;
            }
            sqlite3_finalize(st);
            if (statusRef) statusRef->setText(ok ? ("saved " + name) : "save FAILED");
            refresh();
        });
        guiManager.addElement(std::move(saveBtn));

        auto clearBtn = std::make_unique<Button>(guiManager, 470, 180, 150, 44, "Clear all");
        clearBtn->setOnClickCallback([&](GUIElement*) {
            sqlite3_exec(db, "DELETE FROM scores;", nullptr, nullptr, nullptr);
            if (statusRef) statusRef->setText("table cleared");
            refresh();
        });
        guiManager.addElement(std::move(clearBtn));
        refresh();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (db) sqlite3_close(db);
        return 1;
    }
    if (db) sqlite3_close(db);
    return 0;
}
