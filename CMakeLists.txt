cmake_minimum_required(VERSION 3.31.0)

project(Wunixsh VERSION 0.1.1 LANGUAGES C)          

add_executable(Wunixsh src/shell.c src/commands.c src/main.h)

find_library(PSAPI_LIBRARY Psapi)

target_link_libraries(Wunixsh Psapi)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy "${PROJECT_BINARY_DIR}/Wunixsh.exe" "${CMAKE_CURRENT_SOURCE_DIR}/Wunixsh.exe"
)