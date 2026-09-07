// bench_gui — sintetyczny benchmark GUI na PRAWDZIWYM oknie.
//
// Cel: baseline wydajności sekcji event/render/layout przed refaktorem
// punktu 1 (Dispatcher/Focus/Cursor) i porównywanie po zmianach.
//
// Uruchomienie:
//   ./output/bench_gui [--widgets N] [--sweep] [--seconds S]
//                       [--backend sdl|vulkan] [--hidden]
//                       [--save baseline.json] [--compare baseline.json]
//
//   --widgets N   liczba widgetów sceny (default 200)
//   --sweep       zamiast jednego N jedź po 100,200,1000
//   --seconds S   budżet pętli frame_loop + limit sekcji hover (default 5)
//   --backend     sdl (default) lub vulkan (SDL_CreateGPURenderer)
//   --hidden      ukryj okno (SDL_GUI_HIDDEN=1, np. pod CI)
//   --save F      zapisz wyniki jako JSON (baseline)
//   --compare F   porównaj z baseline i wypisz delty %
//
// Scena: deterministyczna siatka mieszanych widgetów (Button/Label/Slider/
// Checkbox/TextInput/TextArea/Panel z dzieckiem), część zakotwiczona
// (Anchor) żeby sekcja resize miała co przeliczać. Seed RNG = 42.
//
// Sekcje (każda raportuje ops/s + ns/op; frame_loop dodatkowo fps i p95):
//   hover          MOTION po zygzaku przez ekran (tylko processEvent)
//   click          pary DOWN/UP na środkach przycisków
//   slider_drag    DOWN + 20×MOTION + UP na każdym sliderze
//   label_text     setText na labelkach (20× każda)
//   typing         focus TextInput + TEXT_INPUT (+BACKSPACE co 5.)
//   create_destroy 5 rund: dobuduj N/10 widgetów, markForDeletion, cleanup
//   resize         60× handleResize na przemian 1280×800/800×600/1920×1080
//                  (+ prawdziwe SDL_SetWindowSize — stąd prawdziwe okno)
//   frame_loop     realistyczna pętla: eventy + update + cleanup + render +
//                  present przez --seconds sekund (to jedyna sekcja z present)

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "constants.hpp"
#include "anchor.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "text_area.hpp"
#include "panel.hpp"
#include "std.hpp"

#include <cstdio>

namespace {

constexpr int kWinW = 1280;
constexpr int kWinH = 800;

struct SceneRefs {
    std::vector<Button*> buttons;
    std::vector<Label*> labels;
    std::vector<Slider*> sliders;
    std::vector<TextInput*> inputs;
    std::vector<TextArea*> areas;
    std::vector<GUIElement*> all;  // środki do hover/click
};

struct SectionResult {
    std::string section;
    int widgets = 0;
    long long ops = 0;
    double sec = 0.0;
    double perSec = 0.0;
    double nsPerOp = 0.0;
    std::string extra;  // np. "fps=59.1 p95=17.4ms" dla frame_loop
};

using Clock = std::chrono::steady_clock;

SDL_Event makeMotion(int x, int y) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.type = SDL_EVENT_MOUSE_MOTION;
    e.motion.x = static_cast<float>(x);
    e.motion.y = static_cast<float>(y);
    return e;
}

SDL_Event makeButton(SDL_EventType type, Uint8 button, int x, int y) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = type;
    e.button.type = type;
    e.button.button = button;
    e.button.clicks = 1;
    e.button.x = static_cast<float>(x);
    e.button.y = static_cast<float>(y);
    return e;
}

SDL_Event makeTextInput(const char* text) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.type = SDL_EVENT_TEXT_INPUT;
    e.text.text = text;
    return e;
}

SDL_Event makeKey(SDL_EventType type, SDL_Keycode key, Uint16 mod = 0) {
    SDL_Event e;
    SDL_memset(&e, 0, sizeof(e));
    e.type = type;
    e.key.type = type;
    e.key.key = key;
    e.key.mod = mod;
    return e;
}

