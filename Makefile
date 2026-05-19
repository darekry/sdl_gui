# define the C compiler to use
# CC = gcc
CC = clang-22
# define the Cpp compiler to use
# CXX    = g++
CXX    = clang++-22
# Flagi wspólne dla obu trybów

MODULE_CACHE_DIR := modules_cache

# Moduły prekompilowane C++23 - osobne wersje dla debug i release
# Debug: bez optymalizacji, z informacjami debugowania
STD_PCM_DEBUG := $(MODULE_CACHE_DIR)/std_debug.pcm
STD_COMPAT_PCM_DEBUG := $(MODULE_CACHE_DIR)/std.compat_debug.pcm
MODULE_PCMS_DEBUG := $(STD_PCM_DEBUG) $(STD_COMPAT_PCM_DEBUG)

# Release: z optymalizacjami (-O3, -march=native)
STD_PCM_RELEASE := $(MODULE_CACHE_DIR)/std_release.pcm
STD_COMPAT_PCM_RELEASE := $(MODULE_CACHE_DIR)/std.compat_release.pcm
MODULE_PCMS_RELEASE := $(STD_PCM_RELEASE) $(STD_COMPAT_PCM_RELEASE)

# Wybór odpowiednich modułów w zależności od trybu
ifeq ($(RELEASE),1)
    STD_PCM := $(STD_PCM_RELEASE)
    STD_COMPAT_PCM := $(STD_COMPAT_PCM_RELEASE)
    MODULE_PCMS := $(MODULE_PCMS_RELEASE)
else
    STD_PCM := $(STD_PCM_DEBUG)
    STD_COMPAT_PCM := $(STD_COMPAT_PCM_DEBUG)
    MODULE_PCMS := $(MODULE_PCMS_DEBUG)
endif


COMMON_FLAGS = -Wall
COMMON_FLAGS += -Wextra
COMMON_FLAGS += -Wshadow
COMMON_FLAGS += -Wconversion
COMMON_FLAGS += -Wsign-conversion
COMMON_FLAGS += -Wfloat-equal
COMMON_FLAGS += -Wformat=2
COMMON_FLAGS += -Wnon-virtual-dtor
# COMMON_FLAGS += -Wold-style-cast
COMMON_FLAGS += -Woverloaded-virtual
COMMON_FLAGS += -Wreorder
COMMON_FLAGS += -Wzero-as-null-pointer-constant
COMMON_FLAGS += -Wunreachable-code
COMMON_FLAGS += -Wstrict-aliasing
COMMON_FLAGS += -Wpedantic

# Dodatkowe flagi warningów (zakomentowane - można włączyć stopniowo)
COMMON_FLAGS += -Wcast-align
COMMON_FLAGS += -Wcast-qual
COMMON_FLAGS += -Wctor-dtor-privacy
COMMON_FLAGS += -Wdisabled-optimization
# COMMON_FLAGS += -Wdocumentation
COMMON_FLAGS += -Winit-self
# COMMON_FLAGS += -Wlogical-op
COMMON_FLAGS += -Wmissing-declarations
# COMMON_FLAGS += -Wmissing-include-dirs
# COMMON_FLAGS += -Wnoexcept
# COMMON_FLAGS += -Wstrict-overflow=5
# COMMON_FLAGS += -Wswitch-default
# COMMON_FLAGS += -Wundef
COMMON_FLAGS += -Winline
# COMMON_FLAGS += -Wmissing-field-initializers
# COMMON_FLAGS += -Wthread-safety
COMMON_FLAGS += -Wdouble-promotion
COMMON_FLAGS += -Wnull-dereference
COMMON_FLAGS += -Wextra-semi
COMMON_FLAGS += -Wsign-promo
# COMMON_FLAGS += -Werror

COMMON_FLAGS += $(shell sdl2-config --cflags)
COMMON_FLAGS += -stdlib=libc++
COMMON_FLAGS += -std=c++23
COMMON_FLAGS += -I/usr/include/SDL2





COMMON_FLAGS  +=-fmodule-file=std=$(STD_PCM)
COMMON_FLAGS  +=-fmodule-file=std.compat=$(STD_COMPAT_PCM)


# Flagi specyficzne dla trybu Release
RELEASE_FLAGS = -O3
RELEASE_FLAGS += -march=native
RELEASE_FLAGS += -flto
RELEASE_FLAGS += -DNDEBUG

