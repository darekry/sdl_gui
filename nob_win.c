// nob_win.c -- Windows-native build tool for sdl_gui (llvm-mingw toolchain).
//
// nob.c is Linux-only (hardcoded clang++-24, /usr/.../std.cppm, -stdlib=libc++,
// pkg-config via popen/sh, rm/cp shell calls, .so). This file mirrors its
// targets (examples, test, bench, release, non_unity, clean) using only
// llvm-mingw (clang + mingw sysroot + libc++ in one zip, no VS needed):
//
//   1. powershell -ExecutionPolicy Bypass -File tools/install_win_deps.ps1
//      downloads llvm-mingw (-> vendor/llvm-mingw/) and SDL3 mingw devel
//      packages (-> vendor/win64/). Headers/import-libs/DLLs are
//      auto-detected, no pkg-config needed.
//   2. Build this tool with the toolchain itself:
//        .\vendor\llvm-mingw\bin\x86_64-w64-mingw32-clang.exe -o nob_win.exe nob_win.c
//      (no NOB_GO_REBUILD_URSELF on purpose: it shells out to `cc`, which
//      does not exist on Windows; rebuild explicitly as above.)
//   3. .\nob_win.exe test   (or: examples, bench, release, non_unity, clean)
//
// Toolchain lookup order: %LLVM_MINGW% env (dir with bin/ inside) ->
// vendor/llvm-mingw/ -> x86_64-w64-mingw32-clang++ on PATH.
// nob.h process/dir/file APIs already have _WIN32 branches.
//
// Usage: nob_win [examples|test [filter]|bench|release|non_unity|clean] [-r]
//
// Deliberate differences from nob.c:
//   - no C++ modules: src/std.hpp falls back to classic includes on _WIN32,
//     so there is no build_modules() and no -fmodule-file flags at all.
//   - no sanitizers in debug flags yet (plain -g -O0). llvm-mingw ships
//     both runtimes, so -fsanitize=address,undefined is a one-line change
//     once the first green build lands (ASan needs its DLL beside the exes).
//   - no -march=native / -flto in release flags (keep -O3 -DNDEBUG).
//   - SDL flags come from vendor/win64/ probing (mingw layout), not pkg-config:
//     -I<x>/x86_64-w64-mingw32/include + -L.../lib, links as -lSDL3... plus
//     -lwinmm -limm32 -lversion -lsetupapi.
//   - outputs: .obj / .exe / dist/sdl_gui_static.lib (static) +
//     dist/sdl_gui.dll with import dist/sdl_gui.lib (shared).
//   - tests run without the `env` wrapper (does not exist on Windows):
//     SDL_GUI_HIDDEN=1 is set via SetEnvironmentVariableA and inherited.
//   - NEED_MIXER / NEED_SQLITE are skips (like NEED_EMBEDDED/SHADERS), not
//     fatal link errors: on a fresh machine those examples report what to
//     drop into vendor/win64/ instead of breaking the build.
//   - shell `rm -rf` / `cp -r` / `mkdir -p` replaced by rm_rf_win() and
//     nob_copy_file / nob_copy_directory_recursively.
//   - generated-file banners stay byte-identical to nob.c ("Auto-generated
//     by nob.c ...") so switching between nob and nob_win does not rewrite
//     src/embedded_assets.hpp or output/*_data.c.

#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define strdup _strdup
#endif

#define SRC_DIR "src"
#define OUTPUT_DIR "output"
#define DIST_DIR "dist"
#define TESTS_DIR "tests"
#define BENCH_DIR "bench"
#define LIB_DIR "lib"
#define EXAMPLES_DIR "examples"
#define VENDOR_DIR "vendor/win64"

#define MINGW_TRIPLE "x86_64-w64-mingw32"

// Forward declarations (defined further below).
static bool rm_rf_win(const char *path);

// Compiler/linker programs. Resolved by discover_toolchain() to either
// full paths inside vendor/llvm-mingw (or %LLVM_MINGW%) or bare triple-
// prefixed names expected on PATH. The .exe suffix works for both
// CreateProcess (exact file) and PATH lookup (Windows appends .exe anyway).
static const char *CXX = NULL;     // x86_64-w64-mingw32-clang++.exe
static const char *CC = NULL;      // x86_64-w64-mingw32-clang.exe
static const char *LLVM_AR = NULL; // llvm-ar.exe
static const char *g_tc_dir = NULL;
static bool g_have_toolchain = false;

static void discover_toolchain(void) {
    if (CXX != NULL) return; // already resolved
    const char *env = getenv("LLVM_MINGW");
    const char *candidates[] = { env, VENDOR_DIR "/../llvm-mingw" };
    for (size_t i = 0; i < NOB_ARRAY_LEN(candidates); i++) {
        const char *d = candidates[i];
        if (d == NULL || d[0] == '\0') continue;
        const char *probe = nob_temp_sprintf("%s/bin/" MINGW_TRIPLE "-clang++.exe", d);
        if (!nob_file_exists(probe)) continue;
        g_tc_dir = strdup(d);
        CXX = strdup(nob_temp_sprintf("%s/bin/" MINGW_TRIPLE "-clang++.exe", d));
        CC = strdup(nob_temp_sprintf("%s/bin/" MINGW_TRIPLE "-clang.exe", d));
        LLVM_AR = strdup(nob_temp_sprintf("%s/bin/llvm-ar.exe", d));
        break;
    }
    if (CXX == NULL) {
        // No vendored toolchain: expect the triple-prefixed tools on PATH
        // (e.g. llvm-mingw/bin added to PATH, or an MSYS2 mingw shell).
        CXX = MINGW_TRIPLE "-clang++.exe";
        CC = MINGW_TRIPLE "-clang.exe";
        LLVM_AR = "llvm-ar.exe";
    }

    // Fail fast with a readable message instead of per-command noise.
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX, "--version");
    if (!nob_cmd_run(&cmd)) {
        nob_log(NOB_ERROR, "C++ compiler not runnable: %s", CXX);
        nob_log(NOB_ERROR, "Run: powershell -ExecutionPolicy Bypass -File tools/install_win_deps.ps1");
        nob_log(NOB_ERROR, "or set LLVM_MINGW to a dir with bin/" MINGW_TRIPLE "-clang++.exe,");
        nob_log(NOB_ERROR, "or put llvm-mingw/bin on PATH.");
        return;
    }
    g_have_toolchain = true;
    if (g_tc_dir) nob_log(INFO, "Toolchain: %s", g_tc_dir);
}

// ========== PRECOMPILED FLAGS ==========

static const char * common_flags[] = {
    "-Wall",
    "-Wextra",
    "-Wshadow",
    "-Wconversion",
    "-Wsign-conversion",
    "-Wfloat-equal",
    "-Wformat=2",
    "-Wnon-virtual-dtor",
    "-Woverloaded-virtual",
    "-Wreorder",
    "-Wzero-as-null-pointer-constant",
    "-Wunreachable-code",
    "-Wstrict-aliasing",
    "-Wpedantic",
    "-Wcast-align",
    "-Wcast-qual",
    "-Wctor-dtor-privacy",
    "-Wdisabled-optimization",
    "-Winit-self",
    "-Wmissing-declarations",
    "-Winline",
    "-Wdouble-promotion",
    "-Wnull-dereference",
    "-Wextra-semi",
    "-Wsign-promo",
    "-std=c++23",
    "-Isrc",
    "-Ilib",
};

static const char * debug_flags[] = {
    "-g",
    "-O0",
    "-DDEBUG=1",
};

static const char * release_flags[] = {
    "-O3",
    "-DNDEBUG",
};

// Global precompiled commands (initialized once)
static Nob_Cmd g_common = {0};
static Nob_Cmd g_debug = {0};
static Nob_Cmd g_release = {0};
static Nob_Cmd g_sdl_includes = {0}; // -I<vendor/.../include> (probed)
static Nob_Cmd g_sdl_ldirs = {0};     // -L<vendor/.../lib> (probed)
static Nob_File_Paths g_runtime_dlls = {0}; // runtime DLLs copied next to exes (SDL + toolchain libc++)
static bool g_have_sdl = false;
static bool g_have_mixer = false;
static bool g_have_sqlite = false;
static bool g_vendor_probed = false;
static Nob_Compdb g_compdb = {0};    // compile_commands.json entries
static bool g_initialized = false;

// ========== VENDOR PROBING (replaces pkg-config) ==========

// tools/install_win_deps.ps1 extracts each *-devel-*-mingw.zip into its own
// vendor/win64/<name>/ dir with x86_64-w64-mingw32/{include,lib,bin} inside,
// and runtime DLL zips into vendor/win64/bin/<prefix>/. Anything matching
// that layout is picked up; versions never appear in this file.

static bool dll_seen(const char *base) {
    nob_da_foreach(const char*, p, &g_runtime_dlls) {
        if (strcmp(nob_path_name(*p), base) == 0) return true;
    }
    return false;
}

// Collect *.dll directly inside dir (lib/x64 or a bin/ dir).
static void probe_dll_dir(const char *dir) {
    Nob_Dir_Entry e = {0};
    if (!nob_dir_entry_open(dir, &e)) return;
    while (nob_dir_entry_next(&e)) {
        if (strcmp(e.name, ".") == 0 || strcmp(e.name, "..") == 0) continue;
        if (strstr(e.name, ".dll") == NULL) continue;
        if (dll_seen(e.name)) continue;
        nob_da_append(&g_runtime_dlls, strdup(nob_temp_sprintf("%s/%s", dir, e.name)));
    }
    nob_dir_entry_close(e);
}

static bool lib_present(const char *libdir, const char *file) {
    return nob_file_exists(nob_temp_sprintf("%s/%s", libdir, file));
}

