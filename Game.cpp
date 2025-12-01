#include "Game.h"
#include "Constants.h"
#include <chrono>
#include <clocale>

using namespace std::chrono_literals;

// Inicializa as pontuações e o estado do jogo
Game::Game() : game_over(false), p1_score(0), p2_score(0) {
    init_curses();
}

// Destrutor: limpa threads e ncurses
Game::~Game() {
    // Espera todas as threads terminarem.
    // A flag game_over (atômica) garante que elas vão parar
    if (t_input.joinable()) t_input.join();
    if (t_render.joinable()) t_render.join();
    if (t_player1.joinable()) t_player1.join();
    if (t_player2.joinable()) t_player2.join();

    cleanup_curses(); // Limpa o ncurses depois que as threads pararem
}

/// @brief Configura o ncurses para exibir o jogo no terminal
void Game::init_curses() {
    setlocale(LC_ALL, ""); // Permite o ncurses usar caracteres especiais
    initscr(); // Prepara o terminal
    cbreak(); // Inputs do usuário são enviados automaticamente
    noecho(); // Inputs do usuário não dão eco
    curs_set(0); // Torna o cursor do terminal invisível
    keypad(stdscr, TRUE); // Habilita o keypad na janela principal para ler caracteres especiais
    nodelay(stdscr, TRUE); // Não espera I/O para continuar execução
    
    start_color(); // Habilita o uso de cores e cria um par de cor para cada peça do tetris
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK); 
    init_pair(8, COLOR_WHITE, COLOR_WHITE); 

    // Cria janelas para os dois jogadores e suas pontuações
    p1_win = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 2, P1_Y_OFFSET, P1_X_OFFSET);
    p2_win = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 2, P2_Y_OFFSET, P2_X_OFFSET);
    score_win = newwin(5, 50, P1_Y_OFFSET + BOARD_HEIGHT + 3, P1_X_OFFSET);
}

/// @brief Deleta as janelas do ncurses
void Game::cleanup_curses() {
    delwin(p1_win);
    delwin(p2_win);
    delwin(score_win);
    endwin(); 
}

/// @brief Inicia as std::thread
void Game::run() {
    t_input = std::thread(&Game::input_loop, this);
    t_render = std::thread(&Game::render_loop, this);
    t_player1 = std::thread(&Game::player_loop, this, 1);
    t_player2 = std::thread(&Game::player_loop, this, 2);

    // thread principal aguarda a thread de input terminar(fim do jogo)
    t_input.join(); 
    // Quando t_input terminar (usuário apertou 'q'), as threads de jogador e renderização também terminarão
}

/// @brief THREAD 1: INPUT
void Game::input_loop() {
    while (!game_over) {
        int ch = getch();
        if (ch == ERR) {
            std::this_thread::sleep_for(10ms); 
            continue;
        }

        // Checa se a tecla de sair do jogo foi pressionada
        if (ch == QUIT_GAME) {
            game_over = true;
            break;
        }
        
        // Checa os controles dos jogadores
        if (ch == P1_LEFT || ch == P1_RIGHT || ch == P1_ROTATE || ch == P1_DOWN) {
            std::lock_guard lock(p1_input_mutex);
            p1_input_queue.push(ch);
        }
        
        if (ch == P2_LEFT || ch == P2_RIGHT || ch == P2_ROTATE || ch == P2_DOWN) {
            std::lock_guard lock(p2_input_mutex);
            p2_input_queue.push(ch);
        }
    }
}

/// @brief THREAD 2: RENDER
// Atualiza a tela com o estado atual do jogo
void Game::render_loop() {
    while (!game_over) {
        {
            //p1_mutex e p2_mutex são travados para garantir que os tabuleiros não mudem enquanto sao desenhados
            std::lock_guard lock1(p1_mutex);
            std::lock_guard lock2(p2_mutex);
            p1_board.draw(p1_win);
            p2_board.draw(p2_win);
        } 

        werase(score_win);
        box(score_win, 0, 0);
        mvwprintw(score_win, 1, 2, "Jogador 1: %d", p1_score);
        mvwprintw(score_win, 2, 2, "Jogador 2: %d", p2_score);
        mvwprintw(score_win, 3, 2, "Pressione 'q' para sair");
        wrefresh(score_win);

        // Checa o estado dos tabuleiros (protegido por mutex)
        bool p1_lost, p2_lost;
        {
            std::lock_guard lock1(p1_mutex);
            p1_lost = p1_board.is_game_over();
        }
        {
            std::lock_guard lock2(p2_mutex);
            p2_lost = p2_board.is_game_over();
        }

        if (p1_lost || p2_lost) {
            game_over = true; 
            
            // Atualiza a tela uma última vez com o estado final
            {
                std::lock_guard lock1(p1_mutex);
                std::lock_guard lock2(p2_mutex);
                p1_board.draw(p1_win);
                p2_board.draw(p2_win);
            }

            // Exibe o vencedor
            mvwprintw(score_win, 1, 25, "FIM DE JOGO!");
            if (p1_lost && !p2_lost) {
                mvwprintw(score_win, 2, 25, "Jogador 2 Venceu!");
            } else if (p2_lost && !p1_lost) {
                mvwprintw(score_win, 2, 25, "Jogador 1 Venceu!");
            } else {
                mvwprintw(score_win, 2, 25, "Empate!"); // Caso raro
            }
            
            // Muda o ncurses para o modo de input "bloqueante"
            nodelay(stdscr, FALSE);
            
            // Sobrescreve a mensagem de ajuda
            mvwprintw(score_win, 3, 2, "Pressione 'q' para sair... "); 
            wrefresh(score_win);
            
            // Fica preso aqui até o usuário pressionar 'q'
            while (getch() != 'q') {
                // Espera
            }
        }

        std::this_thread::sleep_for(33ms); // ~30 FPS
    }
}

