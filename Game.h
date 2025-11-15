#pragma once

#include "Board.h"
#include <thread>
#include <mutex>
#include <condition_variable>
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
    std::mutex p1_mutex, p2_mutex, render_mutex;

    std::mutex p1_garbage_mutex;
    std::condition_variable p1_garbage_cv;
    int p1_garbage_count{0};

    // Para o lixo do Jogador 2
    std::mutex p2_garbage_mutex;
    std::condition_variable p2_garbage_cv;
    int p2_garbage_count{0};

    // Filas de Input
    std::queue<int> p1_input_queue;
    std::queue<int> p2_input_queue;
    std::mutex p1_input_mutex, p2_input_mutex;

    // As 4 Threads
    std::thread t_input;
    std::thread t_render;
    std::thread t_player1;
    std::thread t_player2;

    void input_loop();
    void render_loop();
    void player_loop(int player_id);
};