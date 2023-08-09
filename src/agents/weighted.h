#pragma once
#include "../agent.h"
#include <limits>
#include "../evaluate.h"
#include <string>
#include <map>
#include <sstream>
#include <fstream>
//
// The weighted agent takes the weighted sum of various features of the board
// to construct a final score
//
#define INF std::numeric_limits<float>::infinity()

namespace chess::agents
{

    class Weighted : public chess::Agent
    {
    public:
        struct EvaluatorWeights
        {
            evaluators::PerPiece positions{10, 20, 5, 7, 12, 4};
            evaluators::PerPiece values{4000, 1050, 500, 325, 325, 100};

            evaluators::MultistageEvaluationWeight check{1, 1, 1};
            evaluators::MultistageEvaluationWeight mobility{5, 3, 0};
            evaluators::MultistageEvaluationWeight vulnerability{0.5, 0.32, 0.4};
            evaluators::MultistageEvaluationWeight pawn_devel{30, 10, 25};
            evaluators::MultistageEvaluationWeight positioning{0.02, 0.02, 0.02};
            evaluators::MultistageEvaluationWeight material{0, 0.6, 10};
            evaluators::MultistageEvaluationWeight king_front_pawns{1, 1, 1};
            evaluators::MultistageEvaluationWeight center_control{1, 1, 1};
            evaluators::MultistageEvaluationWeight protected_pieces{1, 1, 1};
        } weights;

        static Weighted from_file(std::string path)
        {
            std::ifstream file(path);

            if (file.bad())
            {
                throw chess::Error("Failed to load weighted agent from file: " + path);
            }
            std::stringstream buf;
            buf << file.rdbuf();
            // std::cout << buf.str() << std::endl;
            return Weighted::decode(buf.str());
        }

        float weightedsum(const Game &game, Game::Team team) const
        {
            if (game.state == Game::State::WhiteWins && team == Game::Team::White)
                return INF;
            else if (game.state == Game::State::WhiteWins && team == Game::Team::Black)
                return -INF;
            else if (game.state == Game::State::BlackWins && team == Game::Team::Black)
                return INF;
            else if (game.state == Game::State::BlackWins && team == Game::Team::White)
                return -INF;
            else if (game.state == Game::State::Stalemate)
                return 0;
//    // Macro to chose weight depending on game stage
#define weight(name)                                                 \
    (game.stage == Game::GameStage::Opening   ? weights.name.opening \
     : game.stage == Game::GameStage::MidGame ? weights.name.midgame \
                                              : weights.name.endgame)

            // Establish the weights
            float w_check = weight(check);
            float w_mobility = weight(mobility);
            float w_vulnerability = weight(vulnerability);
            float w_pawn_devel = weight(pawn_devel);
            float w_positioning = weight(positioning);
            float w_materialadv = weight(material);
            float w_kingfrontpawns = weight(king_front_pawns);
            float w_centercontrol = weight(center_control);
            float w_covered = weight(protected_pieces);
            // Apply the weights to fetched values
            float check = evaluators::check(game, team) * w_check;
            float mobility = evaluators::mobility(game, team) * w_mobility;
            float vulnerability = evaluators::vulnerability(game, team, weights.values) * w_vulnerability;
            float pawn_devel = evaluators::pawn_development(game, team) * w_pawn_devel;
            float positioning = evaluators::positioning(game, team, weights.positions) * w_positioning;
            float material = evaluators::material_advantage(game, team, weights.values) * w_materialadv;
            float king_front_pawns = evaluators::king_front_pawns(game, team) * w_kingfrontpawns;
            float center_control = evaluators::center_control(game, team) * w_centercontrol;
            float covered = evaluators::covered_pieces(game, team, weights.values) * w_covered;

            float misc_contributors = 0;

            // Dock points for early queen movement
            if (game.stage == Game::GameStage::Opening && (team == Game::Team::White ? (game.positions.whites & game.positions.queens != Bitboard::Row1 & Bitboard::Col4) : (game.positions.blacks & game.positions.queens != Bitboard::Row8 & Bitboard::Col4)))
            {
                //        misc_contributors -= weights.queen_early_movement;
            }

            return check + mobility + vulnerability + pawn_devel + positioning + material + king_front_pawns + center_control + covered + misc_contributors;
        }