void buildScene(GUIManager& mgr, int count, SceneRefs& out) {
    out = SceneRefs{};
    const int cols = std::max(4, static_cast<int>(std::ceil(std::sqrt(count * 1.6))));
    const int cellW = kWinW / cols;
    const int cellH = 64;
    const int rows = (count + cols - 1) / cols;

    for (int i = 0; i < count; ++i) {
        const int cx = (i % cols) * cellW;
        const int cy = (i / cols) * cellH;
        const int kind = i % 10;
        const bool anchored = (i % 5 == 4);  // co 5. widget kotwiczony
        GUIElement* added = nullptr;

        if (kind <= 1) {
            auto w = std::make_unique<Button>(mgr, cx + 4, cy + 8, cellW - 8, 40,
                                              "Btn" + std::to_string(i));
            out.buttons.push_back(w.get());
            added = w.get();
            mgr.addElement(std::move(w));
        } else if (kind == 2 || kind == 8) {
            auto w = std::make_unique<Label>(mgr, cx + 4, cy + 16, "Lbl" + std::to_string(i), 16);
            out.labels.push_back(w.get());
            added = w.get();
            mgr.addElement(std::move(w));
        } else if (kind == 3 || kind == 9) {
            auto w = std::make_unique<Slider>(mgr, cx + 4, cy + 8, cellW - 8, 40,
                                              0, 100, 50, Orientation::Horizontal);
            out.sliders.push_back(w.get());
            added = w.get();
            mgr.addElement(std::move(w));
        } else if (kind == 4) {
            auto w = std::make_unique<Checkbox>(mgr, cx + 4, cy + 12, 32, 32);
            added = w.get();
            mgr.addElement(std::move(w));
        } else if (kind == 5) {
            auto w = std::make_unique<TextInput>(mgr, cx + 4, cy + 8, cellW - 8, 40);
            w->setText("in" + std::to_string(i));
            out.inputs.push_back(w.get());
            added = w.get();
            mgr.addElement(std::move(w));
        } else if (kind == 6) {
            auto w = std::make_unique<TextArea>(mgr, cx + 4, cy + 4, cellW - 8, 56,
                                                constants::kDefaultFontPath, 14);
            w->setText("area" + std::to_string(i));
            out.areas.push_back(w.get());
            added = w.get();
            mgr.addElement(std::move(w));
        } else {  // kind == 7: panel z dzieckiem (głębszy DFS)
            auto w = std::make_unique<Panel>(mgr, cx + 4, cy + 8, cellW - 8, 48);
            auto child = std::make_unique<Label>(mgr, 6, 12, "P" + std::to_string(i), 14);
            out.labels.push_back(child.get());  // labelki paneli też w churnie
            w->addChild(std::move(child));
            added = w.get();
            mgr.addElement(std::move(w));
        }

        if (added && anchored) {
            // Różne kotwice żeby resize miał co liczyć (stretch/center/fill).
            if (i % 3 == 0) added->setAnchor(Anchor::horizontalStretch(5, 5));
            else if (i % 3 == 1) added->setAnchor(Anchor::center());
            else added->setAnchor(Anchor::topBar(8, 4, 4));
            added->updateLayout(kWinW, kWinH);
        }
        if (added) out.all.push_back(added);
    }
    (void)rows;
}

// Zygzak przez ekran: deterministyczne punkty hovera.
std::vector<SDL_Point> sweepPath(int steps) {
    std::vector<SDL_Point> pts;
    pts.reserve(static_cast<size_t>(steps));
    for (int i = 0; i < steps; ++i) {
        const int row = (i / 40) % 20;
        const int col = (row % 2 == 0) ? (i % 40) : (39 - (i % 40));
        pts.push_back({col * (kWinW / 40), row * (kWinH / 20)});
    }
    return pts;
}

