#include "pseudolegal_move_calculator.h"
#include "bitboard.h"
#include "magic/moves.h"
#include <iostream>
using namespace chess;

#define INLINE __attribute__((always_inline))

static std::array<chess::Bitboard, 64> king_moves_table = []() {
  std::array<Bitboard, 64> ret;

  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();
    Bitboard val = board;

    // Up and downshifts dont need to be bounds-checked
    val |= val.up();
    val |= val.down();

    if (col != 0) {
      val |= val.left();
    }
    if (col != 7) {
      val |= val.right();
    }

    ret[idx] = val & ~board;
  }

  return ret;
}();

static std::array<chess::Bitboard, 64> knight_moves_table = []() {
  std::array<Bitboard, 64> ret;

  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();
    Bitboard val = board;

    auto up1 = val.up();
    auto up2 = val.up<2>();
    auto down1 = val.down();
    auto down2 = val.down<2>();

    // Immediate Left
    if (col > 0) {
      val |= up2.left();
      val |= down2.left();
    }

    // Far Left
    if (col > 1) {
      val |= up1.left<2>();
      val |= down1.left<2>();
    }

    // Immediate Right
    if (col < 7) {
      val |= up2.right();
      val |= down2.right();
    }

    // Far Right
    if (col < 6) {
      val |= up1.right<2>();
      val |= down1.right<2>();
    }

    ret[idx] = val & ~board;
    // std::cout << "RESPONSES: " << ret[idx].str() << std::endl;
  }

  return ret;
}();

static std::array<chess::Bitboard, 64> white_pawn_attacks = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();

    // std::cout << board.str() << std::endl;
    auto up = board.up();
    Bitboard val;
    if (col != 0)
      val |= up.left();
    if (col != 7)
      val |= up.right();
    // std::cout << val.str() << std::endl;
    ret[idx] = val;
  }
  return ret;
}();
static std::array<chess::Bitboard, 64> white_pawn_doublejump_mask = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();

    Bitboard val;
    val |= board.up();
    val |= board.up<2>();
    ret[idx] = val;
  }
  return ret;
}();

static std::array<chess::Bitboard, 64> black_pawn_doublejump_mask = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();

    Bitboard val;
    val |= board.down();
    val |= board.down<2>();
    ret[idx] = val;
  }
  return ret;
}();

static std::array<chess::Bitboard, 64> white_pawn_onejump_mask = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();

    Bitboard val;
    val |= board.up();
    ret[idx] = val;
  }
  return ret;
}();

static std::array<chess::Bitboard, 64> black_pawn_onejump_mask = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();

    Bitboard val;
    val |= board.down();
    ret[idx] = val;
  }
  return ret;
}();
static std::array<chess::Bitboard, 64> black_pawn_attacks = []() {
  std::array<Bitboard, 64> ret;
  for (Bitboard board = 1; board; board = board.right()) {
    auto idx = board.trailing_zeroes();
    auto col = board.col_no();
    auto down = board.down();
    Bitboard val;
    if (col != 0)
      val |= down.left();
    if (col != 7)
      val |= down.right();

    ret[idx] = val;
  }
  return ret;
}();
INLINE Bitboard
pseudolegal_calc::king_moves(Bitboard position) {
  return king_moves_table[position.trailing_zeroes()];
}
INLINE Bitboard
pseudolegal_calc::knight_moves(Bitboard position) {
  return knight_moves_table[position.trailing_zeroes()];
}
INLINE Bitboard
pseudolegal_calc::pawn_moves(Bitboard position, Game::Team team,
                             Bitboard world) {
  Bitboard empties = ~world;
  if (team == Game::Team::White) {
    Bitboard ret;
    ret ^= position.up() & empties;
    ret ^= (ret & Bitboard::Row3).up<1>() & empties;
    auto diagonals = white_pawn_attacks[position.trailing_zeroes()] & world;
    return ret | diagonals;

  } else {
    Bitboard ret;
    ret ^= position.down() & empties;
    ret ^= (ret & Bitboard::Row6).down() & empties;
    auto diagonals = black_pawn_attacks[position.trailing_zeroes()] & world;
    return ret | diagonals;
  }
}


Bitboard pseudolegal_calc::parallel_pawn_moves(Bitboard pawns, Game::Team team, Bitboard world){
  /// Parallel pawn move calculation
  /// Currently not in use

  Bitboard empties = ~world;
  if(team == Game::Team::White){
    Bitboard ret;
    // Basic one-square move
    ret ^= pawns.up<1>() & empties;

    // Double-Jump
    ret ^= (ret & Bitboard::Row3).up<1>() & empties;

    while(pawns){
      auto idx = pawns.popbit();
      ret |= white_pawn_attacks[idx] & world;
    }

    return ret;
  }
  else{
    Bitboard ret;
    // Basic one-square move
    ret ^= pawns.down<1>() & empties;

    // Double-Jump
    ret ^= (ret & Bitboard::Row6).down<1>() & empties;

    while(pawns){
      auto idx = pawns.popbit();
      ret |= black_pawn_attacks[idx] & world;
    }

    return ret;
  }
}
INLINE Bitboard
pseudolegal_calc::rook_moves(Bitboard position, Bitboard world) {
  return Rmagic(position.trailing_zeroes(), (uint64_t)world);
}
INLINE Bitboard
pseudolegal_calc::bishop_moves(Bitboard position, Bitboard world) {
  return Bmagic(position.trailing_zeroes(), (uint64_t)world);
}
INLINE Bitboard
pseudolegal_calc::queen_moves(Bitboard position, Bitboard world) {
  Bitboard bits = Qmagic(position.trailing_zeroes(), (uint64_t)world);
  return bits;
}
