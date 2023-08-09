#include "./game.h"
#include "./agents/weighted.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <future>
#include "train.h"
#include "perft.h"
const char *OPENING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
using namespace chess;

Game::State play_game(chess::Agent &player1, chess::Agent &player2)
{
    auto g = chess::Game::create(OPENING_FEN);

    uint64_t i = 0;
    while (g.state == Game::State::WhiteToMove || g.state == Game::State::BlackToMove)
    {
        //        std::cout << "Move: " << i << std::endl;
        auto black_attacks = g.attack_board<Game::Team::Black>();
        //        g.pretty_print(black_attacks);
        auto optimal_move = g.state == Game::State::WhiteToMove ? g.get_agent_move(player1) : g.get_agent_move(player2);
        auto z1 = g.zobrist_hash;

        if (z1 != g.zobrist_hash)
        {
            std::cout << "Incremental Zobrist hash: " << z1 << " does not match expected " << g.zobrist_hash << std::endl;
            exit(1);
        }
        int status = g.make_move(optimal_move);

        if (status != 0)
        {
            std::cout << "Exit with status: " << status << std::endl;
            exit(1);
        }

        //        std::cout << "ITER: " << i << std::endl;
        i++;
    }
    return g.state;
}
void print_state(Game::State s)
{
    switch (s)
    {
    case Game::State::WhiteToMove:
        std::cout << "White to Move!" << std::endl;
        break;
    case Game::State::BlackToMove:
        std::cout << "Black to Move!" << std::endl;
        break;
    case Game::State::BlackWins:
        std::cout << "Black Won!" << std::endl;
        break;
    case Game::State::WhiteWins:
        std::cout << "White Won!" << std::endl;
        break;
    case Game::State::Stalemate:
        std::cout << "Draw!!" << std::endl;
        break;
    }
}



int main(int argc, char** argv)
{   
    // std::cout << agents::Weighted().encode() << std::endl;
    if(argc < 3){
        std::cerr << "Bad Arg Count, Expected FEN string and depth" << std::endl;
        exit(1);
    }
    auto game = chess::Game::create(argv[1]);


    for(int i = 1; i <= atoi(argv[2]); i++){
        auto before = std::chrono::steady_clock::now();
        auto result = perft(game, i);
        auto after = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
        std::cout << "perft("<<i<<") -> " << result << " ( " << delta << "ms )" << std::endl; 
    }
    // trainify();
    return 0;
}
