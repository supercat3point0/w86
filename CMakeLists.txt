cmake_minimum_required(VERSION 3.21)
project(w86 C)

if (NOT EMSCRIPTEN)
  message(FATAL_ERROR "This project has to be built with Emscripten!")
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(w86)

target_compile_options(w86 PRIVATE "-Wall" "-Wextra" "-Wpedantic")
target_link_options(w86 PRIVATE "-sEXPORT_ES6" "--emit-tsd" "w86.d.ts")
target_compile_options(w86 PRIVATE "$<$<CONFIG:Debug>:-g3;-Og>")
target_link_options(w86 PRIVATE "$<$<CONFIG:Debug>:-g3;-Og>")
target_compile_options(w86 PRIVATE "$<$<CONFIG:Release>:-O2;-DNDEBUG>")
target_link_options(w86 PRIVATE "$<$<CONFIG:Release>:-O2>")
target_compile_options(w86 PRIVATE "$<$<CONFIG:RelWithDebInfo>:-g3;-O2;-DNDEBUG>")
target_link_options(w86 PRIVATE "$<$<CONFIG:RelWithDebInfo>:-g3;-O2>")
target_compile_options(w86 PRIVATE "$<$<CONFIG:MinSizeRel>:-Oz;-DNDEBUG>")
target_link_options(w86 PRIVATE "$<$<CONFIG:MinSizeRel>:-Oz>")

add_subdirectory("src")
