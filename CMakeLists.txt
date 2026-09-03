cmake_minimum_required(VERSION 3.26)

project(Game LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(Game
    src/main.c
    src/Game.c
    src/Utils.c
    src/Assets.c
)

target_include_directories(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# raylib
target_include_directories(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib/include)
target_link_directories(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib/lib)
target_link_libraries(Game PRIVATE
	raylib
	winmm
)

target_compile_options(Game PRIVATE
	-Wall
	-Wextra
	-Wpedantic
)