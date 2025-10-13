# define the C compiler to use
# CC = gcc
CC = clang-22
# define the Cpp compiler to use
# CXX    = g++
CXX    = clang++-22
# Flagi wspólne dla obu trybów
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
COMMON_FLAGS += $(shell sdl2-config --cflags)
COMMON_FLAGS += -stdlib=libc++
COMMON_FLAGS += -std=c++23

# Flagi specyficzne dla trybu Release
RELEASE_FLAGS = -O3
RELEASE_FLAGS += -march=native
RELEASE_FLAGS += -flto
RELEASE_FLAGS += -DNDEBUG

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
CPPFLAGS := -Isrc -Ilib

# define library paths in addition to /usr/lib
LDFLAGS  = $(shell sdl2-config --libs)
LDFLAGS += -lSDL2_image
LDFLAGS += -lSDL2_ttf

# define output directory
OUTPUT := output

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
TEST_EXECS      := $(patsubst $(TESTS_DIR)/test_%.cpp,$(OUTPUT)/test_%,$(filter-out $(TESTS_DIR)/test_helper.cpp,$(TEST_SRC_FILES)))
# --- Konfiguracja przykładów ---
EXAMPLE_SRC_FILES := $(wildcard examples/*.cpp)
EXAMPLE_EXECS     := $(patsubst examples/%.cpp,$(OUTPUT)/%,$(EXAMPLE_SRC_FILES))

# --- Główne cele ---

examples: $(UNITY_OBJECT) $(EXAMPLE_EXECS)
all:  examples

# Cel do tradycyjnej kompilacji (non-unity)
# Buduje wszystkie pliki obiektowe, ale nie linkuje ich.
# To wystarczy, aby `bear` przechwycił komendy kompilacji.
non_unity: $(NON_UNITY_OBJECTS)
	@echo "Tradycyjna kompilacja (non-unity) zakończona. Obiekty znajdują się w $(OUTPUT)/"

# Cel do uruchamiania testów
test: $(TEST_UNITY_OBJECT) $(TEST_EXECS)
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

# --- Inne cele ---

.PHONY: clean run non_unity modules

run:
	@echo "No main executable to run. Run examples individually, e.g., ./output/example_button"


clean:
	rm -rf $(OUTPUT)
	rm -f $(TEST_EXECS) $(UNITY_SOURCE) $(UNITY_OBJECT)
	@echo "Cleanup complete!"

$(OUTPUT):
	@mkdir -p $@