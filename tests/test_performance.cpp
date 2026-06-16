#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/panel.hpp"
#include "../src/gui_manager.hpp"

#include <chrono>
#include <vector>
#include <memory>
#include "../src/logger.hpp"

using Clock = std::chrono::high_resolution_clock;
using DurationMs = std::chrono::duration<double, std::milli>;
using DurationUs = std::chrono::duration<double, std::micro>;

static void printBenchmarkResult(const std::string& operation, int count, double totalMs, double avgUs) {
    LOG_INFO("PerfTest", "[BENCHMARK] {} | count: {} | total: {:.3f} ms | avg: {:.3f} us",
             operation, count, totalMs, avgUs);
}

TEST_CASE("Performance benchmarks", "[performance]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Element Creation Benchmark") {
        LOG_INFO("PerfTest", "\n=== Element Creation Benchmark ===");

        for (int count : {100, 500, 1000}) {
            auto start = Clock::now();
            
            std::vector<std::unique_ptr<Button>> buttons;
            buttons.reserve(count);
            for (int i = 0; i < count; ++i) {
                buttons.push_back(std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, ""));
            }
            
            auto end = Clock::now();
            double totalMs = DurationMs(end - start).count();
            double avgUs = DurationUs(end - start).count() / count;
            
            printBenchmarkResult("Button creation", count, totalMs, avgUs);
            REQUIRE(buttons.size() == static_cast<size_t>(count));
        }
    }

    SECTION("Element Deletion Benchmark") {
        LOG_INFO("PerfTest", "\n=== Element Deletion Benchmark ===");

        for (int count : {100, 500, 1000}) {
            std::vector<Button*> buttonPtrs;
            for (int i = 0; i < count; ++i) {
                auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
                buttonPtrs.push_back(button.get());
                manager.addElement(std::move(button));
            }
            
            REQUIRE(buttonPtrs.size() == static_cast<size_t>(count));
            
            auto markStart = Clock::now();
            for (Button* btn : buttonPtrs) {
                btn->markForDeletion();
            }
            auto markEnd = Clock::now();
            double markMs = DurationMs(markEnd - markStart).count();
            double markAvgUs = DurationUs(markEnd - markStart).count() / count;
            printBenchmarkResult("markForDeletion", count, markMs, markAvgUs);
            
            auto cleanupStart = Clock::now();
            manager.cleanup();
            auto cleanupEnd = Clock::now();
            double cleanupMs = DurationMs(cleanupEnd - cleanupStart).count();
            printBenchmarkResult("cleanup() after marking", count, cleanupMs, cleanupMs / count);
        }
    }

    SECTION("Render Benchmark") {
        LOG_INFO("PerfTest", "\n=== Render Benchmark ===");

        for (int elemCount : {10, 50, 100}) {
            std::vector<Button*> buttonPtrs;
            for (int i = 0; i < elemCount; ++i) {
                auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
                buttonPtrs.push_back(button.get());
                manager.addElement(std::move(button));
            }
            
            const int frames = 100;
            auto start = Clock::now();
            
            for (int f = 0; f < frames; ++f) {
                manager.render();
            }
            
            auto end = Clock::now();
            double totalMs = DurationMs(end - start).count();
            double avgFrameMs = totalMs / frames;
            double avgElemUs = DurationUs(end - start).count() / (frames * elemCount);
            
            printBenchmarkResult("Render " + std::to_string(elemCount) + " elements for " + std::to_string(frames) + " frames", 
                                 frames * elemCount, totalMs, avgElemUs);
            LOG_INFO("PerfTest", "  -> Average frame time: {:.3f} ms", avgFrameMs);
            
            for (Button* btn : buttonPtrs) {
                btn->markForDeletion();
            }
            manager.cleanup();
        }
    }

    SECTION("Style Setting Benchmark") {
        LOG_INFO("PerfTest", "\n=== Style Setting Benchmark ===");

        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "");
        Button* btn = button.get();
        manager.addElement(std::move(button));
        
        const int iterations = 10000;
        Style style;
        style.backgroundColor = SDL_Color{100, 100, 100, 255};
        style.textColor = SDL_Color{200, 200, 200, 255};
        style.borderRadius = 5;
        
        auto start = Clock::now();
        for (int i = 0; i < iterations; ++i) {
            btn->setStyle(ElementState::Normal, style);
            btn->setStyle(ElementState::Hover, style);
        }
        auto end = Clock::now();
        
        double totalMs = DurationMs(end - start).count();
        double avgUs = DurationUs(end - start).count() / iterations;
        
        printBenchmarkResult("setStyle()", iterations, totalMs, avgUs);
        
        btn->markForDeletion();
        manager.cleanup();
    }

    SECTION("Label Text Change Benchmark") {
        LOG_INFO("PerfTest", "\n=== Label Text Change Benchmark ===");

        const int labelCount = 100;
        const int changesPerLabel = 100;
        std::vector<Label*> labelPtrs;
        
        for (int i = 0; i < labelCount; ++i) {
            auto label = std::make_unique<Label>(manager, i * 10 % 800, i * 5 % 600, "", 16);
            labelPtrs.push_back(label.get());
            manager.addElement(std::move(label));
        }
        
        auto start = Clock::now();
        
        for (int c = 0; c < changesPerLabel; ++c) {
            for (Label* lbl : labelPtrs) {
                lbl->setText("Text" + std::to_string(c));
            }
        }
        
        auto end = Clock::now();
        int totalOps = labelCount * changesPerLabel;
        double totalMs = DurationMs(end - start).count();
        double avgUs = DurationUs(end - start).count() / totalOps;
        
        printBenchmarkResult("Label setText()", totalOps, totalMs, avgUs);
        
        for (Label* lbl : labelPtrs) {
            lbl->markForDeletion();
        }
        manager.cleanup();
    }

    SECTION("Scaling Benchmark") {
        LOG_INFO("PerfTest", "\n=== Scaling Benchmark ===");

        const int elemCount = 1000;
        const int setSizeCalls = 10;
        std::vector<Button*> buttonPtrs;
        
        for (int i = 0; i < elemCount; ++i) {
            auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
            buttonPtrs.push_back(button.get());
            manager.addElement(std::move(button));
        }
        
        auto start = Clock::now();
        
        for (int c = 0; c < setSizeCalls; ++c) {
            for (Button* btn : buttonPtrs) {
                btn->setSize(50 + c, 25 + c);
            }
        }
        
        auto end = Clock::now();
        int totalOps = elemCount * setSizeCalls;
        double totalMs = DurationMs(end - start).count();
        double avgUs = DurationUs(end - start).count() / totalOps;
        
        printBenchmarkResult("setSize()", totalOps, totalMs, avgUs);
        
        for (Button* btn : buttonPtrs) {
            btn->markForDeletion();
        }
        manager.cleanup();
    }

    SECTION("Positioning Benchmark") {
        LOG_INFO("PerfTest", "\n=== Positioning Benchmark ===");

        const int elemCount = 1000;
        const int setPosCalls = 10;
        std::vector<Button*> buttonPtrs;
        
        for (int i = 0; i < elemCount; ++i) {
            auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
            buttonPtrs.push_back(button.get());
            manager.addElement(std::move(button));
        }
        
        auto start = Clock::now();
        
        for (int c = 0; c < setPosCalls; ++c) {
            for (Button* btn : buttonPtrs) {
                btn->setPosition((c * 10) % 800, (c * 5) % 600);
            }
        }
        
        auto end = Clock::now();
        int totalOps = elemCount * setPosCalls;
        double totalMs = DurationMs(end - start).count();
        double avgUs = DurationUs(end - start).count() / totalOps;
        
        printBenchmarkResult("setPosition()", totalOps, totalMs, avgUs);
        
        for (Button* btn : buttonPtrs) {
            btn->markForDeletion();
        }
        manager.cleanup();
    }

    SECTION("Hierarchy Benchmark") {
        LOG_INFO("PerfTest", "\n=== Hierarchy Benchmark ===");

        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 600);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));
        
        const int iterations = 100;
        
        auto start = Clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            auto child1 = std::make_unique<Button>(manager, 10, 10, 50, 20, "");
            auto child2 = std::make_unique<Button>(manager, 70, 10, 50, 20, "");
            auto child3 = std::make_unique<Button>(manager, 130, 10, 50, 20, "");
            
            panelPtr->addChild(std::move(child1));
            panelPtr->addChild(std::move(child2));
            panelPtr->addChild(std::move(child3));
            
            panelPtr->clearChildren();
        }
        
        auto end = Clock::now();
        int totalOps = iterations * 4;
        double totalMs = DurationMs(end - start).count();
        double avgUs = DurationUs(end - start).count() / totalOps;
        
        printBenchmarkResult("addChild() + clearChildren()", totalOps, totalMs, avgUs);
        
        panelPtr->markForDeletion();
        manager.cleanup();
    }
}

