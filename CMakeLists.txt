cmake_minimum_required(VERSION 3.16)
project(sdlscroller C)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(tmx REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_executable(sdlscroller
        "src/main.c"
        "src/engine/engine.h"
        "src/engine/engine.c"
        "src/engine/game.c"
        "src/engine/tileset.c"
        "src/engine/background_map.c"
        "src/engine/collision_map.c"
        "src/engine/timer.c"
        "src/engine/system.c"
        "src/engine/animation.c"
        "src/engine/animation_sheet.c"
        "src/engine/entity.c"
        src/engine/texture.c src/engine/input.c src/entities/player.c src/entities/player.h src/entities/npc.h src/entities/npc.c src/libs/list/list.h src/libs/list/list.c)

target_link_libraries(sdlscroller ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARY} ${GLEW_LIBRARIES} tmx m)

