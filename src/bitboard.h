#pragma once

#define INLINE __attribute__((always_inline))
#include <bit>
#include <string>
namespace chess {

//
// A "Bitboard" is a wrapper for a 64-bit integer
// I chose to use bitboards as we can use bitwise
// operators on them, which are very fast
//
// The least significant bit (2^0) Represents A1
// The most significant bit (2^63) Represents H8
//
// Shifting left by 8 moves up by a square     ( x << 8 )
// Shifting right by 8 moves down by a square  ( x >> 8 )
// Shifting left by 1 moves right by a square  ( x << 1 )
// Shifting right by 1 moves left by a square  ( x >> 1 )
//
// Bitwise OR  ( | ) is used to combine bitboards
// Bitwise XOR ( ^ ) is used to combine bitboards, BUT invert where they overlap
// Bitwise AND ( & ) is used to find what 2 bitboards share in common
// Bitwise NOT ( ~ ) is used to invert a bitboard (find where it is not set)
//
// for example
// ENEMY_ATTACKS AND MY_PIECES would find where the enemy attacks my pieces
// ENEMY_ATTACKS AND (WHITE_PIECES & KINGS) would find where the white kings are
// attacked (detect check)
// KING_MOVES AND NOT ENEMY_ATTACKS would find where it is legal to move the
// king (sort of)
//
class Bitboard {
private:
  uint64_t m_data = 0;

public:
  std::string str(bool colorize = true) const;
  std::string standard_notation() const;
  INLINE uint8_t count() const { return std::popcount(m_data); }

  Bitboard(uint64_t data) { this->m_data = data; }
  Bitboard() = default;
  INLINE bool empty() const { return !m_data; }

  INLINE explicit operator uint64_t() const { return m_data; }
  INLINE explicit operator bool() const { return m_data; }
  // We make use of direct CPU instructions to drastically
  // increase the speed of the code
  // trailing zeroes are used as an index into move lookup tables
  // and therefore must be a fast operation
  INLINE uint8_t trailing_zeroes() const { return std::countr_zero(m_data); }

  //
  // Removes the least significant 1 bit from the board and returns its index
  //
  INLINE uint8_t popbit(){
    // https://stackoverflow.com/questions/72336579/good-way-of-popping-the-least-signifigant-bit-and-returning-the-index
    uint8_t idx = trailing_zeroes();
    m_data &= m_data-1;
    return idx;
  }


  // Translate a unary bitboard (1 bit set) to a row number
  uint16_t row_no() const { return (trailing_zeroes() / 8); }
  // Translate a unary bitboard (1 bit set) to a column number
  uint16_t col_no() const { return trailing_zeroes() % 8; }

  /////////////////////////////
  ///// Bitwise Operators /////
  /////////////////////////////
  INLINE Bitboard operator&(const Bitboard other) const {

    return other.m_data & m_data;
  }
  INLINE Bitboard operator|(const Bitboard other) const {
    return other.m_data | m_data;
  }
  INLINE Bitboard operator^(const Bitboard other) const {
    return other.m_data ^ m_data;
  }
  Bitboard operator^=(const Bitboard other) {
    m_data ^= other.m_data;
    return *this;
  }
  // INLINE Bitboard operator<<(int shift_by) const {
  //   return m_data << shift_by;
  // }
  // INLINE Bitboard operator>>(int shift_by) const {
  //   return m_data >> shift_by;
  // }

  Bitboard operator&=(const Bitboard other) {
    m_data &= other.m_data;
    return *this;
  }
  Bitboard operator|=(const Bitboard other) {
    m_data |= other.m_data;
    return *this;
  }
  Bitboard operator<<=(uint64_t shify_by) {
    m_data <<= shify_by;
    return *this;
  }
  Bitboard operator>>=(uint64_t shift_by) {
    m_data >>= shift_by;
    return *this;
  }
  INLINE Bitboard operator~() const { return ~m_data; }

  INLINE bool operator!() const { return !m_data; }

  INLINE bool operator==(Bitboard other) const {
    return m_data == other.m_data;
  }
  INLINE bool operator!=(Bitboard other) const{
    return m_data != other.m_data;
  }

  //////////////////////////////////////////////////////////
  //// Wrappers for common operations (for readability) ////
  //////////////////////////////////////////////////////////

  template <int BY = 1> INLINE Bitboard up() const {
    return m_data << (BY << 3);
  }
  template <int BY = 1> INLINE Bitboard down() const {
    return m_data >> (BY << 3);
  }
  template <int BY = 1> INLINE Bitboard left() const { return m_data >> BY; }

  template <int BY = 1> INLINE Bitboard right() const { return m_data << BY; }

  /////////////////////////////////////////////////////
  ////// Precomputed Bitboards for various Masks //////
  /////////////////////////////////////////////////////

  static Bitboard Col1;
  static Bitboard Col2;
  static Bitboard Col3;
  static Bitboard Col4;
  static Bitboard Col5;
  static Bitboard Col6;
  static Bitboard Col7;
  static Bitboard Col8;

  static Bitboard Row1;
  static Bitboard Row2;
  static Bitboard Row3;
  static Bitboard Row4;
  static Bitboard Row5;
  static Bitboard Row6;
  static Bitboard Row7;
  static Bitboard Row8;

  //
  // I have predefined a few bitboards below
  // these boards allow for faster code
  //
  static Bitboard Edges;
  static Bitboard Center;

  static Bitboard Kingside;
  static Bitboard Queenside;

  static Bitboard InitialWhitePawns;
  static Bitboard InitialBlackPawns;

  static Bitboard WhiteQueensideCastleMustBeEmpty;
  static Bitboard WhiteKingsideCastleMustBeEmpty;
  static Bitboard BlackQueensideCastleMustBeEmpty;
  static Bitboard BlackKingsideCastleMustBeEmpty;

  static Bitboard WhiteQueensideCastleMustBeSafe;
  static Bitboard WhiteKingsideCastleMustBeSafe;
  static Bitboard BlackQueensideCastleMustBeSafe;
  static Bitboard BlackKingsideCastleMustBeSafe;

  static Bitboard col_mask(uint8_t col);
  static Bitboard row_mask(uint8_t col);
};

}; // namespace chess
