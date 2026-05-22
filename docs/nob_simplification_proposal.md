# Propozycja uproszczenia nob.c

## Aktualny stan

`nob.c` ma ~1400 linii kodu i zawiera dużo duplikacji:

1. **Pętle for** - w każdym miejscu dodawania flag:
   ```c
   for (size_t i = 0; i < NOB_ARRAY_LEN(common_flags); i++) {
       nob_cmd_append(&cmd, common_flags[i]);
   }
   ```
   
2. **Osobne build functions** - `build_examples()`, `build_tests()`, `build_release()` mają podobny pattern

3. **Release artifacts** - buduje `.a` AND `.so` (2x więcej kompilacji)

4. **Debug vs Release** - dwa kompletnie różne zestawy flag i modułów

## Problem: Struktura nob.h

`nob.h` nie pozwala łatwo dodać tablicy stringów do `Nob_Cmd` przez variadic macro:
- `nob_cmd_append` jest variadic (kończy na `(const char*)-1`)
- `nob_cmd_extend` kopiuje z innej `Nob_Cmd` (nie z static array)

### Rozwiązanie: Helper function

```c
// Nowa helper function w nob.c
static void cmd_append_flags(Nob_Cmd *cmd, const char **flags, size_t count) {
    nob_da_append_many(cmd, flags, count);
}

// Użycie (zamiast pętli for):
cmd_append_flags(&cmd, common_flags, NOB_ARRAY_LEN(common_flags));
```

**ALE**: `nob.h` już ma to! `nob_cmd_extend` używa `nob_da_append_many`:
```c
#define nob_cmd_extend(cmd, other_cmd) \
    nob_da_append_many(cmd, (other_cmd)->items, (other_cmd)->count)
```

Możemy więc stworzyć `Nob_Cmd` z flags i użyć `nob_cmd_extend`:

```c
// Prekompilowane komendy z flagami
static Nob_Cmd make_common_cmd(void) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX);
    nob_da_append_many(&cmd, common_flags, NOB_ARRAY_LEN(common_flags));
    return cmd;
}

static Nob_Cmd make_debug_cmd(void) {
    Nob_Cmd cmd = {0};
    nob_da_append_many(&cmd, debug_flags, NOB_ARRAY_LEN(debug_flags));
    return cmd;
}

static Nob_Cmd make_release_cmd(void) {
    Nob_Cmd cmd = {0};
    nob_da_append_many(&cmd, release_flags, NOB_ARRAY_LEN(release_flags));
    return cmd;
}
```

## Propozycja architektury

### Opcja A: Minimalista (tylko unity build)

**Cele**:
- `./nob` - unity build examples (debug)
- `./nob test` - unity build + run tests
- `./nob clean` - cleanup

**Eliminujemy**:
- Release mode (używamy tylko debug)
- `.so` library (tylko `.a` jeśli potrzebne)
- `non_unity` (używamy unity build)
- Release-specific modules (debug pcm dla wszystkich)

**Korzyści**:
- ~50% mniej kodu
- Prostsze API
- Mniej build time (tylko jeden zestaw modułów)

### Opcja B: Uproszczona z release (zachowana)

**Cele**:
- `./nob` - build examples
- `./nob test` - build + run tests  
- `./nob release` - build `.a` only (nie `.so`)
- `./nob clean` - cleanup

**Eliminujemy**:
- `.so` library (double compilation)
- `non_unity` (unity jest default)
- `compile_commands` (może być przez `./nob compile_commands` - optional)

### Opcja C: Zachowanie funkcjonalności + refaktor

Zachujemy wszystkie obecne features, ale refactor kodu:

**1. Użyj `nob_cmd_extend`**:

```c
// Prekompilowane flagi jako Nob_Cmd
static Nob_Cmd cmd_common = {0};
static Nob_Cmd cmd_debug = {0};
static Nob_Cmd cmd_release = {0};
static bool cmds_initialized = false;

static void init_cmds(void) {
    if (cmds_initialized) return;
    
    nob_da_append_many(&cmd_common, common_flags, NOB_ARRAY_LEN(common_flags));
    nob_da_append_many(&cmd_debug, debug_flags, NOB_ARRAY_LEN(debug_flags));
    nob_da_append_many(&cmd_release, release_flags, NOB_ARRAY_LEN(release_flags));
    
    cmds_initialized = true;
}

// Użycie:
Nob_Cmd cmd = {0};
nob_cmd_append(&cmd, CXX);
nob_cmd_extend(&cmd, &cmd_common);
nob_cmd_extend(&cmd, &cmd_debug);  // lub cmd_release
```

