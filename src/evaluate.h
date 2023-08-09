#pragma once

#include "./game.h"
#include "./pst.h"
#include "./game.tcc"
#include<string>
#include <utility>

inline float randf(float max){
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX) / max);
}
inline float randf(float max, float min){
    return randf(max-min) + min;
}
namespace chess::evaluators {
    inline std::vector<std::string> split(std::string s, std::string delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            token = s.substr (pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back (token);
        }

        res.push_back (s.substr (pos_start));
        return res;
    }
    inline float absf(float v){
        return v >= 0 ? v : -v;
    }
    struct EvalParameter{
        virtual std::string encode() const = 0;
        virtual void mutate(float factor) = 0;
    };

    // TODO: Introduce the single-value parameter for one-time bonuses / reductions

    struct MultistageEvaluationWeight : public EvalParameter{
        float opening = 1;
        float midgame = 1;
        float endgame = 1;

        MultistageEvaluationWeight operator*(float by) const{
            return {opening*by, midgame*by, endgame*by};
        }
        MultistageEvaluationWeight& operator+=(MultistageEvaluationWeight other){
            opening += other.opening;
            midgame += other.midgame;
            endgame += other.endgame;
            return *this;
        }
        void mutate(float range){
            auto r = rand()%3;
            if(r == 0){
                opening = absf(opening + randf(-range, range));
            }
            else if(r == 1){
                midgame = absf(midgame+randf(-range, range));
            }
            else {
                endgame = absf(endgame + randf(-range, range));
            }
        }
        MultistageEvaluationWeight(float op, float mg, float eg){
            opening = op;
            midgame = mg;
            endgame = eg;
        }
        [[nodiscard]] std::string encode() const override{
            return "M;"+std::to_string(opening)+","+std::to_string(midgame)+","+std::to_string(endgame);
        }
        static MultistageEvaluationWeight from_str(std::string s){
            // std::cout << s << std::endl;
            auto parts = split(std::move(s), ",");
#define part(n) std::stof(parts[n])
            return {part(0), part(1), part(2)};
#undef part
        }
    };

    struct PerPiece : EvalParameter{
        float king;
        float queen ;
        float rook;
        float bishop;
        float knight;
        float pawn;

