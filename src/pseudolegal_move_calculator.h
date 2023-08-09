
#include "./game.h"
#include "bitboard.h"

namespace chess::pseudolegal_calc {
Bitboard pawn_moves(Bitboard position, Game::Team team, Bitboard world);
Bitboard king_moves(Bitboard position);
Bitboard knight_moves(Bitboard position);
Bitboard rook_moves(Bitboard position, Bitboard world);
Bitboard bishop_moves(Bitboard position, Bitboard world);
Bitboard queen_moves(Bitboard position, Bitboard world);


Bitboard parallel_pawn_moves(Bitboard pawns, Game::Team team, Bitboard world);
}; // namespace chess::pseudolegal_calc
