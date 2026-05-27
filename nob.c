#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdlib.h>
#include <string.h>

#define CXX "clang++-22"
#define CC "clang-22"

#define SRC_DIR "src"
#define OUTPUT_DIR "output"
#define DIST_DIR "dist"
#define TESTS_DIR "tests"
#define LIB_DIR "lib"
#define MODULE_CACHE_DIR "modules_cache"
#define EXAMPLES_DIR "examples"

#define STD_MODULE_SRC "/usr/lib/llvm-23/share/libc++/v1/std.cppm"
#define STD_COMPAT_MODULE_SRC "/usr/lib/llvm-23/share/libc++/v1/std.compat.cppm"

// ========== PREKOMPILOWANE FLAGI ==========

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
    "-stdlib=libc++",
    "-std=c++23",
    "-I/usr/include/SDL2",
    "-Isrc",
    "-Ilib",
};

static const char * debug_flags[] = {
    "-g",
    "-O0",
    "-fsanitize=address,undefined",
    "-fno-omit-frame-pointer",
    "-fno-optimize-sibling-calls",
    "-DDEBUG=1",
};

static const char * release_flags[] = {
    "-O3",
    "-march=native",
    "-flto",
    "-DNDEBUG",
};

// Globalne prekompilowane komendy (initialized once)
static Nob_Cmd g_common = {0};
static Nob_Cmd g_debug = {0};
static Nob_Cmd g_release = {0};
static Nob_Compdb g_compdb = {0};  // compile_commands.json entries
static bool g_initialized = false;

static void init_globals(void) {
    if (g_initialized) return;
    nob_da_append_many(&g_common, common_flags, NOB_ARRAY_LEN(common_flags));
    nob_da_append_many(&g_debug, debug_flags, NOB_ARRAY_LEN(debug_flags));
    nob_da_append_many(&g_release, release_flags, NOB_ARRAY_LEN(release_flags));
    g_initialized = true;
}

// ========== HELPER FUNCTIONS ==========

static void cmd_add_common(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_common);
}

static void cmd_add_mode(Nob_Cmd *cmd, bool release) {
    nob_cmd_extend(cmd, release ? &g_release : &g_debug);
}

static void cmd_add_sdl2_libs(Nob_Cmd *cmd) {
    nob_cmd_append(cmd, "-lSDL2", "-lSDL2_image", "-lSDL2_ttf", "-lSDL2_gfx");
}

static const char *std_pcm_path(bool release) {
    return release ? MODULE_CACHE_DIR "/std_release.pcm" : MODULE_CACHE_DIR "/std_debug.pcm";
}

static const char *std_compat_pcm_path(bool release) {
    return release ? MODULE_CACHE_DIR "/std_compat_release.pcm" : MODULE_CACHE_DIR "/std_compat_debug.pcm";
}

static void cmd_add_modules(Nob_Cmd *cmd, bool release) {
    nob_cmd_append(cmd, 
        nob_temp_sprintf("-fmodule-file=std=%s", std_pcm_path(release)),
        nob_temp_sprintf("-fmodule-file=std.compat=%s", std_compat_pcm_path(release)));
}

// ========== FILE COLLECTION ==========

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
    return collect_files(paths, EXAMPLES_DIR, ".cpp");
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

// ========== UNITY BUILD ==========

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

// ========== MODULES ==========

