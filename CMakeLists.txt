cmake_minimum_required(VERSION 3.16)
project(ConcurrentTetris)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
find_package(Threads REQUIRED)

add_executable(tetris
    main.cpp
    Game.cpp
    Board.cpp
)

target_link_libraries(tetris 
    ${CURSES_LIBRARIES} 
    Threads::Threads
)