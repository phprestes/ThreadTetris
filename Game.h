#pragma once

#include "Board.h"
#include <thread>
#include <semaphore>
#include <atomic>
#include <queue>
#include <ncurses.h>

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // Ncurses
    void init_curses();
    void cleanup_curses();
    WINDOW* p1_win;
    WINDOW* p2_win;
    WINDOW* score_win;

    // Estado do Jogo
    Board p1_board;
    Board p2_board;
    std::atomic<bool> game_over;
    int p1_score, p2_score;

    // Sincronização
    std::binary_semaphore p1_board_sem{1};
    std::binary_semaphore p2_board_sem{1};

    // Para o lixo dos jogadores
    std::counting_semaphore<100> p1_garbage_sem{0};
    std::counting_semaphore<100> p2_garbage_sem{0};

    // Filas de Input
    std::queue<int> p1_input_queue;
    std::queue<int> p2_input_queue;

    std::binary_semaphore p1_input_sem{1}; // Inicializado com 1
    std::binary_semaphore p2_input_sem{1}; // Inicializado com 1

    // As 4 Threads
    std::thread t_input;
    std::thread t_render;
    std::thread t_player1;
    std::thread t_player2;

    void input_loop();
    void render_loop();
    void player_loop(int player_id);
};