static bool build_modules(bool release) {
    nob_mkdir_if_not_exists(MODULE_CACHE_DIR);
    
    const char *std_pcm = std_pcm_path(release);
    const char *std_compat_pcm = std_compat_pcm_path(release);
    
    // Check if std module needs rebuild
    if (!nob_file_exists(std_pcm) || nob_needs_rebuild1(std_pcm, STD_MODULE_SRC) > 0) {
        nob_log(INFO, "Building std module (%s)...", release ? "release" : "debug");
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX, "-std=c++23", "-stdlib=libc++",
                       "-Wno-reserved-identifier", "-Wno-reserved-module-identifier",
                       release ? "-O3" : "-g", release ? "-march=native" : "-O0");
        if (release) nob_cmd_append(&cmd, "-flto");
        nob_cmd_append(&cmd, "--precompile", "-o", std_pcm, STD_MODULE_SRC);
        
        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, STD_MODULE_SRC, .output = std_pcm);
        
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    // Check if std.compat module needs rebuild (depends on std_pcm)
    if (!nob_file_exists(std_compat_pcm)) {
        nob_log(INFO, "Building std.compat module (%s)...", release ? "release" : "debug");
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX, "-std=c++23", "-stdlib=libc++",
                       nob_temp_sprintf("-fmodule-file=std=%s", std_pcm),
                       "-Wno-reserved-identifier", "-Wno-reserved-module-identifier",
                       release ? "-O3" : "-g", release ? "-march=native" : "-O0");
        if (release) nob_cmd_append(&cmd, "-flto");
        nob_cmd_append(&cmd, "--precompile", "-o", std_compat_pcm, STD_COMPAT_MODULE_SRC);
        
        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, STD_COMPAT_MODULE_SRC, .output = std_compat_pcm);
        
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    return true;
}

// ========== UNITY OBJECT ==========

static bool build_unity_object(bool release, bool for_tests) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    
    if (!build_modules(release)) return false;
    if (!ensure_unity_source()) return false;
    
    const char *output_obj = for_tests ? OUTPUT_DIR "/all_test.o" : OUTPUT_DIR "/all.o";
    
    // Unity object depends on:
    // - all.cpp (timestamp only changes if content changes)
    // - all .cpp files in src/ (because #included by all.cpp)
    // - all .hpp files in src/ (because #included by .cpp files)
    // - modules (std.pcm, std_compat.pcm)
    Nob_File_Paths deps = {0};
    collect_cpp_sources(&deps);
    collect_composite_sources(&deps);
    collect_editor_sources(&deps);
    collect_hpp_sources(&deps);
    collect_files(&deps, SRC_DIR "/composite", ".hpp");
    collect_files(&deps, SRC_DIR "/editor", ".hpp");
    nob_da_append(&deps, OUTPUT_DIR "/all.cpp");
    nob_da_append(&deps, std_pcm_path(release));
    nob_da_append(&deps, std_compat_pcm_path(release));
    
    int needs = nob_needs_rebuild(output_obj, deps.items, deps.count);
    if (needs <= 0 && nob_file_exists(output_obj)) {
        return true;
    }
    
    nob_log(INFO, "Building unity object: %s", output_obj);
    
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX);
    cmd_add_common(&cmd);
    cmd_add_mode(&cmd, release);
    cmd_add_modules(&cmd, release);
    nob_cmd_append(&cmd, "-c", OUTPUT_DIR "/all.cpp", "-o", output_obj);
    
    // Add to compile_commands.json
    nob_compdb_add(&g_compdb, &cmd, OUTPUT_DIR "/all.cpp", .output = output_obj);
    
    // Also add individual source files to compile_commands.json for clangd
    // (even though they're compiled together in unity build)
    Nob_Cmd individual_cmd = {0};
    nob_cmd_append(&individual_cmd, CXX);
    cmd_add_common(&individual_cmd);
    cmd_add_mode(&individual_cmd, release);
    cmd_add_modules(&individual_cmd, release);
    nob_cmd_append(&individual_cmd, "-c");
    // File path will be set per file
    
    Nob_File_Paths all_sources = {0};
    collect_cpp_sources(&all_sources);
    collect_composite_sources(&all_sources);
    collect_editor_sources(&all_sources);
    
    nob_da_foreach(const char*, src, &all_sources) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/%s.o", OUTPUT_DIR, name);
        nob_compdb_add(&g_compdb, &individual_cmd, *src, .output = obj);
    }
    
    return nob_cmd_run(&cmd);
}

// ========== EXAMPLES ==========

static bool build_examples(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    
    if (!build_unity_object(release, false)) return false;
    
    Nob_File_Paths examples = {0};
    if (!collect_example_sources(&examples)) return false;
    
    const char *unity_obj = OUTPUT_DIR "/all.o";
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0;
    
    nob_da_foreach(const char*, src, &examples) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);
        
        // Rebuild check
        const char *inputs[] = {*src, unity_obj};
        if (nob_needs_rebuild(exe, inputs, 2) <= 0) {
            skipped++;
            continue;
        }
        built++;
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, *src, unity_obj);
        cmd_add_sdl2_libs(&cmd);
        
        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }
    
    if (!nob_procs_wait(procs)) return false;
    
    nob_log(INFO, "Examples: %zu built, %zu skipped", built, skipped);
    return true;
}