SectionResult runHover(GUIManager& mgr, int widgets, double budgetSec) {
    const int targetOps = widgets * 20;
    auto path = sweepPath(targetOps);
    long long ops = 0;
    const auto t0 = Clock::now();
    for (auto& p : path) {
        if (std::chrono::duration<double>(Clock::now() - t0).count() > budgetSec) break;
        mgr.processEvent(makeMotion(p.x, p.y));
        ++ops;
    }
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"hover", widgets, ops, sec, ops / sec, sec * 1e9 / ops, ""};
}

SectionResult runClick(GUIManager& mgr, int widgets, const SceneRefs& scene) {
    if (scene.buttons.empty()) return {"click", widgets, 0, 0, 0, 0, "no-buttons"};
    long long ops = 0;
    const auto t0 = Clock::now();
    for (int r = 0; r < 5; ++r) {
        for (auto* b : scene.buttons) {
            auto c = b->getAbsolutePosition();
            const int x = c.x + b->getWidth() / 2, y = c.y + b->getHeight() / 2;
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, x, y));
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, x, y));
            ++ops;
        }
    }
    mgr.cleanup();
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"click", widgets, ops, sec, ops / sec, sec * 1e9 / ops, "events=" + std::to_string(ops * 2)};
}

SectionResult runSliderDrag(GUIManager& mgr, int widgets, const SceneRefs& scene) {
    if (scene.sliders.empty()) return {"slider_drag", widgets, 0, 0, 0, 0, "no-sliders"};
    long long ops = 0;
    const auto t0 = Clock::now();
    for (auto* s : scene.sliders) {
        auto c = s->getAbsolutePosition();
        const int y = c.y + s->getHeight() / 2;
        mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, c.x + 4, y));
        ++ops;
        for (int k = 1; k <= 20; ++k) {
            mgr.processEvent(makeMotion(c.x + 4 + k * (s->getWidth() - 8) / 20, y));
            ++ops;
        }
        mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT,
                                    c.x + s->getWidth() - 4, y));
        ++ops;
    }
    mgr.cleanup();
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"slider_drag", widgets, ops, sec, ops / sec, sec * 1e9 / ops, ""};
}

SectionResult runLabelChurn(GUIManager& mgr, int widgets, SceneRefs& scene) {
    if (scene.labels.empty()) return {"label_text", widgets, 0, 0, 0, 0, "no-labels"};
    (void)mgr;
    long long ops = 0;
    const auto t0 = Clock::now();
    for (int k = 0; k < 20; ++k) {
        for (size_t i = 0; i < scene.labels.size(); ++i) {
            scene.labels[i]->setText("T" + std::to_string(i) + "_" + std::to_string(k));
            ++ops;
        }
    }
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"label_text", widgets, ops, sec, ops / sec, sec * 1e9 / ops, ""};
}

SectionResult runTyping(GUIManager& mgr, int widgets, const SceneRefs& scene) {
    if (scene.inputs.empty()) return {"typing", widgets, 0, 0, 0, 0, "no-inputs"};
    static const char oneA[] = "a";
    long long ops = 0;
    const auto t0 = Clock::now();
    for (auto* in : scene.inputs) {
        auto c = in->getAbsolutePosition();
        mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT,
                                    c.x + in->getWidth() / 2, c.y + in->getHeight() / 2));
        ++ops;
        for (int k = 0; k < 10; ++k) {
            mgr.processEvent(makeTextInput(oneA));
            ++ops;
            if (k % 5 == 4) {
                mgr.processEvent(makeKey(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));
                ++ops;
            }
        }
    }
    mgr.cleanup();
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"typing", widgets, ops, sec, ops / sec, sec * 1e9 / ops, ""};
}

