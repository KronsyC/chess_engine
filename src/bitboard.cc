#include "./bitboard.h"
#include "error.h"
#include <array>
#include <iostream>

using namespace chess;

std::string Bitboard::standard_notation() const {
  auto row = this->row_no();
  auto col = this->col_no();

  char rowc = '1' + row;
  char colc = 'a' + col;

  std::string ret(2, ' ');
  ret[0] = colc;
  ret[1] = rowc;
  return ret;
}
std::string Bitboard::str(bool colorize) const {

  // Most Significant bit at top-right
  // Least Significant bit at bottom-left

  std::string str;
  std::array<bool, 64> printme;
  uint8_t col = 0;
  for (uint64_t i = 0; i < 64; i++) {
    auto row = 7 - i / 8;
    auto col = i % 8;

    printme[row * 8 + col] = (m_data >> i) & 1;
  }

  uint32_t i = 0;
  for (auto b : printme) {
    if (i % 8 == 0)
      str += "\n";
    if (colorize) {
      if (b) {
        str += "\033[0;32m 1\033[0;0m";
      } else
        str += "\033[0;31m 0\033[0;0m";
    } else {
      if (b) {
        str += " 1";
      } else
        str += " 0";
    }

    i++;
  }
  return str;
}

// Masks for columns
Bitboard Bitboard::Col1 = 0x0101010101010101;
Bitboard Bitboard::Col2 = 0x0202020202020202;
Bitboard Bitboard::Col3 = 0x0404040404040404;
Bitboard Bitboard::Col4 = 0x0808080808080808;
Bitboard Bitboard::Col5 = 0x1010101010101010;
Bitboard Bitboard::Col6 = 0x2020202020202020;
Bitboard Bitboard::Col7 = 0x4040404040404040;
Bitboard Bitboard::Col8 = 0x8080808080808080;

// Masks for rows
Bitboard Bitboard::Row1 = 0x00000000000000FF;
Bitboard Bitboard::Row2 = 0x000000000000FF00;
Bitboard Bitboard::Row3 = 0x0000000000FF0000;
Bitboard Bitboard::Row4 = 0x00000000FF000000;
Bitboard Bitboard::Row5 = 0x000000FF00000000;
Bitboard Bitboard::Row6 = 0x0000FF0000000000;
Bitboard Bitboard::Row7 = 0x00FF000000000000;
Bitboard Bitboard::Row8 = 0xFF00000000000000;

// Misc. Masks
Bitboard Bitboard::Edges = Col1 | Col8 | Row1 | Row8;
Bitboard Bitboard::Center = ~Edges;

Bitboard Bitboard::Kingside = Col6 | Col7 | Col8;
Bitboard Bitboard::Queenside = Col1 | Col2 | Col3 | Col4;

Bitboard Bitboard::InitialWhitePawns = Row2;
Bitboard Bitboard::InitialBlackPawns = Row7;

Bitboard Bitboard::WhiteQueensideCastleMustBeEmpty =
    Row1 & (Col2 | Col3 | Col4);
Bitboard Bitboard::WhiteKingsideCastleMustBeEmpty = Row1 & (Col6 | Col7);

Bitboard Bitboard::BlackQueensideCastleMustBeEmpty =
    Row8 & (Col2 | Col3 | Col4);
Bitboard Bitboard::BlackKingsideCastleMustBeEmpty = Row8 & (Col6 | Col7);

Bitboard Bitboard::WhiteQueensideCastleMustBeSafe = Row1 & (Col3 | Col4 | Col5);
Bitboard Bitboard::WhiteKingsideCastleMustBeSafe = Row1 & (Col5 | Col6 | Col7);
Bitboard Bitboard::BlackQueensideCastleMustBeSafe = Row8 & (Col3 | Col4 | Col5);
Bitboard Bitboard::BlackKingsideCastleMustBeSafe = Row8 & (Col5 | Col6 | Col7);
Bitboard Bitboard::Bitboard::col_mask(uint8_t col) {
  switch (col) {
  case 0:
    return Col1;
  case 1:
    return Col2;
  case 2:
    return Col3;
  case 3:
    return Col4;
  case 4:
    return Col5;
  case 5:
    return Col6;
  case 6:
    return Col7;
  case 7:
    return Col8;
  default:
    throw chess::Error("Out-of-bounds column number: " + std::to_string(col));
  }
}
Bitboard Bitboard::row_mask(uint8_t row) {

  switch (row) {
  case 0:
    return Row1;
  case 1:
    return Row2;
  case 2:
    return Row3;
  case 3:
    return Row4;
  case 4:
    return Row5;
  case 5:
    return Row6;
  case 6:
    return Row7;
  case 7:
    return Row8;
  default:
    throw chess::Error("Out-of-bounds row number: " + std::to_string(row));
  }
}