        PerPiece(float king, float queen, float rook, float bishop, float knight, float pawn){
            this->king = king;
            this->queen = queen;
            this->rook = rook;
            this->bishop = bishop;
            this->knight = knight;
            this->pawn = pawn;
        }
        PerPiece operator*(float by) const{
            return {
                king*by,
                queen*by,
                rook*by,
                bishop*by,
                knight*by,
                pawn*by,
            };
        }
        PerPiece& operator+=(PerPiece other){
            king += other.king;
            queen+=other.queen;
            knight+=other.knight;
            rook+=other.rook;
            bishop+=other.bishop;
            pawn+=other.pawn;
            return *this;
        }
        void mutate(float range){
            auto r = rand()%6;
            auto v = randf(-range, range);
            switch(r){
                case 0:
                    king = absf(king+v);
                    break;
                case 1:
                    queen = absf(queen+v);
                    break;
                case 2:
                    rook = absf(rook+v);
                    break;
                case 3:
                    bishop = absf(bishop+v);
                    break;
                case 4:
                    knight = absf(knight + v);
                    break;
                case 5:
                    pawn = absf(pawn + v);
                    break;
            }
        }
        [[nodiscard]] std::string encode() const override{
            return "P;"+std::to_string(king)+","+std::to_string(queen)+","+std::to_string(rook)+","+std::to_string(bishop)+","+std::to_string(knight)+","+std::to_string(pawn);
        }
        static PerPiece from_str(std::string s){
            auto parts = split(std::move(s), ",");
#define part(n) std::stof(parts[n])
            return {part(0), part(1), part(2), part(3), part(4), part(5)};
#undef part
        }

    };

using namespace chess;

// Encourage positions where the enemy is in check
// discourage allowing yourself to fall into check
inline float check(const Game &game, Game::Team team) {
    const float res = team==Game::Team::White ? 1 : -1;
    if(game.is_checked<Game::Team::White>()){
        return -res;
    }
    else if(game.is_checked<Game::Team::Black>()){
        return res;
    }
    else return 0;
}

// Encourage having lots of moves available
inline float mobility(const Game &game, Game::Team team) {
  return team == Game::Team::Black ? game.positions.blacks.count() : game.positions.whites.count();
}

// Discourage being under attack
// pieces with higher weights influence the vulnerability
// more greatly
inline float vulnerability(const Game &game, Game::Team team, PerPiece weights) {
    //
    // FIXME: This function is temporarily disabled because its very expensive and takes up 50% of runtime
    //
    // return 0;
  auto occ =
      team == Game::Team::White ? game.positions.whites : game.positions.blacks;



  // Find all pieces



  // Determine the number of each piece under attack
  auto attacked = team == Game::Team::Black ? game.pseudo_danger_board<Game::Team::Black>() : game.pseudo_danger_board<Game::Team::White>();
    occ &= attacked;

  auto kings = game.positions.kings & occ;
  auto queens = game.positions.queens & occ;
  auto knights = game.positions.knights & occ;
  auto bishops = game.positions.bishops & occ;
  auto rooks = game.positions.rooks & occ;
  auto pawns = game.positions.pawns & occ;



  float kings_cnt = kings.count();
  float queens_cnt = queens.count();
  float knights_cnt = knights.count();
  float bishops_cnt = bishops.count();
  float rooks_cnt = rooks.count();
  float pawns_cnt = pawns.count();

  // Return the weighted sum of these values
  return -(kings_cnt * weights.king +
           queens_cnt * weights.queen +
           knights_cnt * weights.knight +
           bishops_cnt * weights.bishop +
           rooks_cnt * weights.rook +
           pawns_cnt * weights.pawn);
}

// Award developed pawns
inline float pawn_development(const Game &game, Game::Team team) {
  const auto PAWN_ORIGINS =
      team == Game::Team::Black ? Bitboard::Row7 : Bitboard::Row2;
  const auto occ =
      team == Game::Team::Black ? game.positions.blacks : game.positions.whites;
  auto pawns = occ & game.positions.pawns;

  float score = 0;

  while (pawns.count()) {
    auto bit = pawns.trailing_zeroes();
    Bitboard mask = 1ULL << bit;
    pawns ^= mask;

    // Calculate the distance from the mask to the home row
    if (team == Game::Team::White && !(mask & PAWN_ORIGINS)) {

      // Developed by one square
      if (mask.down() & PAWN_ORIGINS)
        score += 1;
      // Developed by two squares
      else if (mask.down<2>() & PAWN_ORIGINS) {
        score += 4;
      }
      // 3 squares
      else if (mask.down<3>() & PAWN_ORIGINS) {
        score += 8;
        // 4 squares, diminishing returns
      } else if (mask.down<4>() & PAWN_ORIGINS) {
        score += 12;
      }
    } else if (team == Game::Team::Black && !(mask & PAWN_ORIGINS)) {
      // Developed by one square
      if (mask.up() & PAWN_ORIGINS)
        score += 1;
      // Developed by two squares
      else if (mask.up<2>() & PAWN_ORIGINS) {
        score += 4;
      }
      // 3 squares
      else if (mask.up<3>() & PAWN_ORIGINS) {
        score += 8;
        // 4 squares, diminishing returns
      } else if (mask.up<4>() & PAWN_ORIGINS) {
        score += 12;
      }
    }
  }

  return score;
}

// Award good piece positioning
inline float positioning(const Game &game, Game::Team team, PerPiece weights) {

  using namespace piece_square_tables;
  const Bitboard occ =
      team == Game::Team::White ? game.positions.whites : game.positions.blacks;

  // Calculate position bitboards
  auto pawns = occ & game.positions.pawns;
  auto rooks = occ & game.positions.rooks;
  auto bishops = occ & game.positions.bishops;
  auto queens = occ & game.positions.queens;
  auto knights = occ & game.positions.knights;
  auto kings = occ & game.positions.kings;

  // Keep track of cumulative scoring for each piece
  float pawns_accum = 0;
  float rooks_accum = 0;
  float bishops_accum = 0;
  float queens_accum = 0;
  float knights_accum = 0;
  float kings_accum = 0;

  bool is_endgame = game.stage == Game::GameStage::Endgame;
  bool is_black = team == Game::Team::Black;

  // Macro to remove unnecessary repetition of the table lookup logic
#define addscores(index, piecename)                                            \
  if (is_endgame && is_black)                                                  \
    piecename##s_accum += endgame::piecename##_table_black[index];             \
  else if (is_endgame)                                                         \
    piecename##s_accum += endgame::piecename##_table_white[index];             \
  else if (is_black)                                                           \
    piecename##s_accum += midgame::piecename##_table_black[index];             \
  else                                                                         \
    piecename##s_accum += midgame::piecename##_table_white[index];

  // 1: Pawn scoring
  while (pawns.count()) {
    auto bit = pawns.trailing_zeroes();
    pawns ^= 1ULL << bit;
    addscores(bit, pawn);
  }

  // 2: Rook scoring
  while (rooks.count()) {
    auto bit = rooks.trailing_zeroes();
    rooks ^= 1ULL << bit;
    addscores(bit, rook);
  }
  // 3: Bishop scoring
  while (bishops.count()) {
    auto bit = bishops.trailing_zeroes();
    bishops ^= 1ULL << bit;
    addscores(bit, bishop);
  }
  // 4: Knight Scoring
  while (knights.count()) {
    auto bit = knights.trailing_zeroes();
    knights ^= 1ULL << bit;
    addscores(bit, knight);
  }
  // 5: King Scoring
  while (kings.count()) {
    auto bit = kings.trailing_zeroes();
    kings ^= 1ULL << bit;
    addscores(bit, king);
  }
  // 6: Queen Scoring
  while (queens.count()) {
    auto bit = queens.trailing_zeroes();
    queens ^= 1ULL << bit;
    addscores(bit, queen);
  }
  return pawns_accum * weights.pawn +
         kings_accum * weights.king+
         queens_accum * weights.queen +
         rooks_accum * weights.rook +
         bishops_accum * weights.bishop +
         knights_accum * weights.knight;
}

inline float material_value(const Game &game, Game::Team team, PerPiece weights) {
  const auto occ =
      team == Game::Team::White ? game.positions.whites : game.positions.blacks;

  float queen_val =
      (occ & game.positions.queens).count() * weights.queen;
  float bishop_val =
      (occ & game.positions.bishops).count() * weights.bishop;
  float rook_val =
      (occ & game.positions.rooks).count() * weights.rook;
  float knight_val =
      (occ & game.positions.knights).count() * weights.knight;

  float pawn_val =
      (occ & game.positions.pawns).count() * weights.pawn;

  return queen_val + bishop_val + rook_val + knight_val + pawn_val;
}

// Award having more material than the other team
inline float material_advantage(const Game &game, Game::Team team, PerPiece weights) {
  if (team == Game::Team::White)
    return material_value(game, Game::Team::White, weights) -
           material_value(game, Game::Team::Black, weights);
  else
    return material_value(game, Game::Team::Black, weights) -
           material_value(game, Game::Team::White, weights);
}

static std::array<Bitboard, 64> white_king_front_pawns = []() constexpr{
    std::array<Bitboard, 64> arr;
    for(int i = 0; i < 64; i++){
        auto col = i % 8;
        Bitboard board = 1ULL << i;
        Bitboard pawns = 0;
        pawns |= board.up();
        if(col != 0){
            pawns |= pawns.left();
        }
        if(col != 7){
            pawns |= pawns.right();
        }
        arr[i] = pawns;
    }
    return arr;
}();
    static std::array<Bitboard, 64> black_king_front_pawns = []() constexpr{
        std::array<Bitboard, 64> arr;
        for(int i = 0; i < 64; i++){
            auto col = i % 8;
            Bitboard board = 1ULL << i;
            Bitboard pawns = 0;
            pawns |= board.down();
            if(col != 0){
                pawns |= pawns.left();
            }
            if(col != 7){
                pawns |= pawns.right();
            }
            arr[i] = pawns;
        }
        return arr;
    }();
inline float king_front_pawns(const Game& game, Game::Team team){
    if(team == Game::Team::White){
        auto king_bit = game.positions.kings & game.positions.whites;
        return (white_king_front_pawns[king_bit.trailing_zeroes()] & game.positions.whites & game.positions.pawns).count();
    }
    else{
        auto king_bit = game.positions.kings & game.positions.whites;
        return (black_king_front_pawns[king_bit.trailing_zeroes()] & game.positions.blacks & game.positions.pawns).count();

    }
}

const std::vector<float> center_ctrl_scores = {
        0, 0, 0,  0, 0,  0, 0, 0,
        0, 0, 0,  2, 2,  0, 0, 0,
        0, 0, 2,  6, 6,  2, 0, 0,
        0, 2, 6, 10, 10, 6, 2, 0,
        0, 2, 6, 10, 10, 6, 2, 0,
        0, 0, 2,  6, 6,  2, 0, 0,
        0, 0, 0,  2, 2,  0, 0, 0,
        0, 0, 0,  0, 0,  0, 0, 0,
};
// Number of center squares
inline float center_control(const Game &game, Game::Team team) {
    // A square is controlled if it is either occupied or exclusively attacked
    if(team == Game::Team::White){
        float total_score = 0;
        auto exclusive_attacks = game.pseudo_attack_board<Game::Team::White>() & ~game.pseudo_attack_board<Game::Team::Black>();
        auto occupation = game.positions.whites;
        auto controlled_squares = occupation | exclusive_attacks;

        FOR_BIT(controlled_squares, {
            total_score += center_ctrl_scores[bit.trailing_zeroes()];
        });
        return total_score;
    }
    else{
        float total_score = 0;
        auto exclusive_attacks = game.pseudo_attack_board<Game::Team::Black>() & ~game.pseudo_attack_board<Game::Team::White>();
        auto occupation = game.positions.blacks;
        auto controlled_squares = occupation | exclusive_attacks;

        FOR_BIT(controlled_squares, {
            total_score += center_ctrl_scores[bit.trailing_zeroes()];
        });
        return total_score;
    }
}

inline float covered_pieces(const Game& game, Game::Team team, PerPiece weights){
    auto covered = team == Game::Team::White ? game.positions.whites & game.pseudo_attack_board<Game::Team::White>() : game.positions.blacks & game.pseudo_attack_board<Game::Team::Black>();
    int sum = 0;
    sum += (covered & game.positions.kings).count() * weights.king;
    sum += (covered & game.positions.queens).count() * weights.queen;
    sum += (covered & game.positions.rooks).count() * weights.rook;
    sum += (covered & game.positions.knights).count() * weights.knight;
    sum += (covered & game.positions.bishops).count() * weights.bishop;
    sum += (covered & game.positions.pawns).count() * weights.pawn;
    return sum;
}

}; // namespace chess::evaluators



// namespace chess::evaluators