// ========== TESTS ==========

static bool build_tests(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    
    if (!build_modules(release)) return false;
    
    // Build catch_amalgamated.o
    const char *catch_obj = OUTPUT_DIR "/catch_amalgamated.o";
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
    
    // Build test_helper.o
    const char *helper_obj = OUTPUT_DIR "/test_helper.o";
    const char *helper_inputs[] = {TESTS_DIR "/test_helper.cpp", TESTS_DIR "/test_helper.hpp",
                                    std_pcm_path(release), std_compat_pcm_path(release)};
    if (nob_needs_rebuild(helper_obj, helper_inputs, 4) > 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-c", TESTS_DIR "/test_helper.cpp", "-o", helper_obj);
        
        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, TESTS_DIR "/test_helper.cpp", .output = helper_obj);
        
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    if (!build_unity_object(release, true)) return false;
    
    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;
    
    const char *unity_obj = OUTPUT_DIR "/all_test.o";
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0;
    
    nob_da_foreach(const char*, src, &tests) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);
        
        const char *inputs[] = {*src, unity_obj, helper_obj, catch_obj};
        if (nob_needs_rebuild(exe, inputs, 4) <= 0) {
            skipped++;
            continue;
        }
        built++;
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, *src, unity_obj, helper_obj, catch_obj);
        cmd_add_sdl2_libs(&cmd);
        
        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }
    
    if (!nob_procs_wait(procs)) return false;
    
    nob_log(INFO, "Tests: %zu built, %zu skipped", built, skipped);
    return true;
}

static bool run_tests(void) {
    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;
    
    nob_da_foreach(const char*, src, &tests) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);
        
        nob_log(INFO, "Running: %s", name);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, exe);
        if (!nob_cmd_run(&cmd, .dont_reset = true)) {
            nob_log(ERROR, "Test %s failed", name);
            return false;
        }
    }
    
    nob_log(INFO, "All tests passed");
    return true;
}

// ========== RELEASE ==========

static const char * hpp_order[] = {
    "src/easing.hpp",
    "src/sdl_deleters.hpp",
    "src/layout_parser.hpp",
    "src/sgml_parser.hpp",
    "src/json_parser.hpp",
    "src/timer_manager.hpp",
    "src/animation_manager.hpp",
    "src/style.hpp",
    "src/font_manager.hpp",
    "src/texture_manager.hpp",
    "src/theme.hpp",
    "src/gui.hpp",
    "src/label.hpp",
    "src/panel.hpp",
    "src/button.hpp",
    "src/checkbox.hpp",
    "src/slider.hpp",
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
    "src/gui_manager.hpp",
    "src/sdl_app.hpp",
    "src/composite/dialog_box.hpp",
    "src/composite/message_box.hpp",
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
    "#include \"gui.hpp\"",
    "#include \"label.hpp\"",
    "#include \"panel.hpp\"",
    "#include \"button.hpp\"",
    "#include \"checkbox.hpp\"",
    "#include \"slider.hpp\"",
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
};

