#define NOB_IMPLEMENTATION
#include "nob.h"

#include <stdlib.h>
#include <string.h>

#define CXX "clang++-24"
#define CC "clang-24"

#define SRC_DIR "src"
#define OUTPUT_DIR "output"
#define DIST_DIR "dist"
#define TESTS_DIR "tests"
#define BENCH_DIR "bench"
#define LIB_DIR "lib"
#define MODULE_CACHE_DIR "modules_cache"
#define EXAMPLES_DIR "examples"

#define STD_MODULE_SRC "/usr/lib/llvm-24/share/libc++/v1/std.cppm"
#define STD_COMPAT_MODULE_SRC "/usr/lib/llvm-24/share/libc++/v1/std.compat.cppm"

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
    "-stdlib=libc++",
    "-std=c++23",
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

// Global precompiled commands (initialized once)
static Nob_Cmd g_common = {0};
static Nob_Cmd g_debug = {0};
static Nob_Cmd g_release = {0};
static Nob_Cmd g_sdl3_cflags = {0};  // SDL3 compile flags from pkg-config
static Nob_Cmd g_sdl3_libs = {0};    // SDL3 link flags from pkg-config
static Nob_Cmd g_mixer_cflags = {0}; // SDL3_mixer flags, only for *mixer* examples
static Nob_Cmd g_mixer_libs = {0};
static Nob_Compdb g_compdb = {0};    // compile_commands.json entries
static bool g_initialized = false;

static void pkg_config_cmd(Nob_Cmd *cmd, const char *args) {
    char buffer[8192] = {0};
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd),
             "PKG_CONFIG_PATH=/usr/local/lib/pkgconfig pkg-config %s 2>/dev/null", args);
    FILE *fp = popen(full_cmd, "r");
    if (!fp) return;
    size_t n = fread(buffer, 1, sizeof(buffer) - 1, fp);
    pclose(fp);
    if (n == 0) return;
    buffer[n] = '\0';
    char *rest = buffer;
    char *token;
    while ((token = strtok_r(rest, " \t\n", &rest))) {
        nob_cmd_append(cmd, nob_temp_strdup(token));
    }
}

static void init_globals(void) {
    if (g_initialized) return;
    nob_da_append_many(&g_common, common_flags, NOB_ARRAY_LEN(common_flags));
    nob_da_append_many(&g_debug, debug_flags, NOB_ARRAY_LEN(debug_flags));
    nob_da_append_many(&g_release, release_flags, NOB_ARRAY_LEN(release_flags));
    pkg_config_cmd(&g_sdl3_cflags, "sdl3 sdl3-image sdl3-ttf --cflags");
    pkg_config_cmd(&g_sdl3_libs, "sdl3 sdl3-image sdl3-ttf --libs");
    pkg_config_cmd(&g_mixer_cflags, "sdl3-mixer --cflags");
    pkg_config_cmd(&g_mixer_libs, "sdl3-mixer --libs");
    g_initialized = true;
}

// ========== HELPER FUNCTIONS ==========

static void cmd_add_common(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_common);
}

static void cmd_add_mode(Nob_Cmd *cmd, bool release) {
    nob_cmd_extend(cmd, release ? &g_release : &g_debug);
}