        float minimax(Game &game, Game::Team ourteam, int depth, float alpha, float beta, bool maximizingplayer) const
        {
            //    std::cout << game.zobrist_hash << std::endl;
            auto enemy =
                ourteam == Game::Team::White ? Game::Team::Black : Game::Team::White;

            //
            // Do a transposition table lookup
            //
            //    if(Game::transposition_table.contains(game.zobrist_hash)){
            //        auto& table_entry = Game::transposition_table[game.zobrist_hash];
            //        // The entry must be at the same depth, or deeper
            //        // this ensures we dont take less accurate evaluations
            //        // from previous iterations
            //        if(table_entry.depth >= depth){
            //            if(table_entry.type == Game::TranspositionTableEntry::Exact){
            //                return table_entry.evaluation;
            //            }
            //            else if(table_entry.type == Game::TranspositionTableEntry::Lowerbound){
            //                alpha = std::max(alpha, table_entry.evaluation);
            //            }
            //            else if(table_entry.type == Game::TranspositionTableEntry::Upperbound){
            //                beta = std::min(beta, table_entry.evaluation);
            //            }
            //            if(alpha > beta){
            //                return table_entry.evaluation;
            //            }
            //        }
            //    }

            if (depth == 0 || (game.state != Game::State::BlackToMove &&
                               game.state != Game::State::WhiteToMove))
            {
                auto eval = weightedsum(game, ourteam) / weightedsum(game, enemy);
                //        Game::transposition_table[game.zobrist_hash].depth = depth;
                //        Game::transposition_table[game.zobrist_hash].evaluation = eval;
                //        Game::transposition_table[game.zobrist_hash].type = Game::TranspositionTableEntry::Exact;

                return eval;
            }

            if (maximizingplayer)
            {
                float maxeval = -INF;
                for (auto move : game.movelist(ourteam))
                {
                    auto newgame = game;
                    newgame.make_move(move);
                    auto eval = minimax(newgame, ourteam, depth - 1, alpha, beta, false);
                    maxeval = std::max(maxeval, eval);
                    alpha = std::max(alpha, eval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
                //        Game::transposition_table[game.zobrist_hash].depth = depth;
                //        Game::transposition_table[game.zobrist_hash].evaluation = maxeval;
                return maxeval;
            }
            else
            {
                float mineval = INF;
                for (auto move : game.movelist(enemy))
                {
                    auto newgame = game;
                    newgame.make_move(move);
                    auto eval = minimax(newgame, ourteam, depth - 1, alpha, beta, true);
                    mineval = std::min(mineval, eval);
                    beta = std::min(beta, eval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
                //        Game::transposition_table[game.zobrist_hash].depth = depth;
                //        Game::transposition_table[game.zobrist_hash].evaluation = mineval;
                return mineval;
            }
        }

        [[nodiscard]] std::map<std::string, const evaluators::EvalParameter *> PARAMETERS() const
        {
            std::map<std::string, const evaluators::EvalParameter *> ret;
            ret["check"] = &weights.check;
            ret["material"] = &weights.material;
            ret["positioning"] = &weights.positioning;
            ret["mobility"] = &weights.mobility;
            ret["pawn_devel"] = &weights.pawn_devel;
            ret["vulnerability"] = &weights.vulnerability;
            ret["values"] = &weights.values;
            ret["positions"] = &weights.positions;
            ret["king_front_pawns"] = &weights.king_front_pawns;
            ret["center_control"] = &weights.center_control;
            ret["protected_pieces"] = &weights.protected_pieces;
            return ret;
        }
        //
        // Encode the current weights into a simple string which can be stored in a document
        //
        std::string encode() const
        {
            // FORMAT:
            // PROP_NAME;DATA_TYPE;...VALUES\n
            //
            // DATA_TYPE can be one of:
            // S -> Single Value
            // M -> Multistage Weight
            // P -> Perpiece Weight (KING,QUEEN,ROOK,BISHOP,KNIGHT,PAWN)
            std::string data;
            auto params = this->PARAMETERS();
            for (const auto &pair : params)
            {
                data += pair.first + ";" + pair.second->encode() + "\n";
            }

            return data;
        }

        static agents::Weighted decode(std::string data)
        {

            int last_idx = -1;
            int idx = 0;
            agents::Weighted agent;
            auto _parameters = agent.PARAMETERS();
            auto params = *(std::map<std::string, evaluators::EvalParameter *> *)(&_parameters);
            auto lines = evaluators::split(data, "\n");
            for (auto l : lines)
            {
                auto first_split = l.find(';');
                auto second_split = l.find(';', first_split + 1);

                auto param_name = l.substr(0, first_split);
                auto data_type = l.substr(first_split + 1, second_split - first_split)[0];
                auto data = l.substr(second_split + 1);
                switch (data_type)
                {
                case 'P':
                    *(evaluators::PerPiece *)params[param_name] = evaluators::PerPiece::from_str(data);
                    break;
                case 'M':
                    *(evaluators::MultistageEvaluationWeight *)params[param_name] = evaluators::MultistageEvaluationWeight::from_str(data);
                    break;
                }
                last_idx = idx;
            }
            return agent;
        }

        int search_depth = 6;

        Game::Move move(const Game &g) const override
        {
            Game::Move bestmove;
            float bestmovescore = -INF;
            auto all_moves = g.movelist(g.current_active_team());

            // Handle the case where all evaluations lead to mate
            // Floating point comparison -inf > -inf will return false otherwise
            // and the engine will crash
            if (all_moves.size() > 0)
                bestmove = all_moves[0];
            for (auto move : all_moves)
            {
                auto newgame = g;
                newgame.make_move(move);
                auto score = minimax(newgame, g.current_active_team(), search_depth - 1, -INF,
                                     INF, false);
                if (score > bestmovescore)
                {
                    bestmove = move;
                    bestmovescore = score;
                }
            }
            //    std::cout << "=== Chose Move: " << bestmove.str() << std::endl;
            return bestmove;
        }
    };
};
