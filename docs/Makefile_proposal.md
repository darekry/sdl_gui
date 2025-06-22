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

MAIN_EXEC := $(OUTPUT)/main

# --- Źródła i obiekty ---
# Wszystkie pliki źródłowe C++ w projekcie
CPPSOURCES := $(wildcard $(SRC)/*.cpp)
# Pliki obiektowe biblioteki (bez main.o)
LIB_OBJ_FILES := $(patsubst $(SRC)/%.cpp,$(OUTPUT)/%.o,$(filter-out $(SRC)/main.cpp,$(CPPSOURCES)))
# Plik obiektowy main
MAIN_OBJ := $(patsubst $(SRC)/main.cpp,$(OUTPUT)/main.o,$(filter $(SRC)/main.cpp,$(CPPSOURCES)))

# --- Konfiguracja testów ---
# Pliki źródłowe testów
TEST_SRC_FILES  := $(wildcard $(TESTS_DIR)/test_*.cpp)
# Plik pomocniczy testów
TEST_HELPER_SRC := $(TESTS_DIR)/test_helper.cpp
TEST_HELPER_OBJ := $(patsubst $(TESTS_DIR)/%.cpp,$(OUTPUT)/%.o,$(TEST_HELPER_SRC))
# Pliki wykonywalne testów
TEST_EXECS      := $(patsubst $(TESTS_DIR)/test_%.cpp,$(OUTPUT)/test_%,$(TEST_SRC_FILES))

# --- Główne cele ---

all: $(MAIN_EXEC)

# Cel do uruchamiania testów
test: $(TEST_EXECS)
	@echo "Running all tests..."
	@for t in $(TEST_EXECS); do \
		./$$t || exit 1; \
	done
	@echo "All tests passed successfully."

# --- Reguły kompilacji ---

# Główny plik wykonywalny
$(MAIN_EXEC): $(MAIN_OBJ) $(LIB_OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Pliki wykonywalne testów
# Każdy test jest linkowany z całą biblioteką i pomocnikiem testów
$(OUTPUT)/test_%: $(TESTS_DIR)/test_%.cpp $(LIB_OBJ_FILES) $(TEST_HELPER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Kompilacja plików obiektowych (zarówno z src, jak i tests)
$(OUTPUT)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC) -c $< -o $@

$(OUTPUT)/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(SRC) -c $< -o $@

# --- Inne cele ---

.PHONY: clean run debug

run: all
	./$(MAIN_EXEC)

debug: CFLAGS += -DDEBUG
debug: all
	gdb $(MAIN_EXEC)

clean:
	rm -rf $(OUTPUT)
	@echo "Cleanup complete!"