**2. Helper function dla compilacji**:

```c
typedef struct {
    const char *src;
    const char *output;
    bool pic;  // Position Independent Code for .so
} Compile_Target;

static bool compile_parallel(Compile_Target *targets, size_t count, 
                             bool release, Nob_Procs *procs);
```

**3. Eliminate release double-compilation**:

Obecnie release kompiluje:
- `output/release/foo.o` (for `.a`)
- `output/release/foo.pic.o` (for `.so`)

**Rozwiązanie**: Kompiluj tylko `.pic.o`, użyj dla `.a` i `.so`:
```c
// .pic.o works for both .a and .so
ar rcs libsdl_gui.a output/release/*.pic.o  // OK!
```

### Opcja D: Dwa skrypty

Podzielić na dwa pliki:
- `nob.c` - główny build (simple, minimal)
- `nob_release.c` - release artifacts (optional, rarely used)

```c
// nob.c - minimalista
int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        return build_and_run_tests();
    }
    if (argc > 1 && strcmp(argv[1], "clean") == 0) {
        return clean();
    }
    return build_examples();
}

// nob_release.c - dla release builds (rzadko używany)
int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    return build_release_artifacts();
}
```

## Analiza potrzeb projektu

**Pytania**:
1. Czy release artifacts są naprawdę potrzebne?
   - Jeśli projekt jest tylko dla development/examples: **NIE**
   - Jeśli projekt jest biblioteką dla innych: **TAK**

2. Czy `.so` (shared library) jest potrzebne?
   - `.a` (static) jest simpler i sufficient dla większości use cases
   - `.so` wymaga `-fPIC` i extra compilation

3. Czy `compile_commands.json` jest potrzebne?
   - Dla clangd/LSP: **TAK**, ale może być generated on-demand
   - Nie musi być w głównym build流程

4. Czy `non_unity` jest potrzebne?
   - Dla unity build: **NIE** (unity jest default)
   - Dla compile_commands.json generation: może być useful

## Rekomendacja

**Opcja B + Opcja C refaktor**:

1. Zachowaj release dla `.a` (eliminate `.so`)
2. Zachowaj `compile_commands` jako optional target
3. Refaktor kodu używając `nob_cmd_extend` pattern
4. Kompiluj tylko jeden zestaw object files dla release

**Przykład kodu po refaktor**:

```c
// Global prekompilowane komendy
static Nob_Cmd g_common = {0};
static Nob_Cmd g_debug = {0};
static Nob_Cmd g_release = {0};

static void init_globals(void) {
    nob_da_append_many(&g_common, common_flags, NOB_ARRAY_LEN(common_flags));
    nob_da_append_many(&g_debug, debug_flags, NOB_ARRAY_LEN(debug_flags));
    nob_da_append_many(&g_release, release_flags, NOB_ARRAY_LEN(release_flags));
}

// Simplified build function
static bool build_exe(const char *src, const char *exe, bool release) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX);
    nob_cmd_extend(&cmd, &g_common);
    nob_cmd_extend(&cmd, release ? &g_release : &g_debug);
    
    // Add module flags, source, output, libs
    nob_cmd_append(&cmd, 
        module_file_std(release), module_file_compat(release),
        "-o", exe, src, unity_object(release));
    get_sdl2_libs(&cmd);
    
    return nob_cmd_run(&cmd);
}
```

**Estimated reduction**: ~50-60% kodu (z ~1400 do ~600-700 linii)

## Implementation Steps

1. **Add `init_globals()`** - prekompiluj flagi jako `Nob_Cmd`
2. **Replace for loops** - użyj `nob_cmd_extend`
3. **Eliminate `.so`** - build tylko `.a` with `.pic.o` or regular `.o`
4. **Consolidate build functions** - single `build_exe()` helper
5. **Optional: Split nob.c/nob_release.c** - jeśli release jest rarely used

## Appendix: nob.h useful macros

```c
// nob.h provides these useful patterns:

// Extend command from another command
nob_cmd_extend(&cmd, &other_cmd);  // Uses nob_da_append_many internally

// Append many items to dynamic array
nob_da_append_many(&arr, items, count);

// Foreach over dynamic array
nob_da_foreach(Type, item, &arr) {
    // item is pointer to current element
}

// String builder shortcuts
nob_sb_append_cstr(&sb, "text");  // Append C string
nob_sb_append_null(&sb);          // Null terminator
```