static bool line_should_remove(const char *line) {
    if (strncmp(line, "#pragma once", 12) == 0) return true;
    if (strncmp(line, "#include <", 10) == 0) return true;
    if (strncmp(line, "#include \"SDL2/", 15) == 0) return true;
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
    nob_sb_append_cstr(&sb, "// C++ Standard Library\n");
    nob_sb_append_cstr(&sb, "#include <cmath>\n#include <functional>\n#include <iostream>\n");
    nob_sb_append_cstr(&sb, "#include <map>\n#include <memory>\n#include <numeric>\n");
    nob_sb_append_cstr(&sb, "#include <optional>\n#include <string>\n#include <string_view>\n");
    nob_sb_append_cstr(&sb, "#include <variant>\n#include <vector>\n\n");
    nob_sb_append_cstr(&sb, "// External libraries\n");
    nob_sb_append_cstr(&sb, "#include <SDL2/SDL.h>\n#include <SDL2/SDL_image.h>\n");
    nob_sb_append_cstr(&sb, "#include <SDL2/SDL_ttf.h>\n#include <SDL2/SDL_pixels.h>\n");
    nob_sb_append_cstr(&sb, "#include <SDL2/SDL_log.h>\n#include <SDL2/SDL2_gfxPrimitives.h>\n\n");
    nob_sb_append_cstr(&sb, "// Project libraries\n#include \"tinyxml2.h\"\n\n");
    
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
            char *copy = nob_temp_strndup(line, len);
            
            if (!line_should_remove(copy)) {
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

// Release: kompiluj tylko .pic.o (używane dla .a i .so)
static bool build_release(void) {
    nob_mkdir_if_not_exists(OUTPUT_DIR "/release");
    nob_mkdir_if_not_exists(OUTPUT_DIR "/release/composite");
    nob_mkdir_if_not_exists(DIST_DIR);
    init_globals();
    
    if (!build_modules(true)) return false;
    
    Nob_File_Paths sources = {0};
    Nob_File_Paths composite = {0};
    Nob_File_Paths editor = {0};
    if (!collect_cpp_sources(&sources)) return false;
    if (!collect_composite_sources(&composite)) return false;
    if (!collect_editor_sources(&editor)) return false;
    
    Nob_File_Paths objects = {0};  // .pic.o paths (used for .a and .so)
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0;
    
    // Compile src/*.cpp -> .pic.o (parallel)
    nob_da_foreach(const char*, src, &sources) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/release/%s.pic.o", OUTPUT_DIR, name);
        nob_da_append(&objects, nob_temp_strdup(obj));
        
        // Rebuild check
        Nob_File_Paths deps = {0};
        nob_da_append(&deps, *src);
        collect_hpp_sources(&deps);
        collect_files(&deps, SRC_DIR "/composite", ".hpp");
        nob_da_append(&deps, std_pcm_path(true));
        nob_da_append(&deps, std_compat_pcm_path(true));
        
        if (nob_needs_rebuild(obj, deps.items, deps.count) <= 0) {
            skipped++;
            continue;
        }
        built++;
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_modules(&cmd, true);
        nob_cmd_append(&cmd, "-fPIC", "-c", *src, "-o", obj);
        
        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }
    
    // Compile src/composite/*.cpp -> .pic.o (parallel)
    nob_da_foreach(const char*, src, &composite) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/release/composite/%s.pic.o", OUTPUT_DIR, name);
        nob_da_append(&objects, nob_temp_strdup(obj));
        
        Nob_File_Paths deps = {0};
        nob_da_append(&deps, *src);
        collect_hpp_sources(&deps);
        collect_files(&deps, SRC_DIR "/composite", ".hpp");
        nob_da_append(&deps, std_pcm_path(true));
        nob_da_append(&deps, std_compat_pcm_path(true));
        
        if (nob_needs_rebuild(obj, deps.items, deps.count) <= 0) {
            skipped++;
            continue;
        }
        built++;
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_modules(&cmd, true);
        nob_cmd_append(&cmd, "-fPIC", "-c", *src, "-o", obj);
        
        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }
    
    // Compile tinyxml2.cpp -> .pic.o
    const char *tinyxml2_obj = OUTPUT_DIR "/release/tinyxml2.pic.o";
    nob_da_append(&objects, nob_temp_strdup(tinyxml2_obj));
    const char *tinyxml2_inputs[] = {LIB_DIR "/tinyxml2.cpp", LIB_DIR "/tinyxml2.h"};
    if (nob_needs_rebuild(tinyxml2_obj, tinyxml2_inputs, 2) > 0) {
        built++;
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        cmd_add_modules(&cmd, true);
        nob_cmd_append(&cmd, "-fPIC", "-c", LIB_DIR "/tinyxml2.cpp", "-o", tinyxml2_obj);
        
        // Add to compile_commands.json
        nob_compdb_add(&g_compdb, &cmd, LIB_DIR "/tinyxml2.cpp", .output = tinyxml2_obj);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    } else {
        skipped++;
    }
    
    if (!nob_procs_wait(procs)) return false;
    nob_log(INFO, "Release objects: %zu built, %zu skipped", built, skipped);
    
    // Build static library (.a)
    const char *lib_a = DIST_DIR "/libsdl_gui.a";
    if (nob_needs_rebuild(lib_a, objects.items, objects.count) > 0) {
        nob_log(INFO, "Building static library: %s", lib_a);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "ar", "rcs", lib_a);
        nob_da_foreach(const char*, obj, &objects) {
            nob_cmd_append(&cmd, *obj);
        }
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    // Build shared library (.so)
    const char *lib_so = DIST_DIR "/libsdl_gui.so";
    if (nob_needs_rebuild(lib_so, objects.items, objects.count) > 0) {
        nob_log(INFO, "Building shared library: %s", lib_so);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, true);
        nob_cmd_append(&cmd, "-shared", "-o", lib_so);
        nob_da_foreach(const char*, obj, &objects) {
            nob_cmd_append(&cmd, *obj);
        }
        cmd_add_sdl2_libs(&cmd);
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    if (!build_combined_header()) return false;
    
    nob_log(INFO, "Release finished. Artifacts in %s/", DIST_DIR);
    return true;
}