TEST_CASE("Detailed render timing", "[performance][render]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Render cache invalidation timing") {
        LOG_INFO("PerfTest", "\n=== Render Cache Invalidation Timing ===");

        const int elemCount = 100;
        std::vector<Button*> buttonPtrs;
        
        for (int i = 0; i < elemCount; ++i) {
            auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
            buttonPtrs.push_back(button.get());
            manager.addElement(std::move(button));
        }
        
        manager.render();
        
        auto dirtyStart = Clock::now();
        for (Button* btn : buttonPtrs) {
            btn->markDirty();
        }
        auto dirtyEnd = Clock::now();
        double dirtyMs = DurationMs(dirtyEnd - dirtyStart).count();
        printBenchmarkResult("markDirty() on " + std::to_string(elemCount) + " elements", elemCount, dirtyMs, dirtyMs * 1000 / elemCount);
        
        auto renderStart = Clock::now();
        manager.render();
        auto renderEnd = Clock::now();
        double renderMs = DurationMs(renderEnd - renderStart).count();
        printBenchmarkResult("render() after cache invalidation", elemCount, renderMs, renderMs * 1000 / elemCount);
        
        for (Button* btn : buttonPtrs) {
            btn->markForDeletion();
        }
        manager.cleanup();
    }

    SECTION("Clean render (cached textures) timing") {
        LOG_INFO("PerfTest", "\n=== Clean Render (Cached Textures) Timing ===");

        const int elemCount = 100;
        std::vector<Button*> buttonPtrs;
        
        for (int i = 0; i < elemCount; ++i) {
            auto button = std::make_unique<Button>(manager, i * 10 % 800, i * 5 % 600, 80, 30, "");
            buttonPtrs.push_back(button.get());
            manager.addElement(std::move(button));
        }
        
        manager.render();
        
        const int frames = 50;
        auto start = Clock::now();
        for (int f = 0; f < frames; ++f) {
            manager.render();
        }
        auto end = Clock::now();
        
        double totalMs = DurationMs(end - start).count();
        double avgFrameMs = totalMs / frames;
        printBenchmarkResult("Cached render " + std::to_string(frames) + " frames", frames * elemCount, totalMs, avgFrameMs * 1000 / elemCount);
        LOG_INFO("PerfTest", "  -> Average frame time (cached): {:.3f} ms", avgFrameMs);
        
        for (Button* btn : buttonPtrs) {
            btn->markForDeletion();
        }
        manager.cleanup();
    }
}