static void cmd_add_sdl3(Nob_Cmd *cmd) {
    nob_cmd_extend(cmd, &g_sdl3_cflags);
    nob_cmd_extend(cmd, &g_sdl3_libs);
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

// ========== EMBEDDED ASSETS ==========

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
// .c file (arrays + sizes) compiled with the regular C compiler. Works with
// any clang target (Linux, MinGW cross, native Windows) — no COFF/ELF
// symbol-prefix issues like with `ld -r -b binary`.
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
            nob_log(NOB_WARNING, "Embedded asset missing: %s — skipping embedded assets (example 32_embedded_assets will not build)",
                    g_embedded_assets[i].path);
            nob_da_free(all_inputs);
            return true;
        }
    }

    const char *data_c_path = OUTPUT_DIR "/embedded_assets_data.c";
    const char *obj_path = OUTPUT_DIR "/embedded_assets_data.o";
    const char *header_path = SRC_DIR "/embedded_assets.hpp";

    int data_needs = nob_needs_rebuild(data_c_path, all_inputs.items, all_inputs.count);
    nob_da_free(all_inputs);
    if (data_needs < 0) return false;

    if (data_needs > 0 || !nob_file_exists(header_path)) {
        nob_log(NOB_INFO, "Generating embedded assets data: %s", data_c_path);
        Nob_String_Builder data = {0};
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

// ========== GPU SHADERS ==========

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
// way as assets: generated .c with byte arrays, no `ld -r -b binary`.
// glslc failure / missing sources -> WARNING + g_have_shaders=false
// (not fatal); examples 40/41_gpu_shader* are then skipped.
// The generated header keeps the exact public names the examples use
// (gpu_shader::<name>, gpu_shader::<name>_size).
static bool build_gpu_shaders(Nob_File_Paths *shader_objects, bool release) {
    (void)release;
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    g_have_shaders = false;

    size_t count = NOB_ARRAY_LEN(g_gpu_shaders);
    if (count == 0) return true;

    // Step 1: Compile GLSL to SPIR-V via glslc
    for (size_t i = 0; i < count; i++) {
        if (!nob_file_exists(g_gpu_shaders[i].path)) {
            nob_log(NOB_WARNING, "Shader source missing: %s — skipping GPU shaders (examples 40/41 will not build)",
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
            nob_log(NOB_WARNING, "glslc failed for %s — skipping GPU shaders (examples 40/41 will not build). "
                    "Install glslc (Vulkan SDK / shaderc) to enable them.", g_gpu_shaders[i].path);
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
            nob_log(NOB_WARNING, "SPIR-V file missing: %s — skipping GPU shaders", spv_inputs.items[i]);
            nob_da_free(spv_inputs);
            return true;
        }
    }

    const char *data_c_path = OUTPUT_DIR "/shader_spirv_data.c";
    const char *obj_path = OUTPUT_DIR "/shader_spirv_data.o";
    const char *header_path = OUTPUT_DIR "/gpu_shader_spirv.hpp";

    int data_needs = nob_needs_rebuild(data_c_path, spv_inputs.items, spv_inputs.count);
    nob_da_free(spv_inputs);
    if (data_needs < 0) return false;

    if (data_needs > 0 || !nob_file_exists(header_path)) {
        nob_log(INFO, "Generating GPU shader data: %s", data_c_path);
        Nob_String_Builder data = {0};
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

        // Step 3: Generate header (same public names as before)
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

// Optional build inputs. Missing assets / glslc / third-party libs are not
// fatal: examples that need them are skipped with an INFO log.
typedef enum {
    NEED_NONE     = 0,
    NEED_EMBEDDED = 1 << 0,  // src/embedded_assets.hpp + embedded data .o
    NEED_SHADERS  = 1 << 1,  // output/gpu_shader_spirv.hpp + SPIR-V data .o + glslc
    NEED_MIXER    = 1 << 2,  // SDL3_mixer (pkg-config sdl3-mixer)
    NEED_SQLITE   = 1 << 3,  // -lsqlite3
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
    return NULL;  // mixer/sqlite stay fatal (link error) as before
}

// ========== EXAMPLES ==========

static bool build_examples(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();
    
    if (!build_unity_object(release, false)) return false;
    
    // Build embedded assets
    Nob_File_Paths embedded_objects = {0};
    if (!build_embedded_assets(&embedded_objects, release)) return false;
    
    // Build GPU shaders
    Nob_File_Paths shader_objects = {0};
    if (!build_gpu_shaders(&shader_objects, release)) return false;
    
    Nob_File_Paths examples = {0};
    if (!collect_example_sources(&examples)) return false;
    
    const char *unity_obj = OUTPUT_DIR "/all.o";
    Nob_Procs procs = {0};
    size_t built = 0, skipped = 0, skipped_no_deps = 0;

    nob_da_foreach(const char*, src, &examples) {
        const char *basename = nob_path_name(*src);
        char *name = nob_temp_strdup(basename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);

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
        cmd_add_modules(&cmd, release);
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
        // Opt-in third-party deps per example (keeps other binaries lean).
        if ((needs & NEED_MIXER) != 0) {
            nob_cmd_extend(&cmd, &g_mixer_cflags);
            nob_cmd_extend(&cmd, &g_mixer_libs);
        }
        if ((needs & NEED_SQLITE) != 0) {
            nob_cmd_append(&cmd, "-lsqlite3");
        }

        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);

        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }

    if (!nob_procs_wait(procs)) return false;

    nob_log(INFO, "Examples: %zu built, %zu skipped, %zu skipped (missing optional deps)",
            built, skipped, skipped_no_deps);
    return true;
}

// ========== TESTS ==========

static bool build_tests(bool release, const char *filter) {
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

    // NOTE: tests never touch embedded assets or GPU SPIR-V (ShaderPanel
    // tests use synthetic bytes), so optional objects are neither built
    // nor linked here — `test` stays green without assets/glslc.

    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;
    
    const char *unity_obj = OUTPUT_DIR "/all_test.o";
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
        
        const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);

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
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, *src, unity_obj, helper_obj, catch_obj);
        cmd_add_sdl3(&cmd);
        
        // Add to compile_commands.json BEFORE running
        nob_compdb_add(&g_compdb, &cmd, *src, .output = exe);
        
        if (!nob_cmd_run(&cmd, .async = &procs)) return false;
    }
    
    if (!nob_procs_wait(procs)) return false;
    
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
    return procs > 16 ? 16 : procs; // ASAN is memory-hungry; cap at 16
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
static void start_test(TestRuns *runs, const char *logs_dir, const char *name,
                       size_t *failed) {
    const char *exe = nob_temp_sprintf("%s/%s", OUTPUT_DIR, name);
    const char *log_path = nob_temp_sprintf("%s/%s.log", logs_dir, name);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "env",
                   "ASAN_OPTIONS=detect_container_overflow=0:detect_leaks=0",
                   "SDL_GUI_HIDDEN=1", exe);

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

static bool run_tests(const char *filter) {
    Nob_File_Paths tests = {0};
    if (!collect_test_sources(&tests)) return false;

    const char *logs_dir = OUTPUT_DIR "/test_logs";
    if (!nob_mkdir_if_not_exists(logs_dir)) return false;

    // Collect the names we are going to run (filtered)
    Nob_File_Paths pending = {0};
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
        nob_da_append(&pending, strdup(name));
    }

    const size_t jobs = test_jobs();
    nob_log(INFO, "Running tests (jobs=%zu, logs in %s/)", jobs, logs_dir);

    TestRuns runs = {0};
    size_t passed = 0, failed = 0;
    size_t next = 0;

    // Dynamic work queue: keep up to `jobs` tests running; as soon as one
    // finishes, start the next pending test.
    while (next < pending.count || runs.count > 0) {
        while (runs.count < jobs && next < pending.count) {
            start_test(&runs, logs_dir, pending.items[next], &failed);
            next++;
        }

        bool progressed = false;
        size_t i = 0;
        while (i < runs.count) {
            int res = nob__proc_wait_async(runs.items[i].proc, 0);
            if (res == 1 || res == -1) {
                progressed = true;
                if (res == -1) {
                    failed++;
                    nob_log(ERROR, "Test %s FAILED (log: %s)", runs.items[i].name, runs.items[i].log_path);
                    print_tail(runs.items[i].log_path, 60);
                } else {
                    passed++;
                    nob_log(INFO, "Test %s passed", runs.items[i].name);
                }
                runs.items[i] = runs.items[runs.count - 1];
                runs.count--;
            } else {
                i++;
            }
        }

        if (!progressed && runs.count > 0) {
            usleep(5000); // all procs still running; avoid a busy spin
        }
    }

    if (skipped > 0) {
        nob_log(INFO, "Tests: %zu passed, %zu failed, %zu skipped (of %zu)", passed, failed, skipped, tests.count);
    } else {
        nob_log(INFO, "Tests: %zu passed, %zu failed (of %zu)", passed, failed, tests.count);
    }
    return failed == 0;
}

