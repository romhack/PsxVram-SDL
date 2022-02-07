cmake_minimum_required(VERSION 3.21)

project(PsxVram-SDL)

# WIN32 -> hide extra console shown, ignored on non-Windows

add_executable (${PROJECT_NAME} WIN32 PsxVram-SDL.c) 

# Make it pixel-perfect on Hi-DPI

if (WIN32)
    set_property(TARGET PsxVram-SDL PROPERTY VS_DPI_AWARE "ON")
endif ()


find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