static void probe_vendor(void) {
    if (g_vendor_probed) return;
    g_vendor_probed = true;

    if (!nob_file_exists(VENDOR_DIR)) {
        nob_log(NOB_ERROR, "Missing %s/ -- run: powershell -ExecutionPolicy Bypass -File tools/install_win_deps.ps1", VENDOR_DIR);
        return;
    }

    bool sdl = false, image = false, ttf = false;

    Nob_Dir_Entry e = {0};
    if (!nob_dir_entry_open(VENDOR_DIR, &e)) return;
    while (nob_dir_entry_next(&e)) {
        if (strcmp(e.name, ".") == 0 || strcmp(e.name, "..") == 0) continue;
        if (strcmp(e.name, "download") == 0 || strcmp(e.name, "bin") == 0) continue;
        const char *sub = nob_temp_sprintf("%s/%s", VENDOR_DIR, e.name);
        // MinGW devel layout (verified: SDL3-devel-3.4.18-mingw.zip):
        //   <pkg>-<ver>/x86_64-w64-mingw32/{include,lib,bin}
        //   lib holds libSDL3.dll.a (import lib), bin holds SDL3.dll.
        const char *base = nob_temp_sprintf("%s/%s/" MINGW_TRIPLE, VENDOR_DIR, e.name);
        const char *inc = nob_temp_sprintf("%s/include", base);
        const char *libdir = nob_temp_sprintf("%s/lib", base);
        const char *bindir = nob_temp_sprintf("%s/bin", base);
        if (!nob_file_exists(inc) || !nob_file_exists(libdir)) continue;

        nob_log(INFO, "Using SDL vendor package: %s", sub);
        nob_cmd_append(&g_sdl_includes, strdup(nob_temp_sprintf("-I%s", inc)));
        nob_cmd_append(&g_sdl_ldirs, strdup(nob_temp_sprintf("-L%s", libdir)));

        if (lib_present(libdir, "libSDL3.dll.a")) sdl = true;
        if (lib_present(libdir, "libSDL3_image.dll.a")) image = true;
        if (lib_present(libdir, "libSDL3_ttf.dll.a")) ttf = true;
        // Optional: picked up when present, examples otherwise skipped.
        if (lib_present(libdir, "libSDL3_mixer.dll.a")) g_have_mixer = true;
        if (lib_present(libdir, "libsqlite3.a") || lib_present(libdir, "libsqlite3.dll.a")) g_have_sqlite = true;

        if (nob_file_exists(bindir)) probe_dll_dir(bindir);
    }
    nob_dir_entry_close(e);

    // Runtime DLL zips (vendor/win64/bin/<prefix>/*.dll) as a fallback.
    const char *bindir = VENDOR_DIR "/bin";
    if (nob_file_exists(bindir)) {
        Nob_Dir_Entry b = {0};
        if (nob_dir_entry_open(bindir, &b)) {
            while (nob_dir_entry_next(&b)) {
                if (strcmp(b.name, ".") == 0 || strcmp(b.name, "..") == 0) continue;
                probe_dll_dir(nob_temp_sprintf("%s/%s", bindir, b.name));
            }
            nob_dir_entry_close(b);
        }
    }

    g_have_sdl = sdl && image && ttf;
    if (!g_have_sdl) {
        nob_log(NOB_ERROR, "Incomplete SDL vendor packages in %s/ (have SDL3=%d image=%d ttf=%d) -- re-run tools/install_win_deps.ps1",
                VENDOR_DIR, (int)sdl, (int)image, (int)ttf);
    }
}

// llvm-mingw links libc++/libunwind shared, so every .exe needs them beside
// it (without them Windows kills the process at startup with 0xC0000139).
// Only the vendored toolchain needs copying; a PATH toolchain's bin/ is
// already visible to the loader.
static void collect_toolchain_dlls(void) {
    if (g_tc_dir == NULL) return;
    static const char *names[] = { "libc++.dll", "libunwind.dll" };
    for (size_t i = 0; i < NOB_ARRAY_LEN(names); i++) {
        const char *full = nob_temp_sprintf("%s/" MINGW_TRIPLE "/bin/%s", g_tc_dir, names[i]);
        if (!nob_file_exists(full)) continue;
        if (dll_seen(names[i])) continue;
        nob_log(INFO, "Using toolchain runtime DLL: %s", full);
        nob_da_append(&g_runtime_dlls, strdup(full));
    }
}

static void init_globals(void) {
    if (g_initialized) return;
    nob_da_append_many(&g_common, common_flags, NOB_ARRAY_LEN(common_flags));
    nob_da_append_many(&g_debug, debug_flags, NOB_ARRAY_LEN(debug_flags));
    nob_da_append_many(&g_release, release_flags, NOB_ARRAY_LEN(release_flags));
    discover_toolchain();
    probe_vendor();
    collect_toolchain_dlls();
    g_initialized = true;
}

// ========== HELPER FUNCTIONS ==========

static void cmd_add_common(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_common);
}

static void cmd_add_mode(Nob_Cmd *cmd, bool release) {
    nob_cmd_extend(cmd, release ? &g_release : &g_debug);
}

// -I flags only (for -c compiles).
static void cmd_add_sdl3_includes(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_sdl_includes);
}

// -I + -L flags + -l libs (for links). The -l names resolve against the
// probed -L dirs (libSDL3.dll.a etc.); SDL_mixer/sqlite only when present.
static void cmd_add_sdl3(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_sdl_includes);
    nob_cmd_extend(cmd, &g_sdl_ldirs);
    nob_cmd_append(cmd, "-lSDL3", "-lSDL3_image", "-lSDL3_ttf");
    if (g_have_mixer) nob_cmd_append(cmd, "-lSDL3_mixer");
    if (g_have_sqlite) nob_cmd_append(cmd, "-lsqlite3");
    // System libs SDL backends need on Windows.
    nob_cmd_append(cmd, "-lwinmm", "-limm32", "-lversion", "-lsetupapi");
}

// ========== FILE COLLECTION (same as nob.c) ==========

static bool collect_files(Nob_File_Paths *paths, const char *dir, const char *ext) {
    Nob_Dir_Entry entry = {0};
    if (!nob_dir_entry_open(dir, &entry)) return false;
    while (nob_dir_entry_next(&entry)) {
        if (ext == NULL || strstr(entry.name, ext) != NULL) {
            nob_da_append(paths, nob_temp_sprintf("%s/%s", dir, entry.name));
        }
    }
    nob_dir_entry_close(entry);
    return !entry.error;
}

static bool collect_cpp_sources(Nob_File_Paths *paths) {
    return collect_files(paths, SRC_DIR, ".cpp");
}

static bool collect_composite_sources(Nob_File_Paths *paths) {
    return collect_files(paths, SRC_DIR "/composite", ".cpp");
}

static bool collect_editor_sources(Nob_File_Paths *paths) {
    return collect_files(paths, SRC_DIR "/editor", ".cpp");
}

static bool collect_hpp_sources(Nob_File_Paths *paths) {
    return collect_files(paths, SRC_DIR, ".hpp");
}

static bool collect_example_sources(Nob_File_Paths *paths) {
    Nob_Dir_Entry entry = {0};
    if (!nob_dir_entry_open(EXAMPLES_DIR, &entry)) return false;
    while (nob_dir_entry_next(&entry)) {
        if (strstr(entry.name, ".cpp") != NULL
            && strcmp(entry.name, "47_standalone.cpp") != 0) {
            nob_da_append(paths, nob_temp_sprintf("%s/%s", EXAMPLES_DIR, entry.name));
        }
    }
    nob_dir_entry_close(entry);
    return !entry.error;
}

static bool collect_test_sources(Nob_File_Paths *paths) {
    Nob_Dir_Entry entry = {0};
    if (!nob_dir_entry_open(TESTS_DIR, &entry)) return false;
    while (nob_dir_entry_next(&entry)) {
        if (strncmp(entry.name, "test_", 5) == 0 && strstr(entry.name, ".cpp") != NULL) {
            if (strcmp(entry.name, "test_helper.cpp") != 0 && strcmp(entry.name, "test_main.cpp") != 0) {
                nob_da_append(paths, nob_temp_sprintf("%s/%s", TESTS_DIR, entry.name));
            }
        }
    }
    nob_dir_entry_close(entry);
    return !entry.error;
}

// ========== UNITY BUILD (same as nob.c) ==========

static bool generate_unity_content(Nob_String_Builder *sb) {
    Nob_File_Paths sources = {0};
    Nob_File_Paths composite = {0};
    Nob_File_Paths editor = {0};

    if (!collect_cpp_sources(&sources)) return false;
    if (!collect_composite_sources(&composite)) return false;
    if (!collect_editor_sources(&editor)) return false;

    nob_sb_append_cstr(sb, "// Generated by nob.c for unity build\n\n");

    nob_da_foreach(const char*, src, &sources) {
        nob_sb_appendf(sb, "#include \"%s\"\n", nob_path_name(*src));
    }
    nob_sb_append_cstr(sb, "#include \"tinyxml2.cpp\"\n");

    nob_da_foreach(const char*, src, &composite) {
        nob_sb_appendf(sb, "#include \"composite/%s\"\n", nob_path_name(*src));
    }

    nob_da_foreach(const char*, src, &editor) {
        nob_sb_appendf(sb, "#include \"editor/%s\"\n", nob_path_name(*src));
    }
    nob_sb_append_null(sb);
    return true;
}

// Generate unity source only if content changed (preserves timestamp)
static bool ensure_unity_source(void) {
    Nob_String_Builder new_content = {0};
    if (!generate_unity_content(&new_content)) {
        nob_sb_free(new_content);
        return false;
    }

    const char *path = OUTPUT_DIR "/all.cpp";

    // Check if file exists and content matches
    if (nob_file_exists(path)) {
        Nob_String_Builder existing = {0};
        if (nob_read_entire_file(path, &existing)) {
            nob_sb_append_null(&existing);
            if (strcmp(new_content.items, existing.items) == 0) {
                // Content unchanged - skip write to preserve timestamp
                nob_sb_free(existing);
                nob_sb_free(new_content);
                return true;
            }
            nob_sb_free(existing);
        }
    }

    // Content changed or file missing - write new content
    nob_log(INFO, "Regenerating unity source: %s", path);
    if (!nob_write_entire_file(path, new_content.items, new_content.count - 1)) {
        nob_sb_free(new_content);
        return false;
    }
    nob_sb_free(new_content);
    return true;
}

