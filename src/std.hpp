#pragma once
// #define __clangd__
#ifdef __clangd__
// ── Traditional includes for clangd ────────────────────────────────────
// clangd has poor support for C++20 modules (especially `import std`).
// When clangd parses the code, it sees these traditional headers.
// When compiling with clang++-22, the `#else` branch uses modules.
//
// Includes are comprehensive to cover all std:: entities used across
// the project (src/, examples/, tests/).

// Core types & memory
#include <cstddef>      // size_t, nullptr_t, ptrdiff_t, max_align_t
#include <cstdint>      // int8_t..int64_t, uint8_t..uint64_t, intptr_t
#include <cstdlib>      // std::abs(int/long), std::div, EXIT_SUCCESS/FAILURE
#include <cstring>      // std::strlen, std::strcmp, std::memcpy
#include <cstdio>       // std::printf, std::fprintf, std::snprintf
#include <cassert>      // assert
#include <exception>    // std::exception
#include <stdexcept>    // std::runtime_error, std::invalid_argument
#include <memory>       // std::unique_ptr, std::shared_ptr, std::make_unique
#include <utility>      // std::pair, std::move, std::forward, std::swap
#include <new>          // placement new, std::nothrow

// Strings
#include <string>       // std::string, std::wstring, std::u8string
#include <string_view>  // std::string_view, std::wstring_view
#include <format>       // std::format, std::format_string (C++23)
#include <print>        // std::print, std::println (C++23)
#include <charconv>     // std::to_chars, std::from_chars (C++17)
#include <cctype>       // std::isalpha, std::isdigit, std::tolower
#include <cwctype>      // std::iswalpha, ...

// Containers
#include <vector>       // std::vector
#include <array>        // std::array
#include <map>          // std::map, std::multimap
#include <unordered_map>// std::unordered_map
#include <set>          // std::set, std::multiset
#include <unordered_set>// std::unordered_set
#include <span>         // std::span (C++20)
#include <queue>        // std::queue, std::priority_queue
#include <deque>        // std::deque
#include <list>         // std::list
#include <stack>        // std::stack

// Algorithms & numerics
#include <algorithm>    // std::min, std::max, std::clamp, std::sort, std::find, ...
#include <numeric>      // std::accumulate, std::iota
#include <cmath>        // std::abs(float), std::floor, std::ceil, std::round, std::sqrt, ...
#include <numbers>      // std::numbers::pi (C++20)
#include <random>       // std::random_device, std::mt19937, std::uniform_int_distribution

// I/O & filesystem
#include <iostream>     // std::cin, std::cout, std::cerr
#include <fstream>      // std::ifstream, std::ofstream, std::fstream
#include <sstream>      // std::stringstream, std::ostringstream
#include <filesystem>   // std::filesystem::path, std::filesystem::directory_iterator

// Functional & callbacks
#include <functional>   // std::function, std::bind, std::ref
#include <tuple>        // std::tuple, std::make_tuple, std::tie

// Types & variants
#include <optional>     // std::optional, std::nullopt
#include <variant>      // std::variant, std::visit
#include <any>          // std::any
#include <expected>     // std::expected, std::unexpected (C++23)
#include <compare>      // std::strong_ordering, std::partial_ordering (C++20)

// Iterators & ranges
#include <iterator>     // std::begin, std::end, std::next, std::distance
#include <ranges>       // std::ranges::... (C++20)

// Threading (not used in project core, but included for completeness)
#include <chrono>       // std::chrono::system_clock, std::chrono::milliseconds
#include <thread>       // std::thread
#include <mutex>        // std::mutex, std::lock_guard
#include <atomic>       // std::atomic

#else
// ── Module-based import for compilation ──────────────────────────────
import std.compat;
#endif