// ========== NON-UNITY ==========

static bool build_non_unity(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    
    if (!build_modules(release)) return false;
    
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
        const char *obj = nob_temp_sprintf("%s/%s.o", OUTPUT_DIR, name);
        
        nob_log(INFO, "Compiling: %s", *src);
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
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
        const char *obj = nob_temp_sprintf("%s/composite_%s.o", OUTPUT_DIR, name);
        
        nob_log(INFO, "Compiling: %s", *src);
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);
        
        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);
        
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    nob_da_foreach(const char*, src, &editor) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *obj = nob_temp_sprintf("%s/editor_%s.o", OUTPUT_DIR, name);
        
        nob_log(INFO, "Compiling: %s", *src);
        
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-c", *src, "-o", obj);
        
        nob_compdb_add(&g_compdb, &cmd, *src, .output = obj);
        
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    nob_log(INFO, "Non-unity finished");
    return true;
}

// ========== CLEAN ==========

static void clean(void) {
    nob_log(INFO, "Cleaning...");
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "rm", "-rf", OUTPUT_DIR, DIST_DIR, MODULE_CACHE_DIR);
    nob_cmd_run(&cmd);
    nob_log(INFO, "Cleaned");
}

// ========== MAIN ==========

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    init_globals();
    
    const char *target = "examples";
    bool release = false;
    
    if (argc > 1) {
        const char *arg = argv[1];
        if (strcmp(arg, "test") == 0) target = "test";
        else if (strcmp(arg, "release") == 0) { target = "release"; release = true; }
        else if (strcmp(arg, "clean") == 0) target = "clean";
        else if (strcmp(arg, "non_unity") == 0) target = "non_unity";
        else if (strcmp(arg, "examples") == 0) target = "examples";
        else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--release") == 0) {
            release = true;
            if (argc > 2) target = argv[2];
        } else {
            nob_log(ERROR, "Unknown target: %s", arg);
            nob_log(INFO, "Targets: examples, test, release, clean, non_unity");
            return 1;
        }
    }
    
    bool ok = false;
    if (strcmp(target, "examples") == 0) ok = build_examples(release);
    else if (strcmp(target, "test") == 0) { ok = build_tests(release); if (ok) ok = run_tests(); }
    else if (strcmp(target, "release") == 0) ok = build_release();
    else if (strcmp(target, "clean") == 0) { clean(); ok = true; }
    else if (strcmp(target, "non_unity") == 0) ok = build_non_unity(release);
    
    // Save compile_commands.json if there were compilations
    if (ok && g_compdb.count > 0) {
        nob_compdb_save(&g_compdb, "compile_commands.json");
        nob_log(INFO, "Generated compile_commands.json (%zu entries)", g_compdb.count);
    }
    // No need to free - program exits
    
    return ok ? 0 : 1;
}