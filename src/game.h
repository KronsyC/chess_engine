#pragma once
#include "./magic/moves.h"
#include "bitboard.h"
#include <array>
#include <vector>
#include<map>
#include<iostream>



namespace chess {
  namespace agents{
  class Weighted;
}
class Agent;
class Game {
    static inline bool magicmoves_initialized = false;

  Game() {
    if (!magicmoves_initialized)
      initmagicmoves();
    magicmoves_initialized = true;
  }

public:


  enum class Team {
    Black,
    White,
  };
  enum class PieceKind {
    Pawn,
    Bishop,
    Rook,
    Knight,
    Queen,
    King,
  };

  static std::string team_str(Team t) {
    switch (t) {
    case Team::White:
      return "White";
    case Team::Black:
      return "Black";
    }
  }
  static std::string kind_str(PieceKind k) {
    switch (k) {
    case PieceKind::Bishop:
      return "Bishop";
    case PieceKind::King:
      return "King";
    case PieceKind::Knight:
      return "Knight";
    case PieceKind::Rook:
      return "Rook";
    case PieceKind::Queen:
      return "Queen";
    case PieceKind::Pawn:
      return "Pawn";
    }
  }
  static Game create(std::string fen_str);

  struct PositionalInfo {
    Bitboard kings;
    Bitboard queens;
    Bitboard bishops;
    Bitboard knights;
    Bitboard rooks;
    Bitboard pawns;

    Bitboard blacks;
    Bitboard whites;

    bool operator==(const PositionalInfo &o) const = default;
  } positions;



  struct CastleInfo {
    bool wks : 1 = true;
    bool wqs : 1 = true;
    bool bks : 1 = true;
    bool bqs : 1 = true;

    bool operator==(const CastleInfo &o) const = default;
  } castle;
  Bitboard enpassant;
  uint32_t halfmoves = 0;
  uint32_t fullmoves = 1;
  enum State {
    WhiteToMove,
    BlackToMove,

    WhiteWins,
    BlackWins,

    Stalemate,

  } state = State::WhiteToMove;

  enum GameStage { Opening, MidGame, Endgame } stage = GameStage::Opening;
  // A way to store info about a given piece
  // NOTE:
  // Only valid until the next move due to the unpredictable
  // nature of the game
  struct Piece {
    PieceKind kind;
    Team team;

    Bitboard position;

    // All moves, not accounting for those
    // which put the king in peril
    Bitboard pseudolegal_moves;
  };

  struct Move {
    Piece *piece;
      Bitboard source_pos;
    Bitboard target_pos;
    enum MoveType {
      Regular,
      Capture,

      Doublejump, // For pawns
      Enpassant,  // For pawns
      PromoteQueen,
      PromoteKnight,
      PromoteRook,
      PromoteBishop,

      QueensideCastle, // For Kings
      KingsideCastle,  // For Kings

    } kind;

    std::string str() const {
      std::string str = "Moving A " + team_str(piece->team) + " " +
                        kind_str(piece->kind) + " from " +
                        piece->position.standard_notation() + " -> " +
                        target_pos.standard_notation();
      return str;
    }
  };
  Team current_active_team() const;


  template<Game::Team CURRENT_TEAM>
  Piece *fetch_piece(Bitboard position) const;

  Piece *fetch_piece(Bitboard position) const;

  void remove_piece(Piece piece);
  void add_piece(Bitboard pos, PieceKind kind, Team team);

  uint8_t make_move(Move m);

  template<Game::Team TEAM>
  [[nodiscard]] bool is_mated() const;

  [[nodiscard]] bool is_stalemate() const;

  template<Game::Team TEAM>
  bool is_checked() const;

  std::string simple_fen() const;

  // Show all positions where a piece of the
  // given team may be in danger
  // (where the enemy attacks)
  template<Game::Team TEAM>
  Bitboard danger_board() const;

  // Return all positions where the given
  // team can attack(move to)
  // (same as danger_board for opposing team)
  template<Game::Team TEAM>
  Bitboard attack_board() const;

  // Similar to checking for a non-empty attack_board
  // but much faster
  template<Game::Team TEAM>
  bool has_attack() const;

    template<Game::Team TEAM>
  Bitboard pseudo_danger_board() const;

    template<Game::Team TEAM>
    Bitboard pseudo_attack_board() const;

  //
  // Return every position where the given
  // team may move to (attack)
  // AND ALSO
  // any castling that the team may do
  //
  template<Game::Team TEAM>
  Bitboard attack_board_incl_castles() const;

  std::vector<Move> movelist(Team team) const;

  void pretty_print(Bitboard highlight = 0) const;


  __attribute__((always_inline)) Game &mut() const {
    return const_cast<Game &>(*this);
  }

  // this should be reset to 0 every time a move is made
  Bitboard cached_pieces = 0;

  // this does not have to be reset as values can be
  // simply overwritten
  std::array<Game::Piece, 64> piece_cache;


  ///////////////////////////
  ///// ZOBRIST STUFF ///////
  ///////////////////////////


  uint64_t zobrist_hash = 0;

  // This should only be called on initialization
  // The zobrist hash is incrementally updated from
  // then-on
  void generate_zobrist_hash();

  struct TranspositionTableEntry{
      float evaluation;
      uint32_t depth;

      enum NodeType{
          Exact,
          Upperbound, // Fail High
          Lowerbound, // Fail Low
      } type;

  };

  static std::map<uint64_t, TranspositionTableEntry> transposition_table;

    Move get_agent_move(const Agent& ag) const;

  ///////////////////////////////////////////
  /////// WEIGHTS FOR THE EVALUATOR /////////
  ///////////////////////////////////////////

  // TODO:
  // We have to actually train these weights using some form
  // of Machine Learning
  // the current weights are absolutely awful and are based purely
  // from guesswork, no hard data involved


  float evaluate(Team, agents::Weighted) const;
  float advantage(Team, agents::Weighted) const;

  ///////////////////////////////////
  //// Engine-Related Functions /////
  ///////////////////////////////////

  // Gets the best move for the current game using
  // the negamax algorithm

  struct RepetitionInfo {
    PositionalInfo positions;
    CastleInfo castles;

    bool operator==(RepetitionInfo o) const {
      return positions == o.positions && castles == o.castles;
    }
  };

  std::vector<RepetitionInfo> repeatable_states;

    [[nodiscard]] Bitboard pseudo_danger_board() const;

    [[nodiscard]] Bitboard pseudo_attack_board() const;
};

}; // namespace chess

#include "./game.tcc"
