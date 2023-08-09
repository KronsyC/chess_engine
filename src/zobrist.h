#pragma once

#include<array>
#include<random>
#include<cstdint>
//
// Relavant Docs:
// https://www.chessprogramming.org/Zobrist_Hashing
//
namespace chess::zobrist {

    typedef uint64_t Hash;

    typedef std::array<Hash, 64> HashList;

    inline std::mt19937_64 gen(123456);
    inline std::uniform_int_distribution <uint64_t> dis;

    inline Hash rand_hash() {
        return dis(gen);
    }


    inline HashList random_values() {
        HashList values;

        for (int i = 0; i < 64; i++) {
            values[i] = rand_hash();
        }

        return values;
    }

#define Def inline HashList

    inline HashList white_pawns = random_values();
    inline HashList black_pawns = random_values();

    inline HashList white_rooks = random_values();
    inline HashList black_rooks = random_values();

    inline HashList white_bishops = random_values();
    inline HashList black_bishops = random_values();

    inline HashList white_knights = random_values();
    inline HashList black_knights = random_values();

    inline HashList white_queens = random_values();
    inline HashList black_queens = random_values();

    inline HashList white_kings = random_values();
    inline HashList black_kings = random_values();

    inline Hash white_kingside = rand_hash();
    inline Hash white_queenside = rand_hash();
    inline Hash black_kingside = rand_hash();
    inline Hash black_queenside = rand_hash();

    inline Hash black_to_move = rand_hash();

    inline Hash enpassant_row1 = rand_hash();
    inline Hash enpassant_row2 = rand_hash();
    inline Hash enpassant_row3 = rand_hash();
    inline Hash enpassant_row4 = rand_hash();
    inline Hash enpassant_row5 = rand_hash();
    inline Hash enpassant_row6 = rand_hash();
    inline Hash enpassant_row7 = rand_hash();
    inline Hash enpassant_row8 = rand_hash();

};