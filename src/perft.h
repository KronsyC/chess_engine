#pragma once
#include "game.h"

/*

Perft is a simple function designed to test the parity of our chess
model with a known valid one.

Takes an initial state, and search depth as an input
and yields the total number of positions that can be reached
after n moves

This is similar to shannon's number calculations
*/

size_t perft(const chess::Game& game, int depth){

    using chess::Game;

    auto state = game.state;
    if(state != Game::State::WhiteToMove && state != Game::State::BlackToMove){
        return 0;
    }

    auto moves = game.movelist(game.current_active_team());
    
    if(depth == 1){
        return moves.size();
    }

    size_t n = 0;

    for(auto& m : moves){
        auto clone = game;

        clone.make_move(m);

        n += perft(clone, depth-1);
    }

    return n;
}