RELEASE_CXXFLAGS := $(COMMON_FLAGS) $(RELEASE_FLAGS)

# Flagi specyficzne dla trybu Debug
DEBUG_FLAGS = -g
DEBUG_FLAGS += -O0
DEBUG_FLAGS += -fsanitize=address,undefined
DEBUG_FLAGS += -DDEBUG=1

# Ustaw CXXFLAGS w zależności od zmiennej DEBUG
ifeq ($(RELEASE),1)
    # Tryb Release
    CXXFLAGS := $(COMMON_FLAGS) $(RELEASE_FLAGS)
else
    # Domyślny tryb Debug
    CXXFLAGS := $(COMMON_FLAGS) $(DEBUG_FLAGS)
endif

# --- Konfiguracja sanitizerów ---
SANITIZE_FLAGS := -fsanitize=address,undefined
ifdef TEST_ENABLE_SANITIZERS
TEST_SANITIZE_FLAGS := $(SANITIZE_FLAGS)
else
TEST_SANITIZE_FLAGS :=
endif
# Flagi kompilacji dla testów: usuń domyślny sanitizer z CXXFLAGS i opcjonalnie dodaj z powrotem
TEST_CXXFLAGS := $(filter-out $(SANITIZE_FLAGS),$(CXXFLAGS)) $(TEST_SANITIZE_FLAGS)

# define any compile-time flags for C
CFLAGS := $(CXXFLAGS) # Zakładam, że CFLAGS mogą być takie same jak CXXFLAGS, lub zostaną usunięte/dostosowane

# define C Preprocessor flags
CPPFLAGS := -Isrc -isystem lib

# define library paths in addition to /usr/lib
LDFLAGS  = $(shell sdl2-config --libs)
LDFLAGS += -lSDL2_image
LDFLAGS += -lSDL2_ttf
LDFLAGS += -lSDL2_gfx

# define output directory
OUTPUT := output
DIST_DIR := dist

# define source directory
SRC := src
TESTS_DIR := tests

# define include directory
INCLUDE := include

# define lib directory
LIB := lib

# Define unity build files
UNITY_SOURCE := $(OUTPUT)/all.cpp
UNITY_OBJECT := $(OUTPUT)/all.o
TEST_UNITY_OBJECT := $(OUTPUT)/all_test.o


