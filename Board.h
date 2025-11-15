#pragma once

#include <vector>
#include <ncurses.h>

class Board {
public:
    Board();
    void initialize();
    bool is_game_over() const;

    // Métodos de lógica do jogo (chamados pelo player_thread)
    bool check_collision(int piece_x, int piece_y, int rotation) const;
    void spawn_new_piece();
    bool move_piece(int dx, int dy);
    void rotate_piece();
    int fix_piece_and_clear_lines();
    void add_garbage(int lines);
    bool check_collision_on_drop() const;
    
    // Método de renderização (chamado pelo render_thread)
    void draw(WINDOW* win) const;

private:
    std::vector<std::vector<int>> grid;
    bool game_over;

    // Estado da peça atual
    int current_piece_type;
    int current_rotation;
    int current_x, current_y;

    int get_piece_block(int piece_type, int rotation, int x, int y) const;
};