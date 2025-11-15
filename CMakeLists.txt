cmake_minimum_required(VERSION 3.16)
project(ConcurrentTetris)

# Requer C++17 (em vez de 20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontra a biblioteca ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Adiciona o executável
add_executable(tetris
    main.cpp
    Game.cpp
    Board.cpp
)

# Linka as bibliotecas
target_link_libraries(tetris ${CURSES_LIBRARY})
target_link_libraries(tetris pthread) # Necessário para threads