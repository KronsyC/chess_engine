#pragma once

//
// The random agent choses a random move from the list of moves
//

#include "../agent.h"
#include "../error.h"
namespace chess::agents{

    class Random : public chess::Agent{

    public:
        Game::Move move(const Game& g) const override{
            auto all_moves = g.movelist(g.current_active_team());
            if(all_moves.size() == 0){
                throw chess::Error("Tried to generate a random move in a finished game");
            }
            auto index = std::rand() % all_moves.size();
            return all_moves[index];
        }

    };
};
