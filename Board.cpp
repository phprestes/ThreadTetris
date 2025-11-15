#include "Board.h"
#include "Constants.h"
#include <cstdlib> // para rand()
#include <ctime>   // para time()

// Definição das 7 peças de Tetris (4 rotações cada)
static const int TETROMINOES[7][4][4][4] = {
    // I
    {{{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}},
     {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
     {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}}},
    // O
    {{{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}}},
    // T
    {{{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}}},
    // S
    {{{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}}},
    // Z
    {{{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}}},
    // J
    {{{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
     {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}},
    // L
    {{{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
     {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}}
};

Board::Board() : game_over(false) {
    if (std::rand() == 0) { // Inicializa o seed uma vez
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }
    initialize();
}

void Board::initialize() {
    grid.assign(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0));
    game_over = false;
    spawn_new_piece();
}

/// @brief Obtém o valor de um bloco (0 ou 1) de uma célula específica na matriz 4x4 de uma peça
/// @param piece_type O índice do tipo de peça (0-6) a ser verificado
/// @param rotation O índice da rotação (0-3) a ser verificada
/// @param x A coordenada X local (0-3) dentro da matriz 4x4 da peça
/// @param y A coordenada Y local (0-3) dentro da matriz 4x4 da peça
/// @return Retorna 1 se a célula [y][x] da peça[tipo][rotação] for um bloco, ou 0 se for uma célula vazia
int Board::get_piece_block(int piece_type, int rotation, int x, int y) const {
    return TETROMINOES[piece_type][rotation][y][x];
}

/// @brief Spawna uma peça aleatória
void Board::spawn_new_piece() {
    current_piece_type = std::rand() % 7;
    current_rotation = 0;
    current_x = BOARD_WIDTH / 2 - 2;
    current_y = 0;

    if (check_collision(current_x, current_y, current_rotation)) {
        game_over = true;
    }
}

/// @brief Checa se o jogo terminou
/// @return Booleano se o jogo terminou ou não
bool Board::is_game_over() const {
    return game_over;
}

/// @brief Verifica se a peça atual colidiria com os limites do tabuleiro ou com peças
/// @param piece_x A coordenada x (canto superior esquerdo) da peça para testar
/// @param piece_y A coordenada y (canto superior esquerdo) da peça para testar
/// @param rotation O índice de rotação (0-3) da peça para testar
/// @return Booleano se há uma colisão ou não
bool Board::check_collision(int piece_x, int piece_y, int rotation) const {
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (get_piece_block(current_piece_type, rotation, x, y) != 0) {
                int board_x = piece_x + x;
                int board_y = piece_y + y;

                // Fora dos limites (esquerda, direita, baixo)
                if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT) {
                    return true;
                }
                
                // Colisão com o grid (não checa o topo, y < 0)
                if (board_y >= 0 && grid[board_y][board_x] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

/// @brief Move a peça dado um deslocamento
/// @param dx Deslocamento em x
/// @param dy Deslocamento em y
/// @return Booleano se foi possível mover a peça ou não
bool Board::move_piece(int dx, int dy) {
    if (!check_collision(current_x + dx, current_y + dy, current_rotation)) {
        current_x += dx;
        current_y += dy;
        return true;
    }
    return false;
}

/// @brief Rotaciona a peça
void Board::rotate_piece() {
    int next_rotation = (current_rotation + 1) % 4;
    if (!check_collision(current_x, current_y, next_rotation)) {
        current_rotation = next_rotation;
    }
}

/// @brief Fixa uma peça que estava caindo e limpa as linhas
/// @return Número de linhas que foram limpas
int Board::fix_piece_and_clear_lines() {
    // "Queima" a peça no grid
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (get_piece_block(current_piece_type, current_rotation, x, y) != 0) {
                int board_x = current_x + x;
                int board_y = current_y + y;
                if (board_y >= 0 && board_y < BOARD_HEIGHT) {
                    grid[board_y][board_x] = current_piece_type + 1; // +1 para cor
                }
            }
        }
    }

    // Limpa linhas
    int lines_cleared = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; --y) {
        bool line_full = true;
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (grid[y][x] == 0) {
                line_full = false;
                break;
            }
        }

        if (line_full) {
            lines_cleared++;
            // Move todas as linhas acima para baixo
            for (int r = y; r > 0; --r) {
                grid[r] = grid[r - 1];
            }
            grid[0].assign(BOARD_WIDTH, 0); // Linha nova no topo
            y++; // Checa a mesma linha novamente (agora com novos dados)
        }
    }
    return lines_cleared;
}

/// @brief Adiciona o lixo enviado pelo outro jogador
/// @param lines Número de linhas de lixo que vão ser adicionadas
void Board::add_garbage(int lines) {
    for (int i = 0; i < lines; ++i) {
        // Checa se o topo está ocupado (game over)
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (grid[0][x] != 0) {
                game_over = true;
                return;
            }
        }

        // Move tudo para cima
        for (int y = 0; y < BOARD_HEIGHT - 1; ++y) {
            grid[y] = grid[y + 1];
        }

        // Adiciona linha de lixo (cor 8) com um buraco
        grid[BOARD_HEIGHT - 1].assign(BOARD_WIDTH, 8);
        grid[BOARD_HEIGHT - 1][std::rand() % BOARD_WIDTH] = 0;
    }
}

// ncurses
void Board::draw(WINDOW* win) const {
    werase(win);
    box(win, 0, 0);

    // Desenha o grid
    for (int y = 0; y < BOARD_HEIGHT; ++y) {
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            if (grid[y][x] != 0) {
                wattron(win, COLOR_PAIR(grid[y][x]));
                mvwprintw(win, y + 1, x * 2 + 1, "[]");
                wattroff(win, COLOR_PAIR(grid[y][x]));
            }
        }
    }

    // Desenha a peça atual
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            if (get_piece_block(current_piece_type, current_rotation, x, y) != 0) {
                int board_x = current_x + x;
                int board_y = current_y + y;
                if (board_y >= 0) {
                    wattron(win, COLOR_PAIR(current_piece_type + 1));
                    mvwprintw(win, board_y + 1, board_x * 2 + 1, "[]");
                    wattroff(win, COLOR_PAIR(current_piece_type + 1));
                }
            }
        }
    }
    wrefresh(win);
}

/// @brief Chama a função de colisão, mas com a lógica de "cair 1"
/// @return Mesma coisa da colisão
bool Board::check_collision_on_drop() const {
    return check_collision(current_x, current_y + 1, current_rotation);
}