SectionResult runCreateDestroy(GUIManager& mgr, int widgets) {
    const int batch = std::max(10, widgets / 10);
    long long ops = 0;
    const auto t0 = Clock::now();
    for (int r = 0; r < 5; ++r) {
        std::vector<GUIElement*> added;
        for (int i = 0; i < batch; ++i) {
            auto b = std::make_unique<Button>(mgr, (i * 37) % kWinW, (i * 53) % kWinH,
                                              80, 30, "tmp");
            added.push_back(b.get());
            mgr.addElement(std::move(b));
            ++ops;
        }
        mgr.update();
        for (auto* e : added) e->markForDeletion();
        mgr.cleanup();
        ops += static_cast<long long>(added.size());
    }
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    return {"create_destroy", widgets, ops, sec, ops / sec, sec * 1e9 / ops,
            "batch=" + std::to_string(batch) + "x5"};
}

SectionResult runResize(GUIManager& mgr, SDL_Window* win, int widgets) {
    static const int sizes[][2] = {{kWinW, kWinH}, {800, 600}, {1920, 1080}};
    const int iters = 60;
    const auto t0 = Clock::now();
    for (int i = 0; i < iters; ++i) {
        const int w = sizes[i % 3][0], h = sizes[i % 3][1];
        SDL_SetWindowSize(win, w, h);  // prawdziwy resize okna
        mgr.handleResize(w, h);
    }
    SDL_SetWindowSize(win, kWinW, kWinH);
    mgr.handleResize(kWinW, kWinH);
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    const long long ops = iters + 1;
    return {"resize", widgets, ops, sec, ops / sec, sec * 1e9 / ops, "SDL_SetWindowSize+handleResize"};
}

SectionResult runFrameLoop(GUIManager& mgr, SDL_Renderer* renderer, int widgets,
                           SceneRefs& scene, double budgetSec) {
    // Rozgrzewka cache żeby mierzyć steady-state, nie pierwsze klatki.
    mgr.render();
    auto path = sweepPath(4000);
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> pickBtn(0, 0);  // nadpisane gdy są przyciski
    size_t pathIdx = 0, labelIdx = 0, inputIdx = 0;
    long long frames = 0, events = 0;
    std::vector<double> frameMs;
    frameMs.reserve(4096);
    const auto t0 = Clock::now();
    int frame = 0;
    while (std::chrono::duration<double>(Clock::now() - t0).count() < budgetSec) {
        const auto f0 = Clock::now();
        // 8 hover-eventów na klatkę wzdłuż zygzaka
        for (int k = 0; k < 8; ++k) {
            auto p = path[pathIdx++ % path.size()];
            mgr.processEvent(makeMotion(p.x, p.y));
            ++events;
        }
        // co 7. klatkę klik w losowy przycisk
        if (!scene.buttons.empty() && frame % 7 == 0) {
            auto* b = scene.buttons[rng() % scene.buttons.size()];
            auto c = b->getAbsolutePosition();
            const int x = c.x + b->getWidth() / 2, y = c.y + b->getHeight() / 2;
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, x, y));
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, x, y));
            events += 2;
        }
        // co 11. klatkę mini-drag slidera
        if (!scene.sliders.empty() && frame % 11 == 0) {
            auto* s = scene.sliders[rng() % scene.sliders.size()];
            auto c = s->getAbsolutePosition();
            const int y = c.y + s->getHeight() / 2;
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, c.x + 4, y));
            mgr.processEvent(makeMotion(c.x + s->getWidth() / 2, y));
            mgr.processEvent(makeButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT,
                                        c.x + s->getWidth() / 2, y));
            events += 3;
        }
        // co 5. klatkę zmiana tekstu jednej labelki + znak do inputa
        if (!scene.labels.empty() && frame % 5 == 0) {
            scene.labels[labelIdx++ % scene.labels.size()]->setText("F" + std::to_string(frame));
        }
        if (!scene.inputs.empty() && frame % 3 == 0) {
            static const char oneK[] = "k";
            mgr.processEvent(makeTextInput(oneK));
            ++events;
            (void)inputIdx;
        }
        (void)pickBtn;

        mgr.update();
        mgr.cleanup();
        SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
        SDL_RenderClear(renderer);
        mgr.render();
        SDL_RenderPresent(renderer);
        frameMs.push_back(std::chrono::duration<double>(Clock::now() - f0).count() * 1000.0);
        ++frames;
        ++frame;
        // realne eventy systemowe (zamknięcie okna kończy wcześniej)
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                budgetSec = 0;  // przerwij pętlę po tej klatce
                break;
            }
            mgr.processEvent(e);
        }
    }
    const double sec = std::chrono::duration<double>(Clock::now() - t0).count();
    std::sort(frameMs.begin(), frameMs.end());
    const double avg = std::accumulate(frameMs.begin(), frameMs.end(), 0.0) /
                       static_cast<double>(frameMs.size());
    const double p95 = frameMs[frameMs.size() * 95 / 100];
    char extra[160];
    std::snprintf(extra, sizeof(extra), "fps=%.1f avg=%.2fms p95=%.2fms events=%lld",
                  static_cast<double>(frames) / sec, avg, p95, events);
    return {"frame_loop", widgets, frames, sec, frames / sec, sec * 1e9 / frames, extra};
}