// ========== UNITY OBJECT (no modules on Windows) ==========

static bool build_unity_object(bool release, bool for_tests) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);

    if (!ensure_unity_source()) return false;

    const char *output_obj = for_tests ? OUTPUT_DIR "/all_test.obj" : OUTPUT_DIR "/all.obj";

    // Unity object depends on all.cpp + every .cpp/.hpp it includes.
    Nob_File_Paths deps = {0};
    collect_cpp_sources(&deps);
    collect_composite_sources(&deps);
    collect_editor_sources(&deps);
    collect_hpp_sources(&deps);
    collect_files(&deps, SRC_DIR "/composite", ".hpp");
    collect_files(&deps, SRC_DIR "/editor", ".hpp");
    nob_da_append(&deps, OUTPUT_DIR "/all.cpp");

    int needs = nob_needs_rebuild(output_obj, deps.items, deps.count);
    if (needs <= 0 && nob_file_exists(output_obj)) {
        return true;
    }

    nob_log(INFO, "Building unity object: %s", output_obj);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX);
    cmd_add_common(&cmd);
    cmd_add_mode(&cmd, release);
    cmd_add_sdl3_includes(&cmd); // clang has no /usr/local/include here
    nob_cmd_append(&cmd, "-c", OUTPUT_DIR "/all.cpp", "-o", output_obj);

    // Add to compile_commands.json
    nob_compdb_add(&g_compdb, &cmd, OUTPUT_DIR "/all.cpp", .output = output_obj);

    // Also add individual source files to compile_commands.json for clangd
    Nob_Cmd individual_cmd = {0};
    nob_cmd_append(&individual_cmd, CXX);
    cmd_add_common(&individual_cmd);
    cmd_add_mode(&individual_cmd, release);
    cmd_add_sdl3_includes(&individual_cmd);
    nob_cmd_append(&individual_cmd, "-c");

    Nob_File_Paths all_sources = {0};
    collect_cpp_sources(&all_sources);
    collect_composite_sources(&all_sources);
    collect_editor_sources(&all_sources);

    nob_da_foreach(const char*, src, &all_sources) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/%s.obj", OUTPUT_DIR, name);
        nob_compdb_add(&g_compdb, &individual_cmd, *src, .output = obj);
    }

    return nob_cmd_run(&cmd);
}

// ========== EMBEDDED ASSETS (same portable generator as nob.c) ==========

// Asset definitions: path + optional font size (-1 = texture, >= 0 = font)
static Nob_Embed_Asset g_embedded_assets[] = {
    {"assets/button1.png", -1},
    {"assets/button_bg.png", -1},
    {"assets/fonts/font.ttf", 16},
};

// Availability of optional build inputs. Missing assets / missing glslc are
// NOT fatal: dependent examples are skipped (see ExampleNeeds below).
static bool g_have_embedded_assets = false;
static bool g_have_shaders = false;

// Emit raw bytes as "0x.., ..." rows (12 per line) into a C source builder.
static void sb_append_hex_bytes(Nob_String_Builder *sb, const char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (i % 12 == 0) nob_sb_append_cstr(sb, "    ");
        nob_sb_appendf(sb, "0x%02x,", (unsigned)(unsigned char)data[i]);
        if (i % 12 == 11 || i + 1 == size) nob_sb_append_cstr(sb, "\n");
        else nob_sb_append_cstr(sb, " ");
    }
}

// Portable ident: anything outside [a-zA-Z0-9] becomes '_' (same rule as
// nob_embed_path_to_ident, kept local so we don't depend on ld symbol rules).
static void path_to_ident(const char *path, char *out, size_t out_size) {
    nob_embed_path_to_ident(path, out, out_size);
}

// Build embedded assets WITHOUT GNU ld: bytes are emitted into a generated
// .c file (arrays + sizes) compiled with the regular C compiler.
// Missing asset files -> WARNING + g_have_embedded_assets=false (not fatal);
// example 32_embedded_assets is then skipped.
static bool build_embedded_assets(Nob_File_Paths *embedded_objects, bool release) {
    (void)release;
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    g_have_embedded_assets = false;

    size_t count = NOB_ARRAY_LEN(g_embedded_assets);
    if (count == 0) return true;

    Nob_File_Paths all_inputs = {0};
    for (size_t i = 0; i < count; i++) {
        nob_da_append(&all_inputs, g_embedded_assets[i].path);
    }

    // All-or-nothing: a single missing file disables the whole set.
    for (size_t i = 0; i < count; i++) {
        if (!nob_file_exists(g_embedded_assets[i].path)) {
            nob_log(NOB_WARNING, "Embedded asset missing: %s -- skipping embedded assets (example 32_embedded_assets will not build)",
                    g_embedded_assets[i].path);
            nob_da_free(all_inputs);
            return true;
        }
    }

    const char *data_c_path = OUTPUT_DIR "/embedded_assets_data.c";
    const char *obj_path = OUTPUT_DIR "/embedded_assets_data.obj";
    const char *header_path = SRC_DIR "/embedded_assets.hpp";

    int data_needs = nob_needs_rebuild(data_c_path, all_inputs.items, all_inputs.count);
    nob_da_free(all_inputs);
    if (data_needs < 0) return false;

    if (data_needs > 0 || !nob_file_exists(header_path)) {
        nob_log(NOB_INFO, "Generating embedded assets data: %s", data_c_path);
        Nob_String_Builder data = {0};
        // Banner kept identical to nob.c so switching tools does not rewrite the file.
        nob_sb_append_cstr(&data, "// Auto-generated by nob.c - DO NOT EDIT\n");
        nob_sb_append_cstr(&data, "#include <stddef.h>\n#include <stdint.h>\n\n");
        for (size_t i = 0; i < count; i++) {
            Nob_String_Builder blob = {0};
            if (!nob_read_entire_file(g_embedded_assets[i].path, &blob)) {
                nob_log(NOB_ERROR, "Could not read asset: %s", g_embedded_assets[i].path);
                nob_sb_free(blob);
                nob_sb_free(data);
                return false;
            }
            char ident[512];
            path_to_ident(g_embedded_assets[i].path, ident, sizeof(ident));
            nob_sb_appendf(&data, "const uint8_t _binary_%s_start[] = {\n", ident);
            sb_append_hex_bytes(&data, blob.items, blob.count);
            nob_sb_append_cstr(&data, "};\n");
            nob_sb_appendf(&data, "const size_t _binary_%s_size = sizeof(_binary_%s_start);\n\n",
                           ident, ident);
            nob_sb_free(blob);
        }
        nob_sb_append_null(&data);
        if (!nob_write_entire_file(data_c_path, data.items, data.count - 1)) {
            nob_sb_free(data);
            return false;
        }
        nob_sb_free(data);

        nob_log(NOB_INFO, "Generating embedded assets header: %s", header_path);
        Nob_String_Builder header = {0};
        nob_sb_append_cstr(&header, "// Auto-generated by nob.c - DO NOT EDIT\n#pragma once\n");
        nob_sb_append_cstr(&header, "#include <stddef.h>\n#include <stdint.h>\n\n");
        for (size_t i = 0; i < count; i++) {
            char ident[512];
            path_to_ident(g_embedded_assets[i].path, ident, sizeof(ident));
            nob_sb_appendf(&header, "extern const uint8_t _binary_%s_start[];\n", ident);
            nob_sb_appendf(&header, "extern const size_t _binary_%s_size;\n", ident);
        }
        nob_sb_append_cstr(&header, "\nstruct EmbeddedAsset {\n");
        nob_sb_append_cstr(&header, "    const char* name;\n");
        nob_sb_append_cstr(&header, "    const uint8_t* data;\n");
        nob_sb_append_cstr(&header, "    size_t size;\n");
        nob_sb_append_cstr(&header, "    int fontSize;  // -1 = texture, >= 0 = font\n");
        nob_sb_append_cstr(&header, "};\n\n");
        nob_sb_append_cstr(&header, "inline const EmbeddedAsset g_embeddedAssets[] = {\n");
        for (size_t i = 0; i < count; i++) {
            char ident[512];
            path_to_ident(g_embedded_assets[i].path, ident, sizeof(ident));
            nob_sb_appendf(&header, "    {\"%s\", _binary_%s_start, _binary_%s_size, %d},\n",
                           g_embedded_assets[i].path, ident, ident, g_embedded_assets[i].font_size);
        }
        nob_sb_append_cstr(&header, "};\n");
        nob_sb_appendf(&header, "inline const size_t g_embeddedAssetCount = %zu;\n", count);
        nob_sb_append_null(&header);
        if (!nob_write_entire_file(header_path, header.items, header.count - 1)) {
            nob_sb_free(header);
            return false;
        }
        nob_sb_free(header);
    }

    // Compile the data file with the plain C compiler (no C++ flags needed).
    const char *c_inputs[] = { data_c_path };
    if (nob_needs_rebuild(obj_path, c_inputs, 1) > 0) {
        nob_log(NOB_INFO, "Compiling embedded assets data: %s", obj_path);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CC, "-O2", "-c", data_c_path, "-o", obj_path);
        if (!nob_cmd_run(&cmd)) return false;
    }

    nob_da_append(embedded_objects, obj_path);
    g_have_embedded_assets = true;
    return true;
}

// ========== GPU SHADERS (same as nob.c; missing glslc -> skip) ==========

typedef struct {
    const char *name;
    const char *path;
    const char *stage;
} GpuShader;

static GpuShader g_gpu_shaders[] = {
    {"desaturate_frag", "examples/shaders/desaturate.frag", "frag"},
    {"triangle_vert",   "examples/shaders/triangle.vert",   "vert"},
    {"triangle_frag",   "examples/shaders/triangle.frag",   "frag"},
    {"time_water_frag", "examples/shaders/time_water.frag", "frag"},
    {"mouse_glow_frag", "examples/shaders/mouse_glow.frag", "frag"},
};