/// @brief THREAD 3 & 4: PLAYER LOGIC
/// @param player_id Referência a qual player a thread está lidando
void Game::player_loop(int player_id) {
    // Referências para o jogador atual e oponente
    Board& my_board = (player_id == 1) ? p1_board : p2_board;
    int& my_score = (player_id == 1) ? p1_score : p2_score;
    std::mutex& my_mutex = (player_id == 1) ? p1_mutex : p2_mutex;
    std::queue<int>& my_input_queue = (player_id == 1) ? p1_input_queue : p2_input_queue;
    std::mutex& my_input_mutex = (player_id == 1) ? p1_input_mutex : p2_input_mutex;
    
    // Referências para o sistema de lixo
    std::mutex& my_garbage_mutex = (player_id == 1) ? p1_garbage_mutex : p2_garbage_mutex;
    int& my_garbage_count = (player_id == 1) ? p1_garbage_count : p2_garbage_count;
    
    std::mutex& opp_garbage_mutex = (player_id == 1) ? p2_garbage_mutex : p1_garbage_mutex;
    int& opp_garbage_count = (player_id == 1) ? p2_garbage_count : p1_garbage_count;
    std::condition_variable& opp_garbage_cv = (player_id == 1) ? p2_garbage_cv : p1_garbage_cv;
    
    const int p_left = (player_id == 1) ? P1_LEFT : P2_LEFT;
    const int p_right = (player_id == 1) ? P1_RIGHT : P2_RIGHT;
    const int p_rotate = (player_id == 1) ? P1_ROTATE : P2_ROTATE;
    const int p_down = (player_id == 1) ? P1_DOWN : P2_DOWN;

    auto last_drop_time = std::chrono::steady_clock::now();
    long drop_speed = 1000; // 1 segundo

    while (!game_over) {
        
        // 1. Processar Lixo recebido
        int garbage_to_add = 0;
        {
            // Trava rapidamente o contador de lixo
            std::lock_guard lock(my_garbage_mutex);
            if (my_garbage_count > 0) {
                garbage_to_add = my_garbage_count; // Pega todo o lixo pendente
                my_garbage_count = 0;              // Zera o contador
            }
        } // Libera o mutex do lixo

        if (garbage_to_add > 0) {
            // Se tinha lixo, trava o tabuleiro e o adiciona
            std::lock_guard lock(my_mutex);
            my_board.add_garbage(garbage_to_add);
            if (my_board.is_game_over()) break;
        }

        // 2. Processar Input do Jogador
        {
            std::lock_guard lock_input(my_input_mutex); // Trava a fila de input

            // Só trava o tabuleiro se houver algo para processar
            if (!my_input_queue.empty()) { 
                
                std::lock_guard lock_board(my_mutex); // Trava o tabuleiro UMA VEZ
                
                while (!my_input_queue.empty()) { // Esvazia a fila
                    int cmd = my_input_queue.front();
                    my_input_queue.pop();

                    // Processa todos os comandos pendentes
                    if (cmd == p_left) my_board.move_piece(-1, 0);
                    else if (cmd == p_right) my_board.move_piece(1, 0);
                    else if (cmd == p_rotate) my_board.rotate_piece();
                    else if (cmd == p_down) {
                        // Tenta mover para baixo
                        if (my_board.move_piece(0, 1)) {
                            last_drop_time = std::chrono::steady_clock::now();
                        } else {
                            int lines_cleared = my_board.fix_piece_and_clear_lines();
                            my_board.spawn_new_piece();
                            if (my_board.is_game_over()) {
                                break;
                            }
                        
                            my_score += lines_cleared * lines_cleared * 10;
                            int garbage_to_send = 0;
                            if (lines_cleared >= 4) garbage_to_send = 3;
                            else if (lines_cleared >= 2) garbage_to_send = lines_cleared - 1;
                            
                            if (garbage_to_send > 0) {
                                { std::lock_guard lock(opp_garbage_mutex);
                                opp_garbage_count += garbage_to_send; }
                                opp_garbage_cv.notify_one();
                            }
                            
                            last_drop_time = std::chrono::steady_clock::now();
                        }
                    }
                }
            }
        }

        // 3. Processar Gravidade (Tick do Jogo)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_drop_time);

        if (elapsed.count() > drop_speed) {
            last_drop_time = now;
            bool piece_fixed = false;
            int lines_cleared = 0;

            {
                std::lock_guard lock(my_mutex);
                if (my_board.check_collision_on_drop()) { // Checa colisão
                    lines_cleared = my_board.fix_piece_and_clear_lines();
                    my_board.spawn_new_piece();
                    if (my_board.is_game_over()) break;
                    piece_fixed = true;
                } else {
                    my_board.move_piece(0, 1);
                }
            } // Libera mutex

            // 4. Enviar Lixo
            if (piece_fixed) { // Se a peça caiu, envia lixo
                my_score += lines_cleared * lines_cleared * 10; 
                
                int garbage_to_send = 0;
                if (lines_cleared >= 4) {
                    garbage_to_send = 3;
                } else if (lines_cleared >= 2) {
                    garbage_to_send = lines_cleared - 1;
                }
                
                if (garbage_to_send > 0) {
                    {
                        // Trava o contador do oponente
                        std::lock_guard lock(opp_garbage_mutex);
                        opp_garbage_count += garbage_to_send; // Adiciona o lixo
                    }
                    // Notifica a outra thread (ela pode estar dormindo)
                    opp_garbage_cv.notify_one(); 
                }
            }
        }
        std::this_thread::sleep_for(50ms); // Poll rate
    }
}
