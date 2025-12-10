cmake_minimum_required(VERSION 3.21)
project(w86-tests ASM)

add_executable(hello "hello.S")
set_target_properties(hello PROPERTIES SUFFIX ".bin" LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")
target_link_options(hello PRIVATE "-nostdlib" "-T" "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")

add_executable(fibonacci "fibonacci.S")
set_target_properties(fibonacci PROPERTIES SUFFIX ".bin" LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")
target_link_options(fibonacci PRIVATE "-nostdlib" "-T" "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")

add_executable(echo "echo.S")
set_target_properties(echo PROPERTIES SUFFIX ".bin" LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")
target_link_options(echo PRIVATE "-nostdlib" "-T" "${CMAKE_CURRENT_SOURCE_DIR}/test.ld")