// Compile GLSL to SPIR-V (glslc) and embed the binaries the same portable
// way as assets: generated .c with byte arrays.
// glslc failure / missing sources -> WARNING + g_have_shaders=false
// (not fatal); examples 40/41_gpu_shader* are then skipped.
static bool build_gpu_shaders(Nob_File_Paths *shader_objects, bool release) {
    (void)release;
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    g_have_shaders = false;

    size_t count = NOB_ARRAY_LEN(g_gpu_shaders);
    if (count == 0) return true;

    // Step 1: Compile GLSL to SPIR-V via glslc
    for (size_t i = 0; i < count; i++) {
        if (!nob_file_exists(g_gpu_shaders[i].path)) {
            nob_log(NOB_WARNING, "Shader source missing: %s -- skipping GPU shaders (examples 40/41 will not build)",
                    g_gpu_shaders[i].path);
            return true;
        }
        const char *spv_path = nob_temp_sprintf("%s/shader_%s.spv", OUTPUT_DIR, g_gpu_shaders[i].name);

        const char *spv_inputs[] = { g_gpu_shaders[i].path };
        int needs = nob_needs_rebuild(spv_path, spv_inputs, 1);
        if (needs < 0) return false;
        if (needs == 0) continue;

        nob_log(INFO, "Compiling shader: %s -> %s", g_gpu_shaders[i].path, spv_path);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "glslc",
            nob_temp_sprintf("-fshader-stage=%s", g_gpu_shaders[i].stage),
            "-o", spv_path, g_gpu_shaders[i].path);
        if (!nob_cmd_run(&cmd)) {
            nob_log(NOB_WARNING, "glslc failed for %s -- skipping GPU shaders (examples 40/41 will not build). "
                    "Install glslc (Vulkan SDK: https://vulkan.lunarg.com) to enable them.", g_gpu_shaders[i].path);
            return true;
        }
    }

    // Step 2: Emit all SPIR-V binaries into one generated .c file
    Nob_File_Paths spv_inputs = {0};
    for (size_t i = 0; i < count; i++) {
        nob_da_append(&spv_inputs, nob_temp_sprintf("%s/shader_%s.spv", OUTPUT_DIR, g_gpu_shaders[i].name));
    }
    for (size_t i = 0; i < spv_inputs.count; i++) {
        if (!nob_file_exists(spv_inputs.items[i])) {
            nob_log(NOB_WARNING, "SPIR-V file missing: %s -- skipping GPU shaders", spv_inputs.items[i]);
            nob_da_free(spv_inputs);
            return true;
        }
    }

    const char *data_c_path = OUTPUT_DIR "/shader_spirv_data.c";
    const char *obj_path = OUTPUT_DIR "/shader_spirv_data.obj";
    const char *header_path = OUTPUT_DIR "/gpu_shader_spirv.hpp";

    int data_needs = nob_needs_rebuild(data_c_path, spv_inputs.items, spv_inputs.count);
    nob_da_free(spv_inputs);
    if (data_needs < 0) return false;

    if (data_needs > 0 || !nob_file_exists(header_path)) {
        nob_log(INFO, "Generating GPU shader data: %s", data_c_path);
        Nob_String_Builder data = {0};
        // Banner kept identical to nob.c (see note at the top of this file).
        nob_sb_append_cstr(&data, "// Auto-generated by nob.c - DO NOT EDIT\n");
        nob_sb_append_cstr(&data, "#include <stddef.h>\n#include <stdint.h>\n\n");
        for (size_t i = 0; i < count; i++) {
            const char *spv_path = nob_temp_sprintf("%s/shader_%s.spv", OUTPUT_DIR, g_gpu_shaders[i].name);
            Nob_String_Builder blob = {0};
            if (!nob_read_entire_file(spv_path, &blob)) {
                nob_log(NOB_ERROR, "Could not read SPIR-V: %s", spv_path);
                nob_sb_free(blob);
                nob_sb_free(data);
                return false;
            }
            nob_sb_appendf(&data, "const uint8_t shader_%s[] = {\n", g_gpu_shaders[i].name);
            sb_append_hex_bytes(&data, blob.items, blob.count);
            nob_sb_append_cstr(&data, "};\n");
            nob_sb_appendf(&data, "const size_t shader_%s_size = sizeof(shader_%s);\n\n",
                           g_gpu_shaders[i].name, g_gpu_shaders[i].name);
            nob_sb_free(blob);
        }
        nob_sb_append_null(&data);
        if (!nob_write_entire_file(data_c_path, data.items, data.count - 1)) {
            nob_sb_free(data);
            return false;
        }
        nob_sb_free(data);

        // Step 3: Generate header (same public names as nob.c)
        nob_log(INFO, "Generating GPU shader header: %s", header_path);
        Nob_String_Builder header = {0};
        nob_sb_append_cstr(&header, "// Auto-generated by nob.c - DO NOT EDIT\n");
        nob_sb_append_cstr(&header, "#pragma once\n\n");
        nob_sb_append_cstr(&header, "#include <stddef.h>\n");
        nob_sb_append_cstr(&header, "#include <stdint.h>\n\n");
        nob_sb_append_cstr(&header, "namespace gpu_shader {\n\n");
        nob_sb_append_cstr(&header, "extern \"C\" {\n");
        for (size_t i = 0; i < count; i++) {
            nob_sb_appendf(&header, "extern const uint8_t shader_%s[];\n", g_gpu_shaders[i].name);
            nob_sb_appendf(&header, "extern const size_t shader_%s_size;\n", g_gpu_shaders[i].name);
        }
        nob_sb_append_cstr(&header, "}\n\n");
        for (size_t i = 0; i < count; i++) {
            nob_sb_appendf(&header, "inline const uint8_t* %s = shader_%s;\n",
                           g_gpu_shaders[i].name, g_gpu_shaders[i].name);
            nob_sb_appendf(&header, "inline const size_t %s_size = shader_%s_size;\n",
                           g_gpu_shaders[i].name, g_gpu_shaders[i].name);
        }
        nob_sb_append_cstr(&header, "\n} // namespace gpu_shader\n");
        nob_sb_append_null(&header);

        if (!nob_write_entire_file(header_path, header.items, header.count - 1)) {
            nob_sb_free(header);
            return false;
        }
        nob_sb_free(header);
    }

    const char *c_inputs[] = { data_c_path };
    if (nob_needs_rebuild(obj_path, c_inputs, 1) > 0) {
        nob_log(NOB_INFO, "Compiling GPU shader data: %s", obj_path);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CC, "-O2", "-c", data_c_path, "-o", obj_path);
        if (!nob_cmd_run(&cmd)) return false;
    }

    nob_da_append(shader_objects, obj_path);
    g_have_shaders = true;
    return true;
}

// ========== EXAMPLE REQUIREMENTS ==========

// Optional build inputs. Missing assets / glslc / mixer / sqlite are not
// fatal: examples that need them are skipped with an INFO log telling what
// to drop into vendor/win64/.
typedef enum {
    NEED_NONE     = 0,
    NEED_EMBEDDED = 1 << 0,  // src/embedded_assets.hpp + embedded data .obj
    NEED_SHADERS  = 1 << 1,  // output/gpu_shader_spirv.hpp + SPIR-V data .obj + glslc
    NEED_MIXER    = 1 << 2,  // SDL3_mixer.lib in vendor/win64/<pkg>/lib/x64/
    NEED_SQLITE   = 1 << 3,  // sqlite3.lib (+ headers) in vendor/win64/sqlite3/
} ExampleNeeds;

static ExampleNeeds example_needs(const char *name) {
    if (strcmp(name, "32_embedded_assets") == 0) return NEED_EMBEDDED;
    if (strcmp(name, "40_gpu_shader") == 0) return NEED_SHADERS;
    if (strcmp(name, "41_gpu_shader_animation") == 0) return NEED_SHADERS;
    // Third-party opt-ins (keep substring matching so future examples work).
    if (strstr(name, "mixer") != NULL) return NEED_MIXER;
    if (strstr(name, "sqlite") != NULL) return NEED_SQLITE;
    return NEED_NONE;
}

static const char *example_needs_missing(ExampleNeeds needs) {
    if ((needs & NEED_EMBEDDED) && !g_have_embedded_assets) return "embedded assets unavailable";
    if ((needs & NEED_SHADERS) && !g_have_shaders) return "GPU shaders unavailable (no glslc)";
    if ((needs & NEED_MIXER) && !g_have_mixer) return "SDL3_mixer unavailable (drop SDL3_mixer-devel-*-mingw.zip content into vendor/win64/)";
    if ((needs & NEED_SQLITE) && !g_have_sqlite) return "sqlite3 unavailable (drop headers+import-lib into vendor/win64/sqlite3/" MINGW_TRIPLE "/{include,lib}/)";
    return NULL;
}

// ========== RUNTIME DLLS ==========

//Copy every probed .dll next to the binaries in dir (once; skips existing).
static bool copy_dlls_to(const char *dir) {
    if (!nob_mkdir_if_not_exists(dir)) return false;
    nob_da_foreach(const char*, src, &g_runtime_dlls) {
        const char *dst = nob_temp_sprintf("%s/%s", dir, nob_path_name(*src));
        if (nob_file_exists(dst)) continue;
        nob_log(INFO, "Copying runtime DLL: %s -> %s", *src, dst);
        if (!nob_copy_file(*src, dst)) return false;
    }
    return true;
}

// SDL import .libs are linked, so every .exe needs the DLLs beside it.
// dist/sdl_gui.dll (built by release) must also be visible to output/*.exe.
static bool ensure_runtime_dlls(void) {
    if (!copy_dlls_to(OUTPUT_DIR)) return false;
    if (!copy_dlls_to(OUTPUT_DIR "/release")) return false;
    const char *dll = DIST_DIR "/sdl_gui.dll";
    if (nob_file_exists(dll)) {
        const char *dst = nob_temp_sprintf("%s/sdl_gui.dll", OUTPUT_DIR);
        if (!nob_file_exists(dst)) {
            nob_log(INFO, "Copying runtime DLL: %s -> %s", dll, dst);
            if (!nob_copy_file(dll, dst)) return false;
        }
    }
    return true;
}