// ========== RELEASE ==========

// All public headers are merged into a single sdl_gui.hpp.
// Order matters (dependencies). Supporting files (std.hpp, logger.hpp,
// constants.hpp, sdl_rect_helpers.hpp, tinyxml2.h) are INLINEd, so that
// dist/ is fully self-contained — the user doesn't need src/ or lib/.
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
    "#include \"modal_dialog.hpp\"",
    "#include \"file_dialog.hpp\"",
};

// Supporting files whose system includes must remain in the merged
// header (std.hpp is de facto a list of includes; tinyxml2.h/logger.hpp etc.
// need their dependencies, and the user has no access to src/ or lib/).
static bool is_support_header(const char *path) {
    return strstr(path, "tinyxml2.h") != NULL
        || strstr(path, "std.hpp") != NULL
        || strstr(path, "logger.hpp") != NULL
        || strstr(path, "constants.hpp") != NULL
        || strstr(path, "sdl_rect_helpers.hpp") != NULL;
}

// std.hpp has two branches: #ifdef __clangd__ (traditional includes) and #else
// (import std.compat, requires -fmodule-file). In release we do NOT remove the
// module branch — we remove the whole conditional construct and keep only the
// includes, so that the header works for the user without precompiled modules.
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

// Release: compile only .pic.o (used for .a and .so)
static bool build_release(void) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
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
        cmd_add_sdl3(&cmd);
        if (!nob_cmd_run(&cmd)) return false;
    }
    
    if (!build_combined_header()) return false;

    // Copy C API header to dist/
    {
        const char *c_header_src  = SRC_DIR "/sdl_gui.h";
        const char *c_header_dst  = DIST_DIR "/sdl_gui.h";
        const char *c_header_deps[] = {c_header_src};
        if (nob_needs_rebuild(c_header_dst, c_header_deps, 1) > 0) {
            nob_log(INFO, "Copying C API header: %s -> %s", c_header_src, c_header_dst);
            Nob_Cmd cmd = {0};
            nob_cmd_append(&cmd, "cp", c_header_src, c_header_dst);
            if (!nob_cmd_run(&cmd)) return false;
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
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "rm", "-rf", DIST_DIR "/docs");
        if (!nob_cmd_run(&cmd)) return false;
        nob_cmd_append(&cmd, "cp", "-r", docs_src, DIST_DIR "/docs");
        if (!nob_cmd_run(&cmd)) return false;
    }

    // Copy the agent skill to dist/skills/ so downstream projects (e.g. a game
    // built on this library) can install it with:
    //   cp -r <sdk>/skills/sdl-gui <game>/.kilo/skills/
    {
        const char *skill_src = "skills/sdl-gui";
        const char *skill_marker = nob_temp_sprintf("%s/%s", skill_src, "SKILL.md");
        if (!nob_file_exists(skill_marker)) {
            nob_log(ERROR, "Missing skills/sdl-gui/SKILL.md");
            return false;
        }
        nob_log(INFO, "Copying agent skill: %s -> %s/skills/", skill_src, DIST_DIR);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "rm", "-rf", DIST_DIR "/skills");
        if (!nob_cmd_run(&cmd)) return false;
        nob_cmd_append(&cmd, "mkdir", "-p", DIST_DIR "/skills");
        if (!nob_cmd_run(&cmd)) return false;
        nob_cmd_append(&cmd, "cp", "-r", skill_src, DIST_DIR "/skills/sdl-gui");
        if (!nob_cmd_run(&cmd)) return false;
    }

    // Smoke test: compile standalone example with combined header + static library
    {
        const char *standalone_src = EXAMPLES_DIR "/47_standalone.cpp";
        const char *standalone_exe = OUTPUT_DIR "/47_standalone";
        const char *standalone_inputs[] = {
            standalone_src,
            DIST_DIR "/sdl_gui.hpp",
            DIST_DIR "/libsdl_gui.a",
            std_pcm_path(true),
            std_compat_pcm_path(true),
        };
        if (nob_needs_rebuild(standalone_exe, standalone_inputs, 5) > 0) {
            nob_log(INFO, "Building standalone example (release sanity check)...");
            Nob_Cmd cmd = {0};
            nob_cmd_append(&cmd, CXX);
            cmd_add_common(&cmd);
            cmd_add_mode(&cmd, true);
            nob_cmd_append(&cmd,
                nob_temp_sprintf("-fmodule-file=std=%s", std_pcm_path(true)),
                nob_temp_sprintf("-fmodule-file=std.compat=%s", std_compat_pcm_path(true)));
            // Only -I dist: proves that sdl_gui.hpp is self-contained
            // (the user doesn't need src/ or lib/).
            nob_cmd_append(&cmd, "-I" DIST_DIR);
            nob_cmd_append(&cmd, "-o", standalone_exe, standalone_src, DIST_DIR "/libsdl_gui.a");
        cmd_add_sdl3(&cmd);
            if (!nob_cmd_run(&cmd)) return false;
        }
    }

    // C API smoke test: compile a pure C file with gcc against dist/sdl_gui.h + libsdl_gui.a
    {
        const char *c_test_src  = OUTPUT_DIR "/release/c_smoke_test.c";
        const char *c_test_exe  = OUTPUT_DIR "/release/c_smoke_test";
        const char *c_header    = DIST_DIR "/sdl_gui.h";
        const char *c_lib       = DIST_DIR "/libsdl_gui.so";
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
            /* Compile C source separately, then link with g++ for C++ runtime */
            const char *c_obj = nob_temp_sprintf("%s.o", c_test_exe);
            Nob_Cmd cc_cmd = {0};
            nob_cmd_append(&cc_cmd, CC, "-std=c11", "-pedantic-errors",
                "-I" DIST_DIR,
                "-c", c_test_src, "-o", c_obj);
            nob_cmd_extend(&cc_cmd, &g_sdl3_cflags);
            if (!nob_cmd_run(&cc_cmd)) return false;

            Nob_Cmd link_cmd = {0};
            nob_cmd_append(&link_cmd, CXX);
            cmd_add_common(&link_cmd);
            nob_cmd_append(&link_cmd,
                "-o", c_test_exe, c_obj, c_lib);
            nob_cmd_extend(&link_cmd, &g_sdl3_cflags);
            nob_cmd_extend(&link_cmd, &g_sdl3_libs);
            if (!nob_cmd_run(&link_cmd)) return false;
        }
    }

    // Build C examples (pure C source compiled with gcc, linked with g++)
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
                const char *exe = nob_temp_sprintf("%s/c_%s", OUTPUT_DIR, name);

                const char *c_header = DIST_DIR "/sdl_gui.h";
                const char *c_lib    = DIST_DIR "/libsdl_gui.so";
                const char *inputs[] = {src, c_header, c_lib};
                if (nob_needs_rebuild(exe, inputs, 3) <= 0) continue;

                nob_log(INFO, "Building C example: %s", name);
                const char *c_obj = nob_temp_sprintf("%s/%s_c.o", OUTPUT_DIR, name);
                Nob_Cmd cc_cmd = {0};
                nob_cmd_append(&cc_cmd, CC, "-std=c11", "-pedantic-errors",
                    "-I" DIST_DIR,
                    "-c", src, "-o", c_obj);
                nob_cmd_extend(&cc_cmd, &g_sdl3_cflags);
                if (!nob_cmd_run(&cc_cmd)) {
                    nob_dir_entry_close(entry);
                    return false;
                }
                Nob_Cmd link_cmd = {0};
                nob_cmd_append(&link_cmd, CXX);
                cmd_add_common(&link_cmd);
                nob_cmd_append(&link_cmd,
                    "-o", exe, c_obj, c_lib);
                nob_cmd_extend(&link_cmd, &g_sdl3_cflags);
                nob_cmd_extend(&link_cmd, &g_sdl3_libs);
                if (!nob_cmd_run(&link_cmd)) {
                    nob_dir_entry_close(entry);
                    return false;
                }
            }
            nob_dir_entry_close(entry);
            /* Note: entry.error stays false after close if all was well */
        }
    }

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

