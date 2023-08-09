#pragma once
#include "./game.h"

namespace chess{
    //
    // An agent is an abstraction over an underlying move-picking function
    //
    class Agent{
    public:
        virtual Game::Move move(const Game& game) const = 0;

        virtual  ~Agent() = default;
    };

};