// ========== EXAMPLES ==========

static bool build_examples(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    if (!g_have_toolchain || !g_have_sdl) return false;

    if (!build_unity_object(release, false)) return false;

    // Build embedded assets
    Nob_File_Paths embedded_objects = {0};
    if (!build_embedded_assets(&embedded_objects, release)) return false;

    // Build GPU shaders
    Nob_File_Paths shader_objects = {0};
    if (!build_gpu_shaders(&shader_objects, release)) return false;

    Nob_File_Paths examples = {0};
    if (!collect_example_sources(&examples)) return false;

    const char *unity_obj = OUTPUT_DIR "/all.obj";
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0, skipped_no_deps = 0;

    nob_da_foreach(const char*, src, &examples) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *exe = nob_temp_sprintf("%s/%s.exe", OUTPUT_DIR, name);

        // Conditional build: skip examples whose optional inputs are missing.
        ExampleNeeds needs = example_needs(name);
        const char *missing = example_needs_missing(needs);
        if (missing) {
            nob_log(INFO, "Skipping example %s (%s)", name, missing);
            skipped_no_deps++;
            continue;
        }

        // Link only the optional objects this example actually needs
        // (keeps other binaries lean and dependency-free).
        bool link_embedded = (needs & NEED_EMBEDDED) != 0;
        bool link_shaders = (needs & NEED_SHADERS) != 0;

        // Build input list: source + unity_obj + needed optional objects
        Nob_File_Paths inputs = {0};
        nob_da_append(&inputs, *src);
        nob_da_append(&inputs, unity_obj);
        if (link_embedded) {
            for (size_t ei = 0; ei < embedded_objects.count; ei++) {
                nob_da_append(&inputs, embedded_objects.items[ei]);
            }
        }
        if (link_shaders) {
            for (size_t si = 0; si < shader_objects.count; si++) {
                nob_da_append(&inputs, shader_objects.items[si]);
            }
        }

        int should_rebuild = nob_needs_rebuild(exe, inputs.items, inputs.count);
        nob_da_free(inputs);
        if (should_rebuild <= 0) {
            skipped++;
            continue;
        }
        built++;

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, *src, unity_obj);
        if (link_embedded) {
            nob_da_foreach(const char*, obj, &embedded_objects) {
                nob_cmd_append(&cmd, *obj);
            }
        }
        if (link_shaders) {
            nob_da_foreach(const char*, obj, &shader_objects) {
                nob_cmd_append(&cmd, *obj);
            }
        }
        cmd_add_sdl3(&cmd);
        // NOTE: no per-example mixer/sqlite handling: their .libs (when
        // present in vendor/) are already part of cmd_add_sdl3().

        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);

        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }

    if (!nob_procs_wait(procs)) return false;
    if (!ensure_runtime_dlls()) return false;

    nob_log(INFO, "Examples: %zu built, %zu skipped, %zu skipped (missing optional deps)",
            built, skipped, skipped_no_deps);
    return true;
}

// ========== TESTS ==========

static bool build_tests(bool release, const char *filter) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    if (!g_have_toolchain || !g_have_sdl) return false;

    // Build catch_amalgamated.obj
    const char *catch_obj = OUTPUT_DIR "/catch_amalgamated.obj";
    const char *catch_inputs[] = {LIB_DIR "/catch_amalgamated.cpp", LIB_DIR "/catch_amalgamated.hpp"};
    if (nob_needs_rebuild(catch_obj, catch_inputs, 2) > 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        nob_cmd_append(&cmd, "-I", LIB_DIR, "-c", LIB_DIR "/catch_amalgamated.cpp", "-o", catch_obj);

        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, LIB_DIR "/catch_amalgamated.cpp", .output = catch_obj);

        if (!nob_cmd_run(&cmd)) return false;
    }

    // Build test_helper.obj
    const char *helper_obj = OUTPUT_DIR "/test_helper.obj";
    const char *helper_inputs[] = {TESTS_DIR "/test_helper.cpp", TESTS_DIR "/test_helper.hpp"};
    if (nob_needs_rebuild(helper_obj, helper_inputs, 2) > 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", TESTS_DIR "/test_helper.cpp", "-o", helper_obj);

        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, TESTS_DIR "/test_helper.cpp", .output = helper_obj);

        if (!nob_cmd_run(&cmd)) return false;
    }

    if (!build_unity_object(release, true)) return false;

    // NOTE: tests never touch embedded assets or GPU SPIR-V (ShaderPanel
    // tests use synthetic bytes), so optional objects are neither built
    // nor linked here.

    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;

    const char *unity_obj = OUTPUT_DIR "/all_test.obj";
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0, filtered = 0;

    nob_da_foreach(const char*, src, &tests) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';

        if (filter && !strstr(name, filter)) {
            filtered++;
            continue;
        }

        const char *exe = nob_temp_sprintf("%s/%s.exe", OUTPUT_DIR, name);

        // Build input list: source + unity_obj + catch_obj + helper_obj
        Nob_File_Paths inputs = {0};
        nob_da_append(&inputs, *src);
        nob_da_append(&inputs, unity_obj);
        nob_da_append(&inputs, helper_obj);
        nob_da_append(&inputs, catch_obj);

        int should_rebuild = nob_needs_rebuild(exe, inputs.items, inputs.count);
        nob_da_free(inputs);
        if (should_rebuild <= 0) {
            skipped++;
            continue;
        }
        built++;

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, *src, unity_obj, helper_obj, catch_obj);
        cmd_add_sdl3(&cmd);

        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);

        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }

    if (!nob_procs_wait(procs)) return false;
    if (!ensure_runtime_dlls()) return false;

    if (filtered > 0) {
        nob_log(INFO, "Tests: %zu built, %zu skipped, %zu filtered", built, skipped, filtered);
    } else {
        nob_log(INFO, "Tests: %zu built, %zu skipped", built, skipped);
    }
    return true;
}

typedef struct {
    const char *name;
    const char *log_path;
    Nob_Proc proc;
} TestRun;

typedef struct {
    TestRun *items;
    size_t count;
    size_t capacity;
} TestRuns;

static size_t test_jobs(void) {
    const char *env_jobs = getenv("NOB_TEST_JOBS");
    if (env_jobs) {
        int jobs = atoi(env_jobs);
        if (jobs > 0) return (size_t)jobs;
    }
    size_t procs = (size_t)nob_nprocs();
    return procs > 16 ? 16 : procs;
}

// Print the last `lines` lines of a file to stderr (for failing test logs).
static void print_tail(const char *path, size_t lines) {
    Nob_String_Builder sb = {0};
    if (!nob_read_entire_file(path, &sb)) {
        nob_log(ERROR, "  (could not read log: %s)", path);
        return;
    }
    nob_sb_append_null(&sb);

    size_t total = sb.count - 1; // without the NUL
    size_t start = 0;
    if (total > 0) {
        size_t found = 0;
        for (size_t i = total; i > 0; --i) {
            if (sb.items[i - 1] == '\n') {
                found++;
                if (found >= lines) { start = i; break; }
            }
        }
    }
    fprintf(stderr, "%.*s\n", (int)(total - start), sb.items + start);
    nob_sb_free(sb);
}

// Start one test process with stdout+stderr redirected to output/test_logs/<name>.log.
// Name and log path are strdup'd into stable memory (nob_temp_* gets clobbered).
// No `env` wrapper: SDL_GUI_HIDDEN=1 is set on our own process in run_tests()
// and inherited by every child (SetEnvironmentVariableA).
static void start_test(TestRuns *runs, const char *logs_dir, const char *name,
                       size_t *failed) {
    const char *exe = nob_temp_sprintf("%s/%s.exe", OUTPUT_DIR, name);
    const char *log_path = nob_temp_sprintf("%s/%s.log", logs_dir, name);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, exe);

    Nob_Fd fdout = nob_fd_open_for_write(log_path);
    if (fdout == NOB_INVALID_FD) {
        nob_log(ERROR, "Could not open test log %s", log_path);
        (*failed)++;
        return;
    }

    Nob_Proc proc = nob_cmd_run_async_redirect_and_reset(&cmd,
        (Nob_Cmd_Redirect){.fdout = &fdout, .fderr = &fdout});

    if (proc == NOB_INVALID_PROC) {
        nob_log(ERROR, "Could not start test %s", name);
        (*failed)++;
        return;
    }

    nob_da_append(runs, ((TestRun){
        .name = strdup(name),
        .log_path = strdup(log_path),
        .proc = proc
    }));
}

// The system clipboard is machine-global and test_text_area, test_text_input
// and test_text_editable all SDL_Set/GetClipboardText() in their paste/copy
// sections. They must never run concurrently (even across processes), or one
// binary's Set("") lands inside another's paste ("HNEWo" becomes "Ho").
// The parallel wave skips them; the serial wave runs them with jobs=1.
static bool is_clipboard_test(const char *name) {
    return strcmp(name, "test_text_area") == 0
        || strcmp(name, "test_text_input") == 0
        || strcmp(name, "test_text_editable") == 0;
}

// Dynamic work queue: keep up to `jobs` tests running; as soon as one
// finishes, start the next pending test.
static void run_one_wave(Nob_File_Paths *wave, const char *logs_dir, size_t jobs,
                         size_t *passed, size_t *failed) {
    TestRuns runs = {0};
    size_t next = 0;

    while (next < wave->count || runs.count > 0) {
        while (runs.count < jobs && next < wave->count) {
            start_test(&runs, logs_dir, wave->items[next], failed);
            next++;
        }

        bool progressed = false;
        size_t i = 0;
        while (i < runs.count) {
            int res = nob__proc_wait_async(runs.items[i].proc, 0);
            if (res == 1 || res == -1) {
                progressed = true;
                if (res == -1) {
                    (*failed)++;
                    nob_log(ERROR, "Test %s FAILED (log: %s)", runs.items[i].name, runs.items[i].log_path);
                    print_tail(runs.items[i].log_path, 60);
                } else {
                    (*passed)++;
                    nob_log(INFO, "Test %s passed", runs.items[i].name);
                }
                runs.items[i] = runs.items[runs.count - 1];
                runs.count--;
            } else {
                i++;
            }
        }

        if (!progressed && runs.count > 0) {
            Sleep(5); // all procs still running; avoid a busy spin
        }
    }
}

