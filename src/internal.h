//#include "./game.h"
//#include "bitboard.h"
#pragma once


#include "pseudolegal_move_calculator.h"

namespace chess::internal {

template <Game::PieceKind piece>
INLINE Bitboard get_pseudolegal_moves(Bitboard position, Game::Team team, Bitboard friends,
                      Bitboard world, Bitboard enpassant) {
  switch (piece) {
  case Game::PieceKind::King:
    return chess::pseudolegal_calc::king_moves(position) & ~friends;
  case Game::PieceKind::Queen:
    return chess::pseudolegal_calc::queen_moves(position, world) & ~friends;
  case Game::PieceKind::Rook:
    return chess::pseudolegal_calc::rook_moves(position, world) & ~friends;
  case Game::PieceKind::Bishop:
    return chess::pseudolegal_calc::bishop_moves(position, world) & ~friends;
  case Game::PieceKind::Knight:
    return chess::pseudolegal_calc::knight_moves(position) & ~friends;
  case Game::PieceKind::Pawn:
    return chess::pseudolegal_calc::pawn_moves(position, team, world | enpassant) &
           ~friends;
  }
}
// Internal Piece-fetching function
// WARNING:
// Undefined behaviour if the piece does not exist

template<Game::Team CURRENT_ACTIVE_TEAM>
inline INLINE Game::Piece fetch_piece_nocache(const Game &game, Bitboard pos) {

  Game::Piece piece;
  piece.position = pos;

  if (pos & game.positions.whites)
    piece.team = Game::Team::White;
  else
    piece.team = Game::Team::Black;

#define getmoves(piecekind)                                                    \
  (get_pseudolegal_moves<piecekind>(                                           \
      pos, piece.team,                                                         \
      piece.team == Game::Team::White ? game.positions.whites                  \
                                      : game.positions.blacks,                 \
      game.positions.whites | game.positions.blacks, CURRENT_ACTIVE_TEAM == piece.team ? game.enpassant : 0))
  if (pos & game.positions.kings) {
    piece.kind = Game::PieceKind::King;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::King);
  }

  else if (pos & game.positions.queens) {
    piece.kind = Game::PieceKind::Queen;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::Queen);
  } else if (pos & game.positions.bishops) {
    piece.kind = Game::PieceKind::Bishop;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::Bishop);
  } else if (pos & game.positions.knights) {
    piece.kind = Game::PieceKind::Knight;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::Knight);
  } else if (pos & game.positions.pawns) {
    piece.kind = Game::PieceKind::Pawn;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::Pawn);
  } else {
    piece.kind = Game::PieceKind::Rook;
    piece.pseudolegal_moves = getmoves(Game::PieceKind::Rook);
  }
#undef getmoves

  return piece;
}



}; // namespace chess::internal
