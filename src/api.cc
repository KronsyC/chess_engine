#include "./api.h"
#include "./game.h"
#include"./agents/weighted.h"
#include "./agents/random.h"
#include "bitboard.h"
#include <iostream>
#define asstate(p) ((chess::Game *)p)
#define asmove(m) ((chess::Game::Move *)m)
struct ChessPiece {
  chess::Game::Piece *ptr;
};

struct ChessMove {
  chess::Game::Move move;
};

// using chess::Piece;
uint8_t chess__move_from(void *move) {
  return asmove(move)->piece->position.trailing_zeroes();
}
uint8_t chess__move_to(void *move) {
  return asmove(move)->target_pos.trailing_zeroes();
}
enum MoveKind chess__move_kind(void *move) {
  return (MoveKind)asmove(move)->kind;
}
void *chess__game_create(const char *FEN_str) {
  try {
    auto game = new chess::Game(chess::Game::create(FEN_str));
    return game;
  } catch (std::exception) {
    return nullptr;
  }
}

enum GameState chess__game_state(void *game) {
  return (enum GameState)(asstate(game)->state);
}

void chess__game_delete(void *game) { delete asstate(game); }
void chess__game_print(void *game) { asstate(game)->pretty_print(); }
void chess__print_board(__Bitboard board) {
  chess::Bitboard b(board);
  std::cout << b.str() << std::endl;
}

__Bitboard chess__game_pawns(void *game) {
  return (uint64_t)asstate(game)->positions.pawns;
}
__Bitboard chess__game_kings(void *game) {
  return (uint64_t)asstate(game)->positions.kings;
}
__Bitboard chess__game_queens(void *game) {
  return (uint64_t)asstate(game)->positions.queens;
}
__Bitboard chess__game_bishops(void *game) {
  return (uint64_t)asstate(game)->positions.bishops;
}
__Bitboard chess__game_rooks(void *game) {
  return (uint64_t)asstate(game)->positions.rooks;
}
__Bitboard chess__game_knights(void *game) {
  return (uint64_t)asstate(game)->positions.knights;
}

__Bitboard chess__game_pieces(void *game, ChessTeam team) {
  switch (team) {
  case TEAM_Black:
    return (uint64_t)asstate(game)->positions.blacks;
  case TEAM_White:
    return (uint64_t)asstate(game)->positions.whites;
  }
}

ChessPiece *chess__piece_find(void *game, uint8_t row, uint8_t col) {
    auto pos = (1ULL << (row*8)) << col;

    auto t = asstate(game)->positions.whites & pos ? Game::Team::White : Game::Team::Black;

  auto piece =t == chess::Game::Team::White ? asstate(game)->fetch_piece<chess::Game::Team::White>((1ULL << row * 8) << col) : asstate(game)->fetch_piece<chess::Game::Team::Black>((1ULL << row * 8) << col);

  if (!piece)
    return nullptr;
  ChessPiece ret;
  ret.ptr = piece;
  return new ChessPiece(ret);
}

void chess__piece_delete(ChessPiece *piece) { delete piece; }
void chess__piece_move(void *game, void *move) {
  auto board = asstate(game);

  board->make_move(*asmove(move));
}
enum ChessTeam chess__piece_get_team(struct ChessPiece *piece) {
  return (ChessTeam)piece->ptr->team;
}
enum PieceKind chess__piece_get_kind(struct ChessPiece *piece) {
  return (PieceKind)piece->ptr->kind;
}

MoveList chess__game_moves(void *_game, ChessTeam team) {
  auto game = asstate(_game);
  auto moves = game->movelist((chess::Game::Team)team);
  MoveList movelist;
  movelist.count = moves.size();
  movelist.data = new void *[moves.size()];
  for (uint64_t i = 0; i < moves.size(); i++) {
    movelist.data[i] = new chess::Game::Move(moves[i]);
  }
  return movelist;
}

bool chess__game_mated(void *game, ChessTeam team) {
    return team == ChessTeam::TEAM_Black ? asstate(game)->is_mated<chess::Game::Team::Black>() : asstate(game)->is_mated<chess::Game::Team::White>();
}

enum ChessTeam chess__game_get_team(void *game) {
  return (enum ChessTeam)asstate(game)->current_active_team();
}

bool chess__game_checked(void *game, ChessTeam team) {
    return team == TEAM_Black ? asstate(game)->is_mated<chess::Game::Team::Black>() : asstate(game)->is_mated<chess::Game::Team::White>();

}
bool chess__game_stalemated(void *game) {
  return asstate(game)->state == chess::Game::State::Stalemate;
}

float chess__game_evaluation(void *game, enum ChessTeam team, void* evaluator) {
  return asstate(game)->evaluate((chess::Game::Team)team, *(chess::agents::Weighted*)evaluator);
}
float chess__game_advantage(void *game, enum ChessTeam team, void* evaluator) {
  return asstate(game)->advantage((chess::Game::Team)team, *(chess::agents::Weighted*)evaluator);
}

uint32_t chess__game_halfmoves(void *game) { return asstate(game)->halfmoves; }
uint32_t chess__game_fullmoves(void *game) { return asstate(game)->fullmoves; }

void* chess__game_agent_move(void* game, void* agent){
  auto g = asstate(game);
  auto a = (chess::Agent*)agent;
  auto move = g->get_agent_move(*a);
  return new chess::Game::Move(move);
}

void chess__delete_move(void* move){
  delete (chess::Game::Move*)move;
}

void* chess__create_weighted_agent(){
  const char* AGENT_DATA = 
  "center_control;M;586.260437,161.158813,1.582336\n"
  "check;M;589.273804,847.496948,62.811127\n"
  "king_front_pawns;M;177.766479,1855.725586,1212.849854\n"
  "material;M;305.160645,0.264507,1663.406128\n"
  "mobility;M;409.657318,19720.175781,7176.076660\n"
  "pawn_devel;M;1381.781372,1.422119,793.646545\n"
  "positioning;M;1723.009399,0.669816,10.812225\n"
  "positions;P;2.565273,46.566368,3.345390,9.758125,2.367783,3.347683\n"
  "protected_pieces;M;734.776001,176.327698,518.665527\n"
  "values;P;27.044556,10002.280273,6036.456055,2083.186523,2017.714600,69.289246\n"
  "vulnerability;M;522.239380,0.151626,0.636688";
  auto ag = new chess::agents::Weighted(chess::agents::Weighted::decode(AGENT_DATA));
  return ag;
}
void* chess__create_random_agent(){
  return new chess::agents::Random();
}

void chess__delete_weighted_agent(void* agent){
  delete (chess::agents::Weighted*)agent;
}

void chess__delete_random_agent(void* agent){
  delete (chess::agents::Random*)agent;
}