static bool run_tests(const char *filter) {
    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;
    if (!ensure_runtime_dlls()) return false;

    const char *logs_dir = OUTPUT_DIR "/test_logs";
    if (!nob_mkdir_if_not_exists(logs_dir)) return false;

    // Hidden windows for the whole run (inherited by child processes).
    SetEnvironmentVariableA("SDL_GUI_HIDDEN", "1");

    // Collect the names we are going to run (filtered), split into the
    // parallel wave and the serial clipboard wave (see is_clipboard_test).
    Nob_File_Paths wave_par = {0};
    Nob_File_Paths wave_clip = {0};
    size_t skipped = 0;
    nob_da_foreach(const char*, src, &tests) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';

        if (filter && !strstr(name, filter)) {
            skipped++;
            continue;
        }
        if (is_clipboard_test(name)) {
            nob_da_append(&wave_clip, strdup(name));
        } else {
            nob_da_append(&wave_par, strdup(name));
        }
    }

    const size_t jobs = test_jobs();
    nob_log(INFO, "Running tests (jobs=%zu, logs in %s/)", jobs, logs_dir);

    size_t passed = 0, failed = 0;
    run_one_wave(&wave_par, logs_dir, jobs, &passed, &failed);
    run_one_wave(&wave_clip, logs_dir, 1, &passed, &failed);

    if (skipped > 0) {
        nob_log(INFO, "Tests: %zu passed, %zu failed, %zu skipped (of %zu)", passed, failed, skipped, tests.count);
    } else {
        nob_log(INFO, "Tests: %zu passed, %zu failed (of %zu)", passed, failed, tests.count);
    }
    return failed == 0;
}

// ========== RELEASE ==========

// All public headers are merged into a single sdl_gui.hpp.
// (tables identical to nob.c -- dist/ output must not depend on the tool.)
static const char * hpp_order[] = {
    "src/std.hpp",
    "lib/tinyxml2.h",
    "src/easing.hpp",
    "src/sdl_deleters.hpp",
    "src/logger.hpp",
    "src/constants.hpp",
    "src/sdl_rect_helpers.hpp",
    "src/component_type.hpp",
    "src/element_handle.hpp",
    "src/overlay_stack.hpp",
    "src/anchor.hpp",
    "src/widget_factory.hpp",
    "src/layout_parser.hpp",
    "src/sgml_parser.hpp",
    "src/json_parser.hpp",
    "src/timer_manager.hpp",
    "src/animation_manager.hpp",
    "src/style.hpp",
    "src/font_manager.hpp",
    "src/texture_manager.hpp",
    "src/theme.hpp",
    "src/layout.hpp",
    "src/gui.hpp",
    "src/theme_presets.hpp",
    "src/label.hpp",
    "src/panel.hpp",
    "src/button.hpp",
    "src/checkbox.hpp",
    "src/slider.hpp",
    "src/range_slider.hpp",
    "src/progress_bar.hpp",
    "src/text_editable.hpp",
    "src/text_input.hpp",
    "src/canvas.hpp",
    "src/cursor.hpp",
    "src/animated_image.hpp",
    "src/radio_button.hpp",
    "src/radio_group.hpp",
    "src/tab_control.hpp",
    "src/text_area.hpp",
    "src/combobox.hpp",
    "src/context_menu.hpp",
    "src/string_grid.hpp",
    "src/list_view.hpp",
    "src/scroll_area.hpp",
    "src/world_view.hpp",
    "src/arc_container.hpp",
    "src/shader_panel.hpp",
    "src/gui_manager.hpp",
    "src/ui_helpers.hpp",
    "src/sdl_app.hpp",
    "src/screen.hpp",
    "src/screen_manager.hpp",
    "src/window.hpp",
    "src/window_manager.hpp",
    "src/gui_context.hpp",
    "src/composite/modal_dialog.hpp",
    "src/composite/dialog_box.hpp",
    "src/composite/message_box.hpp",
    "src/composite/file_dialog.hpp",
    "src/composite/color_dialog.hpp",
};

static const char * includes_to_remove[] = {
    "#include \"easing.hpp\"",
    "#include \"sdl_deleters.hpp\"",
    "#include \"layout_parser.hpp\"",
    "#include \"sgml_parser.hpp\"",
    "#include \"json_parser.hpp\"",
    "#include \"timer_manager.hpp\"",
    "#include \"animation_manager.hpp\"",
    "#include \"style.hpp\"",
    "#include \"font_manager.hpp\"",
    "#include \"texture_manager.hpp\"",
    "#include \"theme.hpp\"",
    "#include \"component_type.hpp\"",
    "#include \"element_handle.hpp\"",
    "#include \"overlay_stack.hpp\"",
    "#include \"widget_factory.hpp\"",
    "#include \"gui.hpp\"",
    "#include \"label.hpp\"",
    "#include \"panel.hpp\"",
    "#include \"button.hpp\"",
    "#include \"checkbox.hpp\"",
    "#include \"slider.hpp\"",
    "#include \"range_slider.hpp\"",
    "#include \"text_input.hpp\"",
    "#include \"canvas.hpp\"",
    "#include \"cursor.hpp\"",
    "#include \"animated_image.hpp\"",
    "#include \"radio_button.hpp\"",
    "#include \"radio_group.hpp\"",
    "#include \"tab_control.hpp\"",
    "#include \"text_area.hpp\"",
    "#include \"combobox.hpp\"",
    "#include \"context_menu.hpp\"",
    "#include \"gui_manager.hpp\"",
    "#include \"sdl_app.hpp\"",
    "#include \"composite/dialog_box.hpp\"",
    "#include \"composite/message_box.hpp\"",
    "#include \"../gui.hpp\"",
    "#include \"../panel.hpp\"",
    "#include \"../button.hpp\"",
    "#include \"../label.hpp\"",
    "#include \"../gui_manager.hpp\"",
    "#include \"../theme.hpp\"",
    "#include \"../text_input.hpp\"",
    "#include \"dialog_box.hpp\"",
    "#include \"../lib/tinyxml2.h\"",
    "#include \"anchor.hpp\"",
    "#include \"layout.hpp\"",
    "#include \"text_editable.hpp\"",
    "#include \"string_grid.hpp\"",
    "#include \"screen.hpp\"",
    "#include \"window.hpp\"",
    "#include \"editor_element.hpp\"",
    "#include \"editor_state.hpp\"",
    "#include \"layout_exporter.hpp\"",
    "#include \"../string_grid.hpp\"",
    "#include \"../style.hpp\"",
    "#include \"../checkbox.hpp\"",
    "#include \"../combobox.hpp\"",
    "#include \"../text_area.hpp\"",
    "#include \"../slider.hpp\"",
    "#include \"../range_slider.hpp\"",
    "#include \"../list_view.hpp\"",
    "#include \"../window_manager.hpp\"",
    "#include \"std.hpp\"",
    "#include \"logger.hpp\"",
    "#include \"constants.hpp\"",
    "#include \"sdl_rect_helpers.hpp\"",
    "#include \"tinyxml2.h\"",
    "#include \"string_grid.hpp\"",
    "#include \"list_view.hpp\"",
    "#include \"progress_bar.hpp\"",
    "#include \"scroll_area.hpp\"",
    "#include \"world_view.hpp\"",
    "#include \"shader_panel.hpp\"",
    "#include \"arc_container.hpp\"",
    "#include \"screen.hpp\"",
    "#include \"screen_manager.hpp\"",
    "#include \"window.hpp\"",
    "#include \"window_manager.hpp\"",
    "#include \"gui_context.hpp\"",
    "#include \"theme_presets.hpp\"",
    "#include \"composite/file_dialog.hpp\"",
    "#include \"composite/modal_dialog.hpp\"",
    "#include \"composite/color_dialog.hpp\"",
    "#include \"color_dialog.hpp\"",
    "#include \"modal_dialog.hpp\"",
    "#include \"file_dialog.hpp\"",
};

static bool is_support_header(const char *path) {
    return strstr(path, "tinyxml2.h") != NULL
        || strstr(path, "std.hpp") != NULL
        || strstr(path, "logger.hpp") != NULL
        || strstr(path, "constants.hpp") != NULL
        || strstr(path, "sdl_rect_helpers.hpp") != NULL;
}

static bool line_is_std_header_guard(const char *line) {
    if (strstr(line, "__clangd__") != NULL) return true;
    if (strcmp(line, "#else") == 0) return true;
    if (strcmp(line, "#endif") == 0) return true;
    return false;
}

static bool line_should_remove(const char *line, const char *path) {
    if (strncmp(line, "#pragma once", 12) == 0) return true;
    bool support = is_support_header(path);
    if (!support) {
        if (strncmp(line, "#include <", 10) == 0) return true;
        if (strncmp(line, "#include \"SDL2/", 15) == 0) return true;
    }
    if (strstr(path, "std.hpp") != NULL && line_is_std_header_guard(line)) return true;
    if (strncmp(line, "import ", 7) == 0) return true;
    if (strncmp(line, "module;", 7) == 0) return true;
    if (strlen(line) == 0) return true;

    for (size_t i = 0; i < NOB_ARRAY_LEN(includes_to_remove); i++) {
        if (strstr(line, includes_to_remove[i])) return true;
    }
    return false;
}

