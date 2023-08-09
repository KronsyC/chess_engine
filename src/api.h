#ifndef CHESS__API_H
#define CHESS__API_H
#include <inttypes.h>
#include <stdbool.h>
typedef uint64_t __Bitboard;

enum ChessTeam {
  TEAM_Black,
  TEAM_White,
};
enum PieceKind { P_Pawn, P_Bishop, P_Rook, P_Knight, P_Queen, P_King };
enum GameState {
  WhiteMoves,

  BlackMoves,

  WhiteWins,
  BlackWins,

  Draw,
};

enum MoveKind {
  Regular,
  Capture,

  Doublejump, // For pawns
  Enpassant,  // For pawns
  PromoteQueen,
  PromoteKnight,
  PromoteRook,
  PromoteBishop,

  QueensideCastle, // For Kings
  KingsideCastle,
};

#ifdef __cplusplus
#define PFX extern "C"
#else
#define PFX
#endif

struct ChessPiece;

struct MoveList {
  void **data;
  uint64_t count;
};

#ifdef _WIN32
#define CFN __declspec(dllexport) PFX
#else
#define CFN PFX
#endif

#define CFLAGFN CFN __Bitboard
#define CBOOLFN CFN bool
#define COBJFN CFN void *
#define CVOIDFN CFN void
#define CINTFN CFN int32_t
#define CUINTFN CFN uint32_t

CFN uint8_t chess__move_from(void *move);
CFN uint8_t chess__move_to(void *move);
CFN enum MoveKind chess__move_kind(void *move);

COBJFN chess__game_create(const char *FEN_str);
CVOIDFN chess__game_delete(void *game);
CVOIDFN chess__game_print(void *game);

CFN enum GameState chess__game_state(void *game);

CFLAGFN chess__game_pawns(void *game);
CFLAGFN chess__game_kings(void *game);
CFLAGFN chess__game_queens(void *game);
CFLAGFN chess__game_rooks(void *game);
CFLAGFN chess__game_bishops(void *game);
CFLAGFN chess__game_knights(void *game);

CFLAGFN chess__game_pieces(void *game, enum ChessTeam team);

CFN struct ChessPiece *chess__piece_find(void *game, uint8_t row, uint8_t col);
CFN enum ChessTeam chess__piece_get_team(struct ChessPiece *piece);
CFN enum PieceKind chess__piece_get_kind(struct ChessPiece *piece);

CVOIDFN chess__print_board(__Bitboard board);

CVOIDFN chess__piece_delete(struct ChessPiece *piece);

CFN enum ChessTeam chess__game_get_team(void *game);

CVOIDFN chess__piece_move(void *game, void *move);

CFN MoveList chess__game_moves(void *game, enum ChessTeam team);

CBOOLFN chess__game_mated(void *game, enum ChessTeam team);
CBOOLFN chess__game_checked(void *game, enum ChessTeam team);
CBOOLFN chess__game_stalemated(void *game);

CUINTFN chess__game_halfmoves(void *game);
CUINTFN chess__game_fullmoves(void *game);

CFN float chess__game_evaluation(void *game, enum ChessTeam team, void* agent);
CFN float chess__game_advantage(void *game, enum ChessTeam team, void* agent);


CFN void* chess__game_agent_move(void* game, void* agent);

CFN void chess__delete_move(void* move);

CFN void* chess__create_weighted_agent();
CFN void* chess__create_random_agent();

CFN void chess__delete_weighted_agent(void* agent);
CFN void chess__delete_random_agent(void* agent);


#undef CFN
#undef PFX
#undef CFLAGFN
#undef CBOOLFN
#undef COBJFN
#undef CVOIDFN
#undef CINTFN
#endif
