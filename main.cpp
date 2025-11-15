#include "Game.h"

int main() {
    Game tetris_game;
    tetris_game.run();
    // O jogo rodará até 'q' ser pressionado ou alguém perder.
    // O destrutor de Game (e das jthreads) cuida da limpeza.
    
    return 0;
}