static bool build_combined_header(void) {
    const char *output = DIST_DIR "/sdl_gui.hpp";

    if (nob_needs_rebuild(output, hpp_order, NOB_ARRAY_LEN(hpp_order)) <= 0) {
        return true;
    }

    nob_log(INFO, "Building combined header: %s", output);

    Nob_String_Builder sb = {0};
    nob_sb_append_cstr(&sb, "// Auto-generated header. Do not edit.\n#pragma once\n\n");
    nob_sb_append_cstr(&sb, "// System headers (library requires: SDL3, SDL3_image, SDL3_ttf)\n");
    nob_sb_append_cstr(&sb, "#include <SDL3/SDL.h>\n#include <SDL3/SDL_gpu.h>\n");
    nob_sb_append_cstr(&sb, "#include <SDL3_image/SDL_image.h>\n#include <SDL3_ttf/SDL_ttf.h>\n\n");
    nob_sb_append_cstr(&sb, "// ===== SDL GUI - complete public API (single header) =====\n\n");
    nob_sb_append_cstr(&sb, "// Compilation: clang++ -std=c++23 -stdlib=libc++\n\n");

    for (size_t i = 0; i < NOB_ARRAY_LEN(hpp_order); i++) {
        Nob_String_Builder file = {0};
        if (!nob_read_entire_file(hpp_order[i], &file)) {
            nob_sb_free(file);
            nob_sb_free(sb);
            return false;
        }
        nob_sb_append_null(&file);

        char *line = file.items;
        while (line && *line) {
            char *end = strchr(line, '\n');
            size_t len = end ? (size_t)(end - line) : strlen(line);
            // Windows checkouts use CRLF: a trailing '\r' would break the
            // exact strcmp matches below ("#else", "#endif") and leak into
            // the output. (Same latent issue exists in nob.c.)
            while (len > 0 && line[len - 1] == '\r') len--;
            char *copy = nob_temp_strndup(line, len);

            if (!line_should_remove(copy, hpp_order[i])) {
                nob_sb_append_buf(&sb, line, len);
                nob_sb_append_cstr(&sb, "\n");
            }
            line = end ? end + 1 : NULL;
        }
        nob_sb_free(file);
    }
    nob_sb_append_null(&sb);

    if (!nob_write_entire_file(output, sb.items, sb.count - 1)) {
        nob_sb_free(sb);
        return false;
    }
    nob_sb_free(sb);
    return true;
}

// Release: static lib (sdl_gui_static.lib) + DLL (sdl_gui.dll + import
// sdl_gui.lib) + combined header + C header + docs + skill + smoke tests.
static bool build_release(void) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    nob_mkdir_if_not_exists(OUTPUT_DIR "/release");
    nob_mkdir_if_not_exists(OUTPUT_DIR "/release/composite");
    nob_mkdir_if_not_exists(DIST_DIR);
    init_globals();
    if (!g_have_toolchain || !g_have_sdl) return false;

    Nob_File_Paths sources = {0};
    Nob_File_Paths composite = {0};
    Nob_File_Paths editor = {0};
    if (!collect_cpp_sources(&sources)) return false;
    if (!collect_composite_sources(&composite)) return false;
    if (!collect_editor_sources(&editor)) return false;

    Nob_File_Paths objects = {0};
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0;

    // Compile src/*.cpp -> release/*.obj (parallel, no -fPIC on COFF)
    nob_da_foreach(const char*, src, &sources) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/release/%s.obj", OUTPUT_DIR, name);
        nob_da_append(&objects, nob_temp_strdup(obj));

        Nob_File_Paths deps = {0};
        nob_da_append(&deps, *src);
        collect_hpp_sources(&deps);
        collect_files(&deps, SRC_DIR "/composite", ".hpp");

        if (nob_needs_rebuild(obj, deps.items, deps.count) <= 0) {
            skipped++;
            continue;
        }
        built++;

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);

        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);

        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }

    // Compile src/composite/*.cpp -> release/composite/*.obj (parallel)
    nob_da_foreach(const char*, src, &composite) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/release/composite/%s.obj", OUTPUT_DIR, name);
        nob_da_append(&objects, nob_temp_strdup(obj));

        Nob_File_Paths deps = {0};
        nob_da_append(&deps, *src);
        collect_hpp_sources(&deps);
        collect_files(&deps, SRC_DIR "/composite", ".hpp");

        if (nob_needs_rebuild(obj, deps.items, deps.count) <= 0) {
            skipped++;
            continue;
        }
        built++;

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);

        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);

        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }

    // Compile tinyxml2.cpp -> release .obj
    const char *tinyxml2_obj = OUTPUT_DIR "/release/tinyxml2.obj";
    nob_da_append(&objects, nob_temp_strdup(tinyxml2_obj));
    const char *tinyxml2_inputs[] = {LIB_DIR "/tinyxml2.cpp", LIB_DIR "/tinyxml2.h"};
    if (nob_needs_rebuild(tinyxml2_obj, tinyxml2_inputs, 2) > 0) {
        built++;
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", LIB_DIR "/tinyxml2.cpp", "-o", tinyxml2_obj);

        nob_compdb_add(&g_compdb, &cmd, LIB_DIR "/tinyxml2.cpp", .output = tinyxml2_obj);

        if (!nob_cmd_run(&cmd)) return false;
    } else {
        skipped++;
    }

    if (!nob_procs_wait(procs)) return false;
    nob_log(INFO, "Release objects: %zu built, %zu skipped", built, skipped);

    // Build static library (GNU ar format; the .lib extension is only
    // a naming convention, GNU ld links it the same as .a).
    const char *lib_static = DIST_DIR "/sdl_gui_static.lib";
    if (nob_needs_rebuild(lib_static, objects.items, objects.count) > 0) {
        nob_log(INFO, "Building static library: %s", lib_static);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, LLVM_AR, "rcs", lib_static);
        nob_da_foreach(const char*, obj, &objects) {
            nob_cmd_append(&cmd, *obj);
        }
        if (!nob_cmd_run(&cmd)) return false;
    }

    // Build shared library. GNU ld writes the import library where told
    // (dist/sdl_gui.lib); C examples and the C smoke test link against it.
    const char *lib_dll = DIST_DIR "/sdl_gui.dll";
    if (nob_needs_rebuild(lib_dll, objects.items, objects.count) > 0) {
        nob_log(INFO, "Building shared library: %s", lib_dll);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        nob_cmd_append(&cmd, "-shared", "-o", lib_dll);
        nob_cmd_append(&cmd, "-Wl,--out-implib," DIST_DIR "/sdl_gui.lib");
        nob_da_foreach(const char*, obj, &objects) {
            nob_cmd_append(&cmd, *obj);
        }
        cmd_add_sdl3(&cmd);
        if (!nob_cmd_run(&cmd)) return false;
    }

    if (!build_combined_header()) return false;

    // Copy C API header to dist/ (no shell `cp`)
    {
        const char *c_header_src  = SRC_DIR "/sdl_gui.h";
        const char *c_header_dst  = DIST_DIR "/sdl_gui.h";
        const char *c_header_deps[] = {c_header_src};
        if (nob_needs_rebuild(c_header_dst, c_header_deps, 1) > 0) {
            nob_log(INFO, "Copying C API header: %s -> %s", c_header_src, c_header_dst);
            if (!nob_copy_file(c_header_src, c_header_dst)) return false;
        }
    }

    // Copy end-user documentation to dist/docs/
    {
        const char *docs_src = "docs/release";
        const char *docs_marker = nob_temp_sprintf("%s/%s", docs_src, "_STYLE.md");
        if (!nob_file_exists(docs_marker)) {
            nob_log(ERROR, "Missing docs/release/ - run documentation generator first");
            return false;
        }
        nob_log(INFO, "Copying documentation: %s -> %s/docs/", docs_src, DIST_DIR);
        if (!rm_rf_win(DIST_DIR "/docs")) return false;
        if (!nob_copy_directory_recursively(docs_src, DIST_DIR "/docs")) {
            nob_log(ERROR, "Could not copy docs to %s/docs", DIST_DIR);
            return false;
        }
    }

    // Copy the agent skill to dist/skills/
    {
        const char *skill_src = "skills/sdl-gui";
        const char *skill_marker = nob_temp_sprintf("%s/%s", skill_src, "SKILL.md");
        if (!nob_file_exists(skill_marker)) {
            nob_log(ERROR, "Missing skills/sdl-gui/SKILL.md");
            return false;
        }
        nob_log(INFO, "Copying agent skill: %s -> %s/skills/", skill_src, DIST_DIR);
        if (!rm_rf_win(DIST_DIR "/skills")) return false;
        if (!nob_mkdir_if_not_exists(DIST_DIR "/skills")) return false;
        if (!nob_copy_directory_recursively(skill_src, DIST_DIR "/skills/sdl-gui")) {
            nob_log(ERROR, "Could not copy skill to %s/skills/sdl-gui", DIST_DIR);
            return false;
        }
    }

    // Smoke test: compile standalone example with combined header + static library
    {
        const char *standalone_src = EXAMPLES_DIR "/47_standalone.cpp";
        const char *standalone_exe = OUTPUT_DIR "/release/47_standalone.exe";
        const char *standalone_inputs[] = {
            standalone_src,
            DIST_DIR "/sdl_gui.hpp",
            DIST_DIR "/sdl_gui_static.lib",
        };
        if (nob_needs_rebuild(standalone_exe, standalone_inputs, 3) > 0) {
            nob_log(INFO, "Building standalone example (release sanity check)...");
            Nob_Cmd cmd = {0};
            nob_cmd_append(&cmd, CXX);
            cmd_add_common(&cmd);
            cmd_add_mode(&cmd, true);
            // Only -I dist: proves that sdl_gui.hpp is self-contained
            // (the user doesn't need src/ or lib/).
            nob_cmd_append(&cmd, "-I" DIST_DIR);
            nob_cmd_append(&cmd, "-o", standalone_exe, standalone_src, DIST_DIR "/sdl_gui_static.lib");
            cmd_add_sdl3(&cmd);
            if (!nob_cmd_run(&cmd)) return false;
        }
    }

    // C API smoke test: compile a pure C file with clang against
    // dist/sdl_gui.h + the DLL import library (dist/sdl_gui.lib)
    {
        const char *c_test_src  = OUTPUT_DIR "/release/c_smoke_test.c";
        const char *c_test_exe  = OUTPUT_DIR "/release/c_smoke_test.exe";
        const char *c_header    = DIST_DIR "/sdl_gui.h";
        const char *c_lib       = DIST_DIR "/sdl_gui.lib";
        const char *c_inputs[]  = {c_test_src, c_header, c_lib};
        const char *c_source =
            "#include <sdl_gui.h>\n"
            "int main(void) {\n"
            "    sdlgui_t g = sdlgui_create(\"test\", 640, 480, 0);\n"
            "    if (g) sdlgui_destroy(g);\n"
            "    return 0;\n"
            "}\n";
        if (!nob_write_entire_file(c_test_src, c_source, strlen(c_source))) return false;
        if (nob_needs_rebuild(c_test_exe, c_inputs, 3) > 0) {
            nob_log(INFO, "C API smoke test: compiling with clang (pure C)...");
            const char *c_obj = nob_temp_sprintf("%s.obj", c_test_exe);
            Nob_Cmd cc_cmd = {0};
            nob_cmd_append(&cc_cmd, CC, "-std=c11", "-pedantic-errors",
                "-I" DIST_DIR,
                "-c", c_test_src, "-o", c_obj);
            cmd_add_sdl3_includes(&cc_cmd);
            if (!nob_cmd_run(&cc_cmd)) return false;

            Nob_Cmd link_cmd = {0};
            nob_cmd_append(&link_cmd, CXX);
            cmd_add_common(&link_cmd);
            nob_cmd_append(&link_cmd,
                "-o", c_test_exe, c_obj, c_lib);
            cmd_add_sdl3(&link_cmd);
            if (!nob_cmd_run(&link_cmd)) return false;
        }
    }

    // Build C examples (pure C source compiled with clang, linked with clang++)
    {
        const char *c_examples_dir = EXAMPLES_DIR "/c";
        Nob_Dir_Entry entry = {0};
        if (nob_dir_entry_open(c_examples_dir, &entry)) {
            while (nob_dir_entry_next(&entry)) {
                if (!strstr(entry.name, ".c")) continue;
                const char *src = nob_temp_sprintf("%s/%s", c_examples_dir, entry.name);
                char *name = nob_temp_strdup(entry.name);
                char *dot = strrchr(name, '.');
                if (dot) *dot = '\0';
                const char *exe = nob_temp_sprintf("%s/c_%s.exe", OUTPUT_DIR, name);

                const char *c_header = DIST_DIR "/sdl_gui.h";
                const char *c_lib    = DIST_DIR "/sdl_gui.lib";
                const char *inputs[] = {src, c_header, c_lib};
                if (nob_needs_rebuild(exe, inputs, 3) <= 0) continue;

                nob_log(INFO, "Building C example: %s", name);
                const char *c_obj = nob_temp_sprintf("%s/%s_c.obj", OUTPUT_DIR, name);
                Nob_Cmd cc_cmd = {0};
                nob_cmd_append(&cc_cmd, CC, "-std=c11", "-pedantic-errors",
                    "-I" DIST_DIR,
                    "-c", src, "-o", c_obj);
                cmd_add_sdl3_includes(&cc_cmd);
                if (!nob_cmd_run(&cc_cmd)) {
                    nob_dir_entry_close(entry);
                    return false;
                }
                Nob_Cmd link_cmd = {0};
                nob_cmd_append(&link_cmd, CXX);
                cmd_add_common(&link_cmd);
                nob_cmd_append(&link_cmd,
                    "-o", exe, c_obj, c_lib);
                cmd_add_sdl3(&link_cmd);
                if (!nob_cmd_run(&link_cmd)) {
                    nob_dir_entry_close(entry);
                    return false;
                }
            }
            nob_dir_entry_close(entry);
        }
    }

    // Smoke-test exes live in output/ and output/release/; both need DLLs.
    if (!ensure_runtime_dlls()) return false;

    nob_log(INFO, "Release finished. Artifacts in %s/", DIST_DIR);
    return true;
}