TEST_CASE("Memory allocation patterns", "[performance][memory]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Batch creation vs incremental creation") {
        LOG_INFO("PerfTest", "\n=== Batch vs Incremental Creation ===");

        const int count = 500;
        
        auto batchStart = Clock::now();
        std::vector<std::unique_ptr<Button>> batchButtons;
        batchButtons.reserve(count);
        for (int i = 0; i < count; ++i) {
            batchButtons.push_back(std::make_unique<Button>(manager, i % 800, i % 600, 80, 30, ""));
        }
        auto batchEnd = Clock::now();
        double batchMs = DurationMs(batchEnd - batchStart).count();
        printBenchmarkResult("Batch creation (reserve first)", count, batchMs, batchMs * 1000 / count);
        
        auto incrStart = Clock::now();
        std::vector<std::unique_ptr<Button>> incrButtons;
        for (int i = 0; i < count; ++i) {
            incrButtons.push_back(std::make_unique<Button>(manager, i % 800, i % 600, 80, 30, ""));
        }
        auto incrEnd = Clock::now();
        double incrMs = DurationMs(incrEnd - incrStart).count();
        printBenchmarkResult("Incremental creation (no reserve)", count, incrMs, incrMs * 1000 / count);
        
        double diffPercent = ((incrMs - batchMs) / batchMs) * 100.0;
        LOG_INFO("PerfTest", "  -> Difference: {:.1f}%", diffPercent);
    }

    SECTION("Panel child management overhead") {
        LOG_INFO("PerfTest", "\n=== Panel Child Management Overhead ===");

        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 600);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));
        
        const int childCount = 100;
        
        auto addStart = Clock::now();
        for (int i = 0; i < childCount; ++i) {
            panelPtr->addChild(std::make_unique<Button>(manager, i * 5, i * 3, 40, 20, ""));
        }
        auto addEnd = Clock::now();
        double addMs = DurationMs(addEnd - addStart).count();
        printBenchmarkResult("addChild() sequential", childCount, addMs, addMs * 1000 / childCount);
        
        REQUIRE(panelPtr->getChildren().size() == static_cast<size_t>(childCount));
        
        auto clearStart = Clock::now();
        panelPtr->clearChildren();
        auto clearEnd = Clock::now();
        double clearMs = DurationMs(clearEnd - clearStart).count();
        printBenchmarkResult("clearChildren() on " + std::to_string(childCount) + " children", childCount, clearMs, clearMs * 1000 / childCount);
        
        panelPtr->markForDeletion();
        manager.cleanup();
    }
}