# --- Źródła i obiekty ---
# Wszystkie pliki źródłowe C++ w projekcie
CPPSOURCES := $(wildcard $(SRC)/*.cpp)
LIB_SRC := lib/tinyxml2.cpp

# --- Konfiguracja tradycyjnej kompilacji (dla compile_commands.json) ---
# Cel `non_unity` kompiluje każdy plik .cpp osobno.
# Służy do generowania `compile_commands.json` za pomocą `bear` lub `intercept-build`:
# $ bear -- make non_unity
#
# Lista plików obiektowych dla tradycyjnej kompilacji (każdy .cpp -> .o)
NON_UNITY_OBJECTS := $(patsubst $(SRC)/%.cpp,$(OUTPUT)/%.o,$(CPPSOURCES))

# Reguła wzorcowa do kompilacji pojedynczego pliku źródłowego na plik obiektowy
# Ta reguła jest używana tylko przez cel `non_unity`
$(OUTPUT)/%.o: $(SRC)/%.cpp | $(OUTPUT) 
	@echo "Kompilowanie (non-unity): $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# --- Konfiguracja testów ---
# Pliki źródłowe testów
TEST_SRC_FILES  := $(wildcard $(TESTS_DIR)/test_*.cpp)
# Plik pomocniczy testów
TEST_HELPER_SRC := $(TESTS_DIR)/test_helper.cpp
TEST_HELPER_OBJ := $(OUTPUT)/test_helper.o
# Pliki wykonywalne testów
TEST_EXECS      := $(patsubst $(TESTS_DIR)/test_%.cpp,$(OUTPUT)/test_%,$(filter-out $(TESTS_DIR)/test_helper.cpp $(TESTS_DIR)/test_main.cpp,$(TEST_SRC_FILES)))
# --- Konfiguracja przykładów ---
EXAMPLE_SRC_FILES := $(wildcard examples/*.cpp)
EXAMPLE_EXECS     := $(patsubst examples/%.cpp,$(OUTPUT)/%,$(EXAMPLE_SRC_FILES))

# --- Główne cele ---

release: $(DIST_DIR)/libsdl_gui.a $(DIST_DIR)/libsdl_gui.so $(DIST_DIR)/sdl_gui.hpp modules_release
	@echo "Release build finished. Artifacts are in $(DIST_DIR)/"

examples: $(UNITY_OBJECT) $(EXAMPLE_EXECS) modules_all
all:  examples

# Cel do tradycyjnej kompilacji (non-unity)
# Buduje wszystkie pliki obiektowe, ale nie linkuje ich.
# To wystarczy, aby `bear` przechwycił komendy kompilacji.
non_unity: $(NON_UNITY_OBJECTS) modules
	@echo "Tradycyjna kompilacja (non-unity) zakończona. Obiekty znajdują się w $(OUTPUT)/"

# Cel do uruchamiania testów
test: $(TEST_UNITY_OBJECT) $(TEST_EXECS) modules
	@echo "Running all tests..."
	@for t in $(TEST_EXECS); do \
	ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 ./$$t || exit 1; \
	done
	@echo "All tests passed successfully."

# --- Reguły kompilacji ---

# Pliki wykonywalne testów
# Każdy test jest linkowany z całą biblioteką i pomocnikiem testów
$(OUTPUT)/test_%: $(TESTS_DIR)/test_%.cpp $(TEST_UNITY_OBJECT) $(TEST_HELPER_OBJ) $(OUTPUT)/catch_amalgamated.o
	$(CXX) $(TEST_CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Pliki wykonywalne przykładów
$(OUTPUT)/example_%: examples/example_%.cpp $(UNITY_OBJECT)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Reguła dla unity build
$(UNITY_OBJECT): $(UNITY_SOURCE)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Reguła dla unity build (wersja testowa bez sanitizerów domyślnie)
$(TEST_UNITY_OBJECT): $(UNITY_SOURCE)
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

define UNITY_BUILD_CONTENT
// This file is generated by Makefile for unity build
// Do not edit directly.

$(foreach file,$(CPPSOURCES) $(LIB_SRC),#include "$(notdir $(file))"
)
endef

$(UNITY_SOURCE): $(CPPSOURCES) $(LIB_SRC) | $(OUTPUT)
	$(file >$@,$(UNITY_BUILD_CONTENT))


$(TEST_HELPER_OBJ): $(TEST_HELPER_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUTPUT)/catch_amalgamated.o: $(LIB)/catch_amalgamated.cpp $(LIB)/catch_amalgamated.hpp
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_CXXFLAGS) -I$(LIB) -c $< -o $@

# --- Reguły dla wydania (release) ---

# Pliki nagłówkowe i obiektowe
# Pliki nagłówkowe i obiektowe
# Ręcznie zdefiniowana kolejność plików nagłówkowych, aby spełnić zależności między klasami.
# Jest to konieczne do poprawnego stworzenia jednego, połączonego pliku nagłówkowego.
HPP_SOURCES := \
	$(SRC)/easing.hpp \
	$(SRC)/sdl_deleters.hpp \
	$(SRC)/layout_parser.hpp \
	$(SRC)/sgml_parser.hpp \
	$(SRC)/json_parser.hpp \
	$(SRC)/timer_manager.hpp \
	$(SRC)/animation_manager.hpp \
	$(SRC)/style.hpp \
	$(SRC)/font_manager.hpp \
	$(SRC)/texture_manager.hpp \
	$(SRC)/theme.hpp \
	$(SRC)/gui.hpp \
	$(SRC)/label.hpp \
	$(SRC)/panel.hpp \
	$(SRC)/button.hpp \
	$(SRC)/checkbox.hpp \
	$(SRC)/slider.hpp \
	$(SRC)/text_input.hpp \
	$(SRC)/canvas.hpp \
	$(SRC)/cursor.hpp \
	$(SRC)/animated_image.hpp \
	$(SRC)/radio_button.hpp \
	$(SRC)/radio_group.hpp \
	$(SRC)/tab_control.hpp \
	$(SRC)/text_area.hpp \
	$(SRC)/combobox.hpp \
	$(SRC)/context_menu.hpp \
	$(SRC)/gui_manager.hpp \
	$(SRC)/sdl_app.hpp
# Definicja plików obiektowych dla tinyxml2
TINYXML2_SRC := $(LIB)/tinyxml2.cpp
TINYXML2_OBJ := $(patsubst $(LIB)/%.cpp,$(OUTPUT)/release/%.o,$(TINYXML2_SRC))
TINYXML2_PIC_OBJ := $(patsubst $(LIB)/%.cpp,$(OUTPUT)/release/%.pic.o,$(TINYXML2_SRC))

LIB_OBJECTS := $(patsubst $(SRC)/%.cpp,$(OUTPUT)/release/%.o,$(CPPSOURCES)) $(TINYXML2_OBJ)
LIB_PIC_OBJECTS := $(patsubst $(SRC)/%.cpp,$(OUTPUT)/release/%.pic.o,$(CPPSOURCES)) $(TINYXML2_PIC_OBJ)

# Połączony plik nagłówkowy
$(DIST_DIR)/sdl_gui.hpp: $(HPP_SOURCES) | $(DIST_DIR)
	@echo "Creating combined header file..."
	@echo "// Auto-generated header. Do not edit." > $@
	@echo "#pragma once" >> $@
	@echo "" >> $@
	@echo "// C++ Standard Library" >> $@
	@echo "#include <cmath>" >> $@
	@echo "#include <functional>" >> $@
	@echo "#include <iostream>" >> $@
	@echo "#include <map>" >> $@
	@echo "#include <memory>" >> $@
	@echo "#include <numeric>" >> $@
	@echo "#include <optional>" >> $@
	@echo "#include <string>" >> $@
	@echo "#include <string_view>" >> $@
	@echo "#include <variant>" >> $@
	@echo "#include <vector>" >> $@
	@echo "" >> $@
	@echo "// External libraries" >> $@
	@echo "#include <SDL2/SDL.h>" >> $@
	@echo "#include <SDL2/SDL_image.h>" >> $@
	@echo "#include <SDL2/SDL_ttf.h>" >> $@
	@echo "#include <SDL2/SDL_pixels.h>" >> $@
	@echo "#include <SDL2/SDL_log.h>" >> $@
	@echo "#include <SDL2/SDL2_gfxPrimitives.h>" >> $@
	@echo "" >> $@
	@echo "// Project libraries" >> $@
	@echo "#include \"tinyxml2.h\"" >> $@
	@echo "" >> $@
	@# Łączymy wszystkie pliki nagłówkowe, dodając nową linię po każdym pliku i usuwając zbędne dyrektywy
	@(for f in $(HPP_SOURCES); do \
		cat $$f; \
		echo; \
	done) | sed \
		-e '/#include "easing.hpp"/d' \
		-e '/#include "sdl_deleters.hpp"/d' \
		-e '/#include "layout_parser.hpp"/d' \
		-e '/#include "sgml_parser.hpp"/d' \
		-e '/#include "json_parser.hpp"/d' \
		-e '/#include "timer_manager.hpp"/d' \
		-e '/#include "animation_manager.hpp"/d' \
		-e '/#include "style.hpp"/d' \
		-e '/#include "font_manager.hpp"/d' \
		-e '/#include "texture_manager.hpp"/d' \
		-e '/#include "theme.hpp"/d' \
		-e '/#include "gui.hpp"/d' \
		-e '/#include "label.hpp"/d' \
		-e '/#include "panel.hpp"/d' \
		-e '/#include "button.hpp"/d' \
		-e '/#include "checkbox.hpp"/d' \
		-e '/#include "slider.hpp"/d' \
		-e '/#include "text_input.hpp"/d' \
		-e '/#include "canvas.hpp"/d' \
		-e '/#include "cursor.hpp"/d' \
		-e '/#include "animated_image.hpp"/d' \
		-e '/#include "radio_button.hpp"/d' \
		-e '/#include "radio_group.hpp"/d' \
		-e '/#include "tab_control.hpp"/d' \
		-e '/#include "text_area.hpp"/d' \
		-e '/#include "combobox.hpp"/d' \
		-e '/#include "context_menu.hpp"/d' \
		-e '/#include "gui_manager.hpp"/d' \
		-e '/#include "sdl_app.hpp"/d' \
		-e '/^#pragma once/d' \
		-e '/^#include <.*>/d' \
		-e '/^#include "SDL2\/.*"/d' \
		-e '/#include "..\/lib\/tinyxml2.h"/d' \
		| grep -v '^\s*$$' >> $@
# Biblioteka statyczna
$(DIST_DIR)/libsdl_gui.a: $(LIB_OBJECTS) | $(DIST_DIR)
	@echo "Creating static library..."
	ar rcs $@ $^

# Biblioteka współdzielona
$(DIST_DIR)/libsdl_gui.so: $(LIB_PIC_OBJECTS) | $(DIST_DIR)
	@echo "Creating shared library..."
	$(CXX) $(RELEASE_CXXFLAGS) -shared -o $@ $^ $(LDFLAGS)

# Reguły kompilacji plików obiektowych dla bibliotek
$(OUTPUT)/release/%.o: $(SRC)/%.cpp | $(OUTPUT)/release
	$(CXX) $(RELEASE_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUTPUT)/release/%.pic.o: $(SRC)/%.cpp | $(OUTPUT)/release
	$(CXX) $(RELEASE_CXXFLAGS) $(CPPFLAGS) -fPIC -c $< -o $@

# Reguły kompilacji dla tinyxml2
$(TINYXML2_OBJ): $(TINYXML2_SRC) | $(OUTPUT)/release
	$(CXX) $(RELEASE_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(TINYXML2_PIC_OBJ): $(TINYXML2_SRC) | $(OUTPUT)/release
	$(CXX) $(RELEASE_CXXFLAGS) $(CPPFLAGS) -fPIC -c $< -o $@

# --- Inne cele ---

.PHONY: clean run non_unity modules modules_debug modules_release modules_all release

run:
	@echo "No main executable to run. Run examples individually, e.g., ./output/example_button"


clean:
	rm -rf $(OUTPUT) $(DIST_DIR)
	rm -f $(TEST_EXECS) $(UNITY_SOURCE) $(UNITY_OBJECT)
# 	rm -f $(MODULE_PCMS_DEBUG) $(MODULE_PCMS_RELEASE)
	@echo "Cleanup complete!"

$(OUTPUT):
	@mkdir -p $@

$(DIST_DIR):
	@mkdir -p $@

$(OUTPUT)/release:
	@mkdir -p $@




# --- Moduły prekompilowane C++23 ---
# Osobne cele dla budowania modułów debug i release

.PHONY: modules modules_debug modules_release

# Główny cel - buduje moduły odpowiednie dla aktualnego trybu (debug/release)
modules: $(MODULE_PCMS)

# Wymuś przebudowanie wszystkich modułów (debug i release)
modules_all: modules_debug modules_release
	@echo "Wszystkie moduły zbudowane."

# Buduj moduły debug
modules_debug: $(MODULE_PCMS_DEBUG)
	@echo "Moduły debug zbudowane."

# Buduj moduły release
modules_release: $(MODULE_PCMS_RELEASE)
	@echo "Moduły release zbudowane."

# --- Moduły DEBUG (bez optymalizacji, z informacjami debugowania) ---
$(STD_PCM_DEBUG): | $(MODULE_CACHE_DIR)
	@echo "Kompilowanie modułu std (debug)..."
	$(CXX) -std=c++23 -stdlib=libc++ -Wno-reserved-identifier -Wno-reserved-module-identifier -g -O0 --precompile -o $@ /usr/lib/llvm-22/share/libc++/v1/std.cppm

$(STD_COMPAT_PCM_DEBUG): $(STD_PCM_DEBUG) | $(MODULE_CACHE_DIR)
	@echo "Kompilowanie modułu std.compat (debug)..."
	$(CXX) -std=c++23 -stdlib=libc++ -fmodule-file=std=$(STD_PCM_DEBUG) -Wno-reserved-identifier -Wno-reserved-module-identifier -g -O0 --precompile -o $@ /usr/lib/llvm-22/share/libc++/v1/std.compat.cppm

# --- Moduły RELEASE (z pełną optymalizacją: -O3, -march=native, -flto) ---
$(STD_PCM_RELEASE): | $(MODULE_CACHE_DIR)
	@echo "Kompilowanie modułu std (release)..."
	$(CXX) -std=c++23 -stdlib=libc++ -Wno-reserved-identifier -Wno-reserved-module-identifier -O3 -march=native -flto --precompile -o $@ /usr/lib/llvm-22/share/libc++/v1/std.cppm

$(STD_COMPAT_PCM_RELEASE): $(STD_PCM_RELEASE) | $(MODULE_CACHE_DIR)
	@echo "Kompilowanie modułu std.compat (release)..."
	$(CXX) -std=c++23 -stdlib=libc++ -fmodule-file=std=$(STD_PCM_RELEASE) -Wno-reserved-identifier -Wno-reserved-module-identifier -O3 -march=native -flto --precompile -o $@ /usr/lib/llvm-22/share/libc++/v1/std.compat.cppm

$(MODULE_CACHE_DIR):
	@mkdir -p $@