// ========== NON-UNITY ==========

static bool build_non_unity(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    if (!g_have_toolchain || !g_have_sdl) return false;

    Nob_File_Paths sources = {0};
    Nob_File_Paths composite = {0};
    Nob_File_Paths editor = {0};
    if (!collect_cpp_sources(&sources)) return false;
    if (!collect_composite_sources(&composite)) return false;
    if (!collect_editor_sources(&editor)) return false;

    nob_da_foreach(const char*, src, &sources) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/%s.obj", OUTPUT_DIR, name);

        nob_log(INFO, "Compiling: %s", *src);

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);

        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);

        if (!nob_cmd_run(&cmd)) return false;
    }

    nob_da_foreach(const char*, src, &composite) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/composite_%s.obj", OUTPUT_DIR, name);

        nob_log(INFO, "Compiling: %s", *src);

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);

        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);

        if (!nob_cmd_run(&cmd)) return false;
    }

    nob_da_foreach(const char*, src, &editor) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/editor_%s.obj", OUTPUT_DIR, name);

        nob_log(INFO, "Compiling: %s", *src);

        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_sdl3_includes(&cmd);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);

        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);

        if (!nob_cmd_run(&cmd)) return false;
    }

    nob_log(INFO, "Non-unity finished");
    return true;
}

// ========== BENCH ==========

// Synthetic GUI benchmark on a REAL window (see bench/bench_gui.cpp).
// Builds output/bench_gui.exe; run it directly, e.g.:
//   output/bench_gui.exe --widgets 200 --seconds 5
static bool build_bench(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    if (!g_have_toolchain || !g_have_sdl) return false;

    if (!build_unity_object(release, false)) return false;

    // NOTE: bench_gui.cpp uses no embedded assets or GPU shaders.
    const char *src = BENCH_DIR "/bench_gui.cpp";
    const char *exe = OUTPUT_DIR "/bench_gui.exe";
    const char *unity_obj = OUTPUT_DIR "/all.obj";

    Nob_File_Paths inputs = {0};
    nob_da_append(&inputs, src);
    nob_da_append(&inputs, unity_obj);

    if (nob_needs_rebuild(exe, inputs.items, inputs.count) > 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, src, unity_obj);
        cmd_add_sdl3(&cmd);

        nob_compdb_add(&g_compdb, &cmd, src, .output = exe);

        nob_da_free(inputs);
        if (!nob_cmd_run(&cmd)) return false;
    } else {
        nob_da_free(inputs);
    }

    if (!ensure_runtime_dlls()) return false;

    nob_log(INFO, "Bench ready: %s", exe);
    return true;
}

// ========== CLEAN (no shell `rm -rf`) ==========

// Recursive delete via Win32 API (nob.h has no recursive dir remove).
static bool rm_rf_win(const char *path) {
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return true; // already gone
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        Nob_Dir_Entry e = {0};
        if (!nob_dir_entry_open(path, &e)) return false;
        bool ok = true;
        while (nob_dir_entry_next(&e)) {
            if (strcmp(e.name, ".") == 0 || strcmp(e.name, "..") == 0) continue;
            if (!rm_rf_win(nob_temp_sprintf("%s/%s", path, e.name))) ok = false;
        }
        nob_dir_entry_close(e);
        if (!ok) return false;
        SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
        if (!RemoveDirectoryA(path)) {
            nob_log(NOB_ERROR, "Could not remove directory %s", path);
            return false;
        }
        return true;
    }
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
    if (!DeleteFileA(path)) {
        nob_log(NOB_ERROR, "Could not delete file %s", path);
        return false;
    }
    return true;
}

static bool clean(void) {
    nob_log(INFO, "Cleaning...");
    bool ok = true;
    if (!rm_rf_win(OUTPUT_DIR)) ok = false;
    if (!rm_rf_win(DIST_DIR)) ok = false;
    if (!rm_rf_win("modules_cache")) ok = false; // kept for Linux-dir cleanliness
    if (ok) nob_log(INFO, "Cleaned");
    return ok;
}

// ========== MAIN ==========

int main(int argc, char **argv) {
    // NOTE: no NOB_GO_REBUILD_URSELF -- it shells out to `cc`, which does not
    // exist on Windows. Rebuild this tool explicitly: clang -o nob_win nob_win.c
    init_globals();

    const char *target = "examples";
    bool release = false;
    const char *test_filter = NULL;

    if (argc > 1) {
        const char *arg = argv[1];
        if (strcmp(arg, "test") == 0) {
            target = "test";
            if (argc > 2) test_filter = argv[2];
        } else if (strcmp(arg, "release") == 0) { target = "release"; release = true; }
        else if (strcmp(arg, "clean") == 0) target = "clean";
        else if (strcmp(arg, "non_unity") == 0) target = "non_unity";
        else if (strcmp(arg, "examples") == 0) target = "examples";
        else if (strcmp(arg, "bench") == 0) target = "bench";
        else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--release") == 0) {
            release = true;
            if (argc > 2) target = argv[2];
        } else {
            nob_log(ERROR, "Unknown target: %s", arg);
            nob_log(INFO, "Targets: examples, test, bench, release, clean, non_unity");
            return 1;
        }
    }

    bool ok = false;
    if (strcmp(target, "examples") == 0) ok = build_examples(release);
    else if (strcmp(target, "test") == 0) { ok = build_tests(release, test_filter); if (ok) ok = run_tests(test_filter); }
    else if (strcmp(target, "bench") == 0) ok = build_bench(release);
    else if (strcmp(target, "release") == 0) ok = build_release();
    else if (strcmp(target, "clean") == 0) ok = clean();
    else if (strcmp(target, "non_unity") == 0) ok = build_non_unity(release);

    // Save compile_commands.json if there were compilations
    if (ok && g_compdb.count > 0) {
        nob_compdb_save(&g_compdb, "compile_commands.json");
        nob_log(INFO, "Generated compile_commands.json (%zu entries)", g_compdb.count);
    }
    // No need to free - program exits

    return ok ? 0 : 1;
}