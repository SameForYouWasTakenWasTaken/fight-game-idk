cmake_minimum_required(VERSION 3.14)
project("Game")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main 
    "src/main.cpp"
    "include/Game.h"
    "include/NPCs/Player.h"
    "src/Game.cpp"
 "include/NPCs/Entity.h" "src/NPCs/Entity.cpp" "src/NPCs/Player.cpp" "include/Projectiles/Bullet.h" "src/NPCs/Projectiles/Bullet.cpp")
target_include_directories(main PRIVATE "include")

# Dependencies
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(raylib spdlog)

target_link_libraries(main PRIVATE raylib spdlog)

# Copy resources after build
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:main>/resources
)