// ========== BENCH ==========

// Synthetic GUI benchmark on a REAL window (see bench/bench_gui.cpp).
// Builds output/bench_gui; run it directly, e.g.:
//   ./output/bench_gui --widgets 200 --seconds 5
//   ./output/bench_gui --sweep --seconds 5 --save bench_baseline.json
//   ./output/bench_gui --compare bench_baseline.json
static bool build_bench(bool release) {
    nob_mkdir_if_not_exists(OUTPUT_DIR);
    init_globals();

    if (!build_unity_object(release, false)) return false;

    // NOTE: bench_gui.cpp uses no embedded assets or GPU shaders.
    const char *src = BENCH_DIR "/bench_gui.cpp";
    const char *exe = OUTPUT_DIR "/bench_gui";
    const char *unity_obj = OUTPUT_DIR "/all.o";

    Nob_File_Paths inputs = {0};
    nob_da_append(&inputs, src);
    nob_da_append(&inputs, unity_obj);

    if (nob_needs_rebuild(exe, inputs.items, inputs.count) > 0) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, CXX);
        cmd_add_common(&cmd);
        cmd_add_mode(&cmd, release);
        cmd_add_modules(&cmd, release);
        nob_cmd_append(&cmd, "-o", exe, src, unity_obj);
        cmd_add_sdl3(&cmd);

        nob_compdb_add(&g_compdb, &cmd, src, .output = exe);

        nob_da_free(inputs);
        if (!nob_cmd_run(&cmd)) return false;
    } else {
        nob_da_free(inputs);
    }

    nob_log(INFO, "Bench ready: %s", exe);
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