void printRow(const SectionResult& r) {
    if (r.ops == 0) {
        std::printf("%-14s w=%-5d  SKIP (%s)\n", r.section.c_str(), r.widgets, r.extra.c_str());
        return;
    }
    std::printf("%-14s w=%-5d  ops=%-7lld  %9.1f ops/s  %10.1f ns/op  %s\n",
                r.section.c_str(), r.widgets, r.ops, r.perSec, r.nsPerOp,
                r.extra.c_str());
}

void saveJson(const std::string& path, const std::vector<SectionResult>& all,
              const std::string& backend) {
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "bench: cannot write %s\n", path.c_str());
        return;
    }
    std::fprintf(f, "{\"backend\":\"%s\",\"results\":[\n", backend.c_str());
    for (size_t i = 0; i < all.size(); ++i) {
        const auto& r = all[i];
        std::fprintf(f,
                     "  {\"widgets\":%d,\"section\":\"%s\",\"ops\":%lld,"
                     "\"sec\":%.4f,\"per_sec\":%.2f,\"ns_per_op\":%.1f,\"extra\":\"%s\"}%s\n",
                     r.widgets, r.section.c_str(), r.ops, r.sec, r.perSec,
                     r.nsPerOp, r.extra.c_str(), i + 1 < all.size() ? "," : "");
    }
    std::fprintf(f, "]}\n");
    std::fclose(f);
    std::printf("saved baseline: %s (%zu rows)\n", path.c_str(), all.size());
}

// Minimalny parser naszego własnego JSON: szuka linii
// {"widgets":N,"section":"S",...,"per_sec":X,...}.
void compareJson(const std::string& path, const std::vector<SectionResult>& all) {
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) {
        std::fprintf(stderr, "bench: cannot read %s\n", path.c_str());
        return;
    }
    std::string content;
    char buf[4096];
    while (std::fgets(buf, sizeof(buf), f)) content += buf;
    std::fclose(f);
    std::printf("\n--- compare vs %s (per_sec, + = szybciej niż baseline) ---\n",
                path.c_str());
    for (const auto& r : all) {
        if (r.ops == 0) continue;
        char key[128];
        std::snprintf(key, sizeof(key), "\"widgets\":%d,\"section\":\"%s\"",
                      r.widgets, r.section.c_str());
        auto pos = content.find(key);
        if (pos == std::string::npos) {
            std::printf("%-14s w=%-5d  (brak w baseline)\n", r.section.c_str(), r.widgets);
            continue;
        }
        auto pp = content.find("\"per_sec\":", pos);
        if (pp == std::string::npos) continue;
        const double base = std::atof(content.c_str() + pp + 10);
        const double delta = base > 0 ? (r.perSec - base) / base * 100.0 : 0.0;
        std::printf("%-14s w=%-5d  teraz=%9.1f  baza=%9.1f  %+.1f%%\n",
                    r.section.c_str(), r.widgets, r.perSec, base, delta);
    }
}

}  // namespace

int main(int argc, char** argv) {
    int widgets = 200;
    bool sweep = false;
    double seconds = 5.0;
    std::string backend = "sdl";
    bool hidden = false;
    std::string savePath, comparePath;

    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        auto needVal = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "bench: %s needs a value\n", name);
                std::exit(1);
            }
            return argv[++i];
        };
        if (a == "--widgets" || a == "-n") widgets = std::atoi(needVal("--widgets"));
        else if (a == "--sweep") sweep = true;
        else if (a == "--seconds" || a == "-s") seconds = std::atof(needVal("--seconds"));
        else if (a == "--backend") backend = needVal("--backend");
        else if (a == "--hidden") hidden = true;
        else if (a == "--save") savePath = needVal("--save");
        else if (a == "--compare") comparePath = needVal("--compare");
        else if (a == "--help" || a == "-h") {
            std::printf("usage: bench_gui [--widgets N] [--sweep] [--seconds S]\n"
                        "                 [--backend sdl|vulkan] [--hidden]\n"
                        "                 [--save F.json] [--compare F.json]\n");
            return 0;
        } else {
            std::fprintf(stderr, "bench: unknown arg %s (see --help)\n", argv[i]);
            return 1;
        }
    }
    if (hidden) setenv("SDL_GUI_HIDDEN", "1", 1);

    std::vector<int> counts = sweep ? std::vector<int>{100, 200, 1000}
                                    : std::vector<int>{widgets};

    try {
        std::vector<SectionResult> all;
        for (int n : counts) {
            std::unique_ptr<SDLApp> app;
            if (backend == "vulkan") {
                app = std::make_unique<SDLApp>("bench_gui (vulkan)", kWinW, kWinH,
                                               true, GPU_VULKAN);
            } else {
                app = std::make_unique<SDLApp>("bench_gui", kWinW, kWinH, true);
            }
            SDL_Renderer* renderer = app->getRenderer();
            auto mgr = std::make_unique<GUIManager>(renderer, Viewport{kWinW, kWinH});
            mgr->setTheme(Theme::createDefaultTheme());

            SceneRefs scene;
            buildScene(*mgr, n, scene);
            mgr->update();
            mgr->cleanup();

            std::printf("== widgets=%d (btn=%zu lbl=%zu sld=%zu inp=%zu area=%zu, backend=%s) ==\n",
                        n, scene.buttons.size(), scene.labels.size(), scene.sliders.size(),
                        scene.inputs.size(), scene.areas.size(), backend.c_str());

            // Kolejność: najpierw czyste eventy (bez rendera), resize, na końcu
            // frame_loop z present (po nim scena jest przepięta rozmiarami,
            // ale to już koniec tej wartości N).
            all.push_back(runHover(*mgr, n, std::min(seconds, 10.0)));
            printRow(all.back());
            all.push_back(runClick(*mgr, n, scene));
            printRow(all.back());
            all.push_back(runSliderDrag(*mgr, n, scene));
            printRow(all.back());
            all.push_back(runLabelChurn(*mgr, n, scene));
            printRow(all.back());
            all.push_back(runTyping(*mgr, n, scene));
            printRow(all.back());
            all.push_back(runCreateDestroy(*mgr, n));
            printRow(all.back());
            all.push_back(runResize(*mgr, app->getWindow(), n));
            printRow(all.back());
            all.push_back(runFrameLoop(*mgr, renderer, n, scene, seconds));
            printRow(all.back());
            std::printf("\n");
            // mgr + app giną tu (kolejne N buduje scenę od zera).
        }
        if (!savePath.empty()) saveJson(savePath, all, backend);
        if (!comparePath.empty()) compareJson(comparePath, all);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "bench failed: %s\n", e.what());
        return 1;
    }
    return 0;
}
