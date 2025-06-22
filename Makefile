# define the C compiler to use
# CC = gcc
CC = clang
# define the Cpp compiler to use
# CXX    = g++
CXX    = clang++
FLAGS  = -Wall
FLAGS += -Wextra
FLAGS += -Og
FLAGS += $(shell sdl2-config --cflags)
FLAGS += -march=native
FLAGS += -g
#FLAGS += -flto
#FLAGS += -fmodules-ts
#define any compile-time flags for C
CFLAGS := $(FLAGS)

# define any compile-time flags for C++
CXXFLAGS  := -std=c++20
#CXXFLAGS += -Wnon-virtual-dtor
CXXFLAGS  += $(FLAGS)
CXXFLAGS  += -stdlib=libc++

# define C Preprocessor flags
CPPFLAGS := -Isrc

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

 # --- Źródła i obiekty ---
 # Wszystkie pliki źródłowe C++ w projekcie
 CPPSOURCES := $(wildcard $(SRC)/*.cpp)
 # Pliki obiektowe biblioteki
 LIB_OBJ_FILES := $(patsubst $(SRC)/%.cpp,$(OUTPUT)/%.o,$(CPPSOURCES))

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

all: examples

examples: $(EXAMPLE_EXECS)

# Cel do uruchamiania testów
test: $(LIB_OBJ_FILES) $(TEST_EXECS)
	@echo "Running all tests..."
	@for t in $(TEST_EXECS); do \
		./$$t || exit 1; \
	done
	@echo "All tests passed successfully."

# --- Reguły kompilacji ---

# Pliki wykonywalne testów
# Każdy test jest linkowany z całą biblioteką i pomocnikiem testów
$(OUTPUT)/test_%: $(TESTS_DIR)/test_%.cpp $(LIB_OBJ_FILES) $(TEST_HELPER_OBJ) $(OUTPUT)/catch_amalgamated.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Pliki wykonywalne przykładów
$(OUTPUT)/example_%: examples/example_%.cpp $(LIB_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Kompilacja plików obiektowych (zarówno z src, jak i tests)
$(OUTPUT)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@


$(TEST_HELPER_OBJ): $(TEST_HELPER_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUTPUT)/catch_amalgamated.o: $(LIB)/catch_amalgamated.cpp $(LIB)/catch_amalgamated.hpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(LIB) -c $< -o $@

# --- Inne cele ---

.PHONY: clean run debug

run:
	@echo "No main executable to run. Run examples individually, e.g., ./output/example_button"

debug:
	@echo "No main executable to debug. Debug examples individually."

clean:
	rm -rf $(OUTPUT)
	rm -f $(TEST_EXECS)
	@echo "Cleanup complete!"
