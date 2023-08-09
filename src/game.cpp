#include "./agents/weighted.h"
#include "game.tcc"
#include "./agent.h"
#include "zobrist.h"
#include "evaluate.h"
#include<iostream>
using namespace chess;
#define FOR_BIT(board, exec)                                                   \
  for (Bitboard bit = 1; bit; bit <<= 1) {                                     \
    if (!(board & bit)) {                                                      \
      continue;                                                                \
    } else {                                                                   \
      exec                                                                     \
    }                                                                          \
  }


zobrist::Hash enpassant_hash(Bitboard pos){
    if(pos.empty())return 0;
    switch (pos.trailing_zeroes() / 8) {
        case 0:
            return zobrist::enpassant_row1;
        case 1:
            return zobrist::enpassant_row2;
        case 2:
            return zobrist::enpassant_row3;
        case 3:
            return zobrist::enpassant_row4;
        case 4:
            return zobrist::enpassant_row5;
        case 5:
            return zobrist::enpassant_row6;
        case 6:
            return zobrist::enpassant_row7;
        case 7:
            return zobrist::enpassant_row8;
        default: return 0;
    }
}

bool Game::is_stalemate() const {
    //
    // Criteria for stalemate:
    // 1. Current side's king is not in check
    // 2. No legal moves remain for the current team
    //
    // auto current_team = current_active_team();
    return (!is_checked<Game::Team::White>() && !has_attack<Game::Team::White>()) || (!is_checked<Game::Team::Black>() &&
                                                                                      !has_attack<Game::Team::Black>());
};


void Game::remove_piece(Piece piece) {
    // Remove from color board
    if (piece.team == Team::White) {
        positions.whites ^= piece.position;
    } else
        positions.blacks ^= piece.position;

    // Remove from piece board
#define ZOB(piecekind) \
    if(piece.team == Team::White)zobrist_hash ^= zobrist:: white_##piecekind [piece.position.trailing_zeroes()]; \
    else zobrist_hash ^= zobrist:: black_##piecekind [piece.position.trailing_zeroes()];

    switch (piece.kind) {
        case PieceKind::Pawn:
            positions.pawns ^= piece.position;
            ZOB(pawns)
            break;
        case PieceKind::Rook:
            positions.rooks ^= piece.position;
            ZOB(rooks)
            // Invalidate Castling
            // We dont need to check for teams because if an enemy rook occupies the
            // current rooks position castling would have to be illegal anyway
            if (piece.position == (Bitboard::Row1 & Bitboard::Col1) && castle.wqs) {
                zobrist_hash ^= zobrist::white_queenside;
                castle.wqs = false;
            } else if (piece.position == (Bitboard::Row1 & Bitboard::Col8) && castle.wks) {
                zobrist_hash ^= zobrist::white_kingside;
                castle.wks = false;
            } else if (piece.position == (Bitboard::Row8 & Bitboard::Col1) && castle.bqs) {
                zobrist_hash ^= zobrist::black_queenside;
                castle.bqs = false;
            } else if (piece.position == (Bitboard::Row8 & Bitboard::Col8) && castle.bks) {
                zobrist_hash ^= zobrist::black_kingside;
                castle.bks = false;
            }
            break;
        case PieceKind::Knight:
            positions.knights ^= piece.position;
            ZOB(knights)
            break;
        case PieceKind::Queen:
            positions.queens ^= piece.position;
            ZOB(queens)
            break;
        case PieceKind::Bishop:
            positions.bishops ^= piece.position;
            ZOB(bishops)
            break;
        case PieceKind::King:
            positions.kings ^= piece.position;
            ZOB(kings)
            break;
    }
#undef ZOB
    // We have to recalculate all of the cached pieces
    // because many moves are dependant on other pieces
    // positioning (i.e rooks)
    cached_pieces = 0;
}

void Game::add_piece(Bitboard pos, PieceKind kind, Team team) {
    if (team == Team::Black)
        positions.blacks |= pos;
    else
        positions.whites |= pos;
#define ZOB(piecekind) \
    if(team == Team::White)zobrist_hash ^= zobrist:: white_##piecekind [pos.trailing_zeroes()]; \
    else zobrist_hash ^= zobrist:: black_##piecekind [pos.trailing_zeroes()];

    switch (kind) {
        case PieceKind::King:
            positions.kings |= pos;
            ZOB(kings)
            break;
        case PieceKind::Queen:
            positions.queens |= pos;
            ZOB(queens)
            break;
        case PieceKind::Bishop:
            positions.bishops |= pos;
            ZOB(bishops)
            break;
        case PieceKind::Knight:
            positions.knights |= pos;
            ZOB(knights)
            break;
        case PieceKind::Rook:
            positions.rooks |= pos;
            ZOB(rooks)
            break;
        case PieceKind::Pawn:
            ZOB(pawns)
            positions.pawns |= pos;
            break;
    }
}


void Game::pretty_print(Bitboard highlight) const {
    // std::wcout.sync_with_stdio(false);
    // std::wcout.imbue(std::locale("en_IE.utf8"));
    const auto BLACK = 'a'-'A';
    const auto WHITE = 0x0;

    const auto KING = 'K';
    const auto QUEEN = 'Q';
    const auto ROOK = 'R';
    const auto BISHOP = 'B';
    const auto KNIGHT = 'N';
    const auto PAWN = 'P';
    // const_cast<Game &>(*this).cached_pieces = 0;
    for (uint32_t i = 0; i < 64; i++) {

        auto row = 7 - i / 8;
        auto col = i % 8;
        // std::cout << col << ", " << row << std::endl;
        Bitboard board((1ULL << (8 * row)) << col);
        auto piece = fetch_piece(board);

        // std::cout << board.str() << std::endl;
        bool is_black = (row + col) % 2 != 0;
        bool is_hl = (bool) (board & highlight);
        std::string ansi_str = "\033[0;30;1";
        // std::cout << board.str() << std::endl;
        if (is_black && is_hl) {
            ansi_str+=";101";
        } else if (is_black) {
            ansi_str+=";107";
        } else if (is_hl) {
            ansi_str+=";104";
        } else {
            ansi_str+=";106";
        }
        if(piece && piece->team == Team::Black)ansi_str+=";4";
        ansi_str+="m";
        std::cout << ansi_str;
        std::cout.flush();
        if (piece) {

            char charcode = piece->team == Team::Black ? BLACK : WHITE;
            switch (piece->kind) {
                case PieceKind::King:

                    charcode += KING;
                    break;
                case PieceKind::Queen:
                    charcode += QUEEN;
                    break;
                case PieceKind::Rook:
                    charcode += ROOK;
                    break;
                case PieceKind::Knight:
                    charcode += KNIGHT;
                    break;
                case PieceKind::Pawn:
                    charcode += PAWN;
                    break;
                case PieceKind::Bishop:
                    charcode += BISHOP;
                    break;
            }

            std::wcout << charcode << " ";
            std::wcout.flush();

        } else {
            std::cout << "  ";
        }
        std::cout << "\033[0;0m";
        if (board & Bitboard::Col8)
            std::cout << "\n";
        std::cout.flush();
    }

    std::cout << std::endl;
}

std::vector <Game::Move> Game::movelist(Team team) const {
    auto bb = team == Team::Black ? positions.blacks : positions.whites;
    auto enemybb = team == Team::Black ? positions.whites : positions.blacks;

//    std::cout << bb.str() << "\n"<< enemybb.str() << std::endl;
    std::vector <Game::Move> moves;

    // Put capturing moves before other moves
    // Optimization for the minmax algorithm
    // pruning should generally happen sooner
    std::vector <Game::Move> capturing_moves;

    FOR_BIT(bb, {
        auto piece_pos = bit;
        auto piece = fetch_piece(piece_pos);
        FOR_BIT(piece->pseudolegal_moves, {

                Game::Move m;
                m.piece = piece;
                m.target_pos = bit;
                m.source_pos = piece_pos;
                m.kind = (bit & enemybb) ? Game::Move::MoveType::Capture
                : Game::Move::MoveType::Regular;
                if (is_legal_move(*this, *piece, bit)) {
                    if (piece->kind == PieceKind::Pawn) {
                        // Doublejump is equivalent to moving up/down 16 bits
                        if ((piece->position.up<2>()) == bit ||
                            (piece->position.down<2>()) == bit) {
                            m.kind = Game::Move::Doublejump;
                        } else if (bit & enpassant) {
                            m.kind = Game::Move::Enpassant;
                        } else if (bit & (Bitboard::Row1 | Bitboard::Row8)) {
                            m.kind = Game::Move::PromoteBishop;
                            moves.push_back(m);
                            m.kind = Game::Move::PromoteKnight;
                            moves.push_back(m);
                            m.kind = Game::Move::PromoteQueen;
                            moves.push_back(m);
                            m.kind = Game::Move::PromoteRook;
                            moves.push_back(m);
                            continue;
                        }
                    }
                    if (m.kind == Game::Move::MoveType::Capture ||
                        m.kind == Game::Move::MoveType::Enpassant) {
                        capturing_moves.push_back(m);
                    } else {
                        moves.push_back(m);
                    }
                }
        });

        if (piece->kind == PieceKind::King) {
            auto defense = piece->team == Team::Black ? this->danger_board<Game::Team::Black>() : danger_board<Game::Team::White>();
            auto can_kingside = can_castle_kingside(
                    piece->team, positions.blacks | positions.whites, defense,
                    piece->team == Team::Black ? castle.bks : castle.wks);
            auto can_queenside = can_castle_queenside(
                    piece->team, positions.blacks | positions.whites, defense,
                    piece->team == Team::Black ? castle.bqs : castle.wqs);

            Bitboard row =
                    piece->team == Team::White ? Bitboard::Row1 : Bitboard::Row8;

            if (can_kingside) {
                Game::Move m;
                m.piece = piece;
                m.source_pos = piece->position;
                m.target_pos = row & Bitboard::Col7;
                m.kind = Move::MoveType::KingsideCastle;
                moves.push_back(m);
            }
            if (can_queenside) {
                Game::Move m;
                m.piece = piece;
                m.source_pos = piece->position;
                m.target_pos = row & Bitboard::Col3;
                m.kind = Move::MoveType::QueensideCastle;
                moves.push_back(m);
            }
        }
    });

    for (auto m: moves) {
        capturing_moves.push_back(m);
    }
    return capturing_moves;
}

std::string Game::simple_fen() const {
    std::string ret;

    int blank_cnt = 0;
    for (uint64_t i = 0; i < 64; i++) {
        auto row = i / 8;
        auto col = 7 - i % 8;
        auto BOARD = 1ULL << (63 - (row * 8 + col));
        if (i % 8 == 0) {
            if (blank_cnt) {
                ret += std::to_string(blank_cnt);
                blank_cnt = 0;
            }
            ret += "/";
        }
        auto piece = fetch_piece(BOARD);

        if (piece) {
            if (blank_cnt) {
                ret += std::to_string(blank_cnt);
                blank_cnt = 0;
            }

            char c;
            switch (piece->kind) {
                case PieceKind::Pawn:
                    c = 'p';
                    break;
                case PieceKind::King:
                    c = 'k';
                    break;
                case PieceKind::Knight:
                    c = 'n';
                    break;
                case PieceKind::Bishop:
                    c = 'b';
                    break;
                case PieceKind::Queen:
                    c = 'q';
                    break;
                case PieceKind::Rook:
                    c = 'r';
                    break;
            }
            if (piece->team == Team::White)
                c += 'A' - 'a';

            ret.push_back(c);
        } else {
            blank_cnt++;
        }
    }
    if (blank_cnt) {
        ret += std::to_string(blank_cnt);
    }

    if (state == State::WhiteToMove) {
        ret += " w";
    } else if (state == State::BlackToMove) {
        ret += " b";
    } else if (state == State::BlackWins) {
        ret += " w";
    } else if (state == State::WhiteWins) {
        ret += " b";
    } else
        ret += " ?";

    ret += " ";
    if (castle.wks || castle.wqs || castle.bks || castle.bqs) {
        if (castle.wks)
            ret += "K";
        if (castle.wqs)
            ret += "Q";
        if (castle.bks)
            ret += "k";
        if (castle.bqs)
            ret += "q";
    } else
        ret += "-";

    if (enpassant) {
        ret += " " + enpassant.standard_notation();
    } else
        ret += " -";

    ret += " " + std::to_string(halfmoves);
    ret += " " + std::to_string(fullmoves);
    return ret.substr(1);
}

//
// Create a new game with an FEN string
//
Game Game::create(std::string fen_str) {
    uint8_t row = 7;
    uint8_t col = 0;

    Game game;

    // STEP 1: Positions

    // Helper to prevent code duplication
#define flip(forwhite, forblack, board)                                        \
  case forwhite:                                                               \
    game.positions.whites |= Bitboard((1ULL << col) << (row * 8));             \
    game.positions.board |= Bitboard((1ULL << col) << (row * 8));              \
    col++;                                                                     \
    break;                                                                     \
  case forblack:                                                               \
    game.positions.blacks |= Bitboard((1ULL << col) << (row << 3));            \
    game.positions.board |= Bitboard((1ULL << col) << (row << 3));             \
    col++;                                                                     \
    break;
    uint32_t idx = 0;
    for (auto c: fen_str) {
        switch (c) {
            // Go to the next row
            case '/':
                row--;
                col = 0;
                break;

                // Skip n columns
            case '1' ... '9':
                col += c - '0';
                break;

            flip('R', 'r', rooks);
            flip('P', 'p', pawns);
            flip('K', 'k', kings);
            flip('Q', 'q', queens);
            flip('B', 'b', bishops);
            flip('N', 'n', knights);
            case ' ':
                goto after;
            default:
                throw chess::Error("Unrecognized FEN symbol: " + std::to_string(c));
        }
        idx++;
    }
    after:

    auto active_color = fen_str[idx + 1];
    if (active_color != 'w' && active_color != 'b')
        throw chess::Error("Unrecognized team color");
    game.state = active_color == 'w' ? State::WhiteToMove : State::BlackToMove;

    //
    // Parse castle status
    //
    auto castle_status = fen_str.substr(idx + 3, idx + 8);
    auto end_idx = castle_status.find(' ');
    idx += 3 + end_idx + 1;
    castle_status = castle_status.substr(0, end_idx);
    if (castle_status != "-") {
        for (auto c: castle_status) {
            switch (c) {
                case 'K':
                    game.castle.wks = true;
                    break;
                case 'Q':
                    game.castle.wqs = true;
                    break;
                case 'k':
                    game.castle.bks = true;
                    break;
                case 'q':
                    game.castle.bqs = true;
                    break;
            }
        }
    }
    auto enpassant = fen_str.substr(idx);

    //
    // parse enpassant status
    //
    if (enpassant[0] != '-') {
        uint8_t col = enpassant[1] - 'a';
        uint8_t row = enpassant[2] - '0';
        game.enpassant = (1ULL << col) << (row * 8);
        idx += 5;

    } else
        idx += 2;

    auto halfmove = fen_str.substr(idx);
    auto halfmove_endidx = halfmove.find(' ');
    halfmove = halfmove.substr(0, halfmove_endidx);
    game.halfmoves = std::stoul(halfmove);
    idx += halfmove_endidx + 1;

    auto fullmove = fen_str.substr(idx);
    game.fullmoves = std::stoul(fullmove);
    // game.generate_zobrist_hash();


    // Material count < 40 -> A Capture must have happened
    if((game.positions.whites | game.positions.blacks).count() < 40){
        game.stage = Game::GameStage::MidGame;
    }
    if(game.positions.whites.count() <= 3 || game.positions.blacks.count() <= 3){
        game.stage = Game::GameStage::Endgame;
    }
    return game;
}

// STATUS CODES:
// 0 -> Move was successfully made
// 1 -> Move is for wrong team
// 2 -> Game is in a non-moving state
//
uint8_t Game::make_move(Move m) {
    const uint8_t _EXIT_SUCCESS = 0;
    const uint8_t _EXIT_BAD_TEAM = 1;
    const uint8_t _EXIT_BAD_STATE = 2;
    // The cache gets cleared in this function
    // so its a good idea to copy the piece
    // to prevent UB
    auto piece = *m.piece;
    if(positions.whites & positions.blacks){
        auto& g = *this;
        std::cout << "OVERLAP: " << (positions.whites & positions.blacks).str() << std::endl;
        throw chess::Error("BAD STATE!");
    }

    //
    // Legality Checks
    //

    if (piece.team != current_active_team()) {
        return _EXIT_BAD_TEAM;
    }
    if (state != State::WhiteToMove && state != State::BlackToMove) {
        return _EXIT_BAD_STATE;
    }

    ///////////////////////////////////////////////////////
    /////////// MOVEMENT STATE CHANGES ////////////////////
    ///////////////////////////////////////////////////////

    // Fifty-move rule
    halfmoves++;

    // ENPASSANT

    // Reset enpassant info
    zobrist_hash ^= enpassant_hash(enpassant);
    enpassant = 0;

    if (piece.kind == PieceKind::Pawn) {
        repeatable_states.clear();
        halfmoves = 0;
        if (m.kind == Move::MoveType::Doublejump) {
            // Doublejump / set enpassant value
            if (piece.team == Team::White)
                enpassant = piece.position.up();
            else
                enpassant = piece.position.down();
            zobrist_hash ^= enpassant_hash(enpassant);
        }
    }
    // When the king moves, invalidate all castling
    if (piece.kind == PieceKind::King) {
        if (piece.team == Team::White) {
            if(castle.wks)zobrist_hash ^= zobrist::white_kingside;
            if(castle.wqs)zobrist_hash ^= zobrist::white_queenside;
            castle.wks = false;
            castle.wqs = false;
        } else {
            if(castle.bks)zobrist_hash ^= zobrist::black_kingside;
            if(castle.bqs)zobrist_hash ^= zobrist::black_queenside;
            castle.bks = false;
            castle.bqs = false;
        }
    }

    // One-side invalidation
    if (piece.kind == PieceKind::Rook) {
        const auto WHITE_KS = Bitboard::Row1 & Bitboard::Col8;
        const auto WHITE_QS = Bitboard::Row1 & Bitboard::Col1;

        const auto BLACK_KS = Bitboard::Row8 & Bitboard::Col8;
        const auto BLACK_QS = Bitboard::Row8 & Bitboard::Col1;

        if(piece.position & WHITE_KS && castle.wks){
            zobrist_hash ^= zobrist::white_kingside;
            castle.wks = false;
        }
        else if(piece.position & WHITE_QS && castle.wqs){
            zobrist_hash ^= zobrist::white_queenside;
            castle.wqs = false;
        }
        else if(piece.position & BLACK_KS && castle.bks){
            zobrist_hash ^= zobrist::black_kingside;
            castle.bks = false;
        }
        else if(piece.position & BLACK_QS && castle.bqs){
            zobrist_hash ^= zobrist::black_queenside;
            castle.bqs = false;
        }
    }

    //////////////////////////////////////////////////////
    ///////////// APPLY ANY AUXILLIARY MOVES /////////////
    //////////////////////////////////////////////////////

    // When Capturing
    if (m.kind == Move::MoveType::Capture) {
        // Reset the halfmove counter (fifty-move rule)
        halfmoves = 0;
        repeatable_states.clear();
        // delete the piece on the target square
        auto targ = m.piece->team == Game::Team::Black ? fetch_piece<Game::Team::White>(m.target_pos) : fetch_piece<Game::Team::Black>(m.target_pos);
        remove_piece(*targ);
    }
        // When Castling
    else if (m.kind == Move::MoveType::KingsideCastle ||
             m.kind == Move::MoveType::QueensideCastle) {
        // The row in which the rook resides, depending on the team
        const Bitboard HOME_ROW =
                piece.team == Team::White ? Bitboard::Row1 : Bitboard::Row8;

        // The column in which the rook initially resides, depending on castle side
        const Bitboard ROOK_INITIAL_COL = m.kind == Move::MoveType::KingsideCastle
                                          ? Bitboard::Col8
                                          : Bitboard::Col1;
        // The column in which the rook will reside, depending on castle side
        const Bitboard ROOK_NEW_COL = m.kind == Move::MoveType::KingsideCastle
                                      ? Bitboard::Col6
                                      : Bitboard::Col4;

        const Bitboard ROOK_INITIAL_POS = HOME_ROW & ROOK_INITIAL_COL;
        const Bitboard ROOK_NEW_POS = HOME_ROW & ROOK_NEW_COL;
        const Bitboard REPOSITION_MAP = ROOK_INITIAL_POS | ROOK_NEW_POS;

        // We move the rook to the new square
        if (piece.team == Team::White) {
            positions.whites ^= REPOSITION_MAP;

            // Update the zobrist (remove the old position and add the new one)
            zobrist_hash ^= zobrist::white_rooks[ROOK_INITIAL_POS.trailing_zeroes()];
            zobrist_hash ^= zobrist::white_rooks[ROOK_NEW_POS.trailing_zeroes()];
        } else {
            positions.blacks ^= REPOSITION_MAP;

            zobrist_hash ^= zobrist::black_rooks[ROOK_INITIAL_POS.trailing_zeroes()];
            zobrist_hash ^= zobrist::black_rooks[ROOK_NEW_POS.trailing_zeroes()];
        }
        positions.rooks ^= REPOSITION_MAP;

    }
    // When capturing via enpassant
    if (m.kind == Move::MoveType::Enpassant) {

        // We kill off the piece behind the new position
        auto KILL_BOARD =
                piece.team == Team::White ? m.target_pos.down() : m.target_pos.up();

        // Apply the kill board
        if (piece.team == Team::White) {
            positions.blacks ^= KILL_BOARD;
            zobrist_hash ^= zobrist::white_pawns[KILL_BOARD.trailing_zeroes()];
        } else {
            positions.whites ^= KILL_BOARD;
            zobrist_hash ^= zobrist::black_pawns[KILL_BOARD.trailing_zeroes()];
        }
        positions.pawns ^= KILL_BOARD;
    }

    if (stage == GameStage::Opening) {
        // Move the game into midgame if a capture happens during the opening
        if (m.kind == Move::MoveType::Capture ||
            m.kind == Move::MoveType::Enpassant) {
            stage = GameStage::MidGame;
        }
    } else if (stage == GameStage::MidGame) {
        // Endgame occurs when either team has 3 pieces or less
        if (positions.whites.count() <= 3 || positions.blacks.count() <= 3) {
            stage = GameStage::Endgame;
        }
    }

    // Finally, We can actually apply the move to the piece
    remove_piece(piece);
    auto enemies = current_active_team() == Game::Team::White ? positions.blacks : positions.whites;
    if(m.target_pos & enemies){
        remove_piece(*fetch_piece(m.target_pos));
    }
    // Integrate promotion into the move (optimization)
    if (m.kind == Move::PromoteRook) {


        add_piece(m.target_pos, PieceKind::Rook, piece.team);
    } else if (m.kind == Move::PromoteQueen) {
        add_piece(m.target_pos, PieceKind::Queen, piece.team);
    } else if (m.kind == Move::PromoteKnight) {
        add_piece(m.target_pos, PieceKind::Knight, piece.team);
    } else if (m.kind == Move::PromoteBishop) {
        add_piece(m.target_pos, PieceKind::Bishop, piece.team);
    } else {
        // Classic Move
        add_piece(m.target_pos, piece.kind, piece.team);
    }

    // fivefold repetition
    RepetitionInfo inf;
    inf.castles = castle;
    inf.positions = positions;

    std::vector <RepetitionInfo> &repeats = repeatable_states;

    auto cnt = std::count(repeats.begin(), repeats.end(), inf);

    // Fivefold repetition
    if (cnt == 4) {
        state = Game::State::Stalemate;
        return _EXIT_SUCCESS;
    }

    repeats.push_back(inf);
    // Reset the cache info, so we don't get bugged "ghost" pieces in the next
    // move
    this->cached_pieces = 0;
    ////////////////////////////////////////////////
    //////////// GAME STATE CHANGES ////////////////
    ////////////////////////////////////////////////


    if(is_mated<Team::Black>())this->state = State::WhiteWins;
    else if(is_mated<Team::White>())this->state = State::BlackWins;

    else if (this->is_stalemate()) {
        this->state = State::Stalemate;
    } else if (this->halfmoves == 100) {
        // Halfmove rule has been met
        // Checkmate takes precedence over this rule,
        // so it must be checked for only if mate did
        // not occur
        this->state = State::Stalemate;
        return _EXIT_SUCCESS;
    }


    // Pass the turn to the other team
    if (this->state == State::BlackToMove){
        zobrist_hash ^= zobrist::black_to_move;
        this->state = State::WhiteToMove;
//        generate_zobrist_hash();
    }
    else if (this->state == State::WhiteToMove){
        zobrist_hash ^= zobrist::black_to_move;
        this->state = State::BlackToMove;
//        generate_zobrist_hash();
    }
    if (piece.team == Team::Black) {
        fullmoves++;
    }
    return _EXIT_SUCCESS;
}


Game::Move Game::get_agent_move(const Agent &ag) const {
    auto move = ag.move(*this);
    return move;
}

inline float fast_sigmoid(float x){
    return x / ( 1 + (x < 0 ? -x : x) );
}

float Game::advantage(Game::Team team, agents::Weighted eval) const {
    auto my_eval = evaluate(team, eval);
    auto enemy_eval = evaluate(team == Game::Team::White ? Game::Team::Black : Game::Team::White, eval);

    // Fast Sigmoid
    std::cout << "Eval: " << my_eval << std::endl;
    std::cout << "Enemy: " << enemy_eval << std::endl;
    auto diff = my_eval - enemy_eval;
    return cbrtf(diff);
    std::cout << "Diff: " << diff << std::endl;
    auto sig =  fast_sigmoid(diff);
    std::cout << "Advantage: " << sig << std::endl;
    return sig;
}

//
// A dead-simple game evaluation function
//
// We essentially just fetch a few statistics about the game
// multiply each one by some weight
// and sum them all together
//
// (weighted sum)
//
float Game::evaluate(Team team, agents::Weighted eval) const {
    // TODO: Create a default instance of the weighted evaluator
    //       to generate the evaluation scores
    auto ws = eval.weightedsum(*this, team);
    return sqrt(sqrtf(ws) * log2f(ws));
    // Early-Exit for checkmate and stalemate
//    if (state == State::WhiteWins && team != Team::White)
//        return -INF;
//    else if (state == State::BlackWins && team != Team::Black)
//        return -INF;
//    else if (state == State::WhiteWins && team == Team::White)
//        return INF;
//    else if (state == State::BlackWins && team == Team::Black)
//        return INF;
//    else if (state == State::Stalemate)
//        return 0;
////    return 0;
//    switch (stage) {
//        case GameStage::Opening:
//            return eval_game<GameStage::Opening>(*this, team);
//        case GameStage::MidGame:
//            return eval_game<GameStage::MidGame>(*this, team);
//        case GameStage::Endgame:
//            return eval_game<GameStage::Endgame>(*this, team);
//    }
}
//
// Calculate the Zobrist hash of the board
//
void Game::generate_zobrist_hash() {
    zobrist::Hash hash = 0;

    FOR_BIT(positions.pawns & positions.whites, {
        hash ^= zobrist::white_pawns[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.pawns & positions.blacks, {
        hash ^= zobrist::black_pawns[bit.trailing_zeroes()];
    });

    FOR_BIT(positions.rooks & positions.whites, {
        hash ^= zobrist::white_rooks[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.rooks & positions.blacks, {
        hash ^= zobrist::black_rooks[bit.trailing_zeroes()];
    });

    FOR_BIT(positions.bishops & positions.whites, {
        hash ^= zobrist::white_bishops[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.bishops & positions.blacks, {
        hash ^= zobrist::black_bishops[bit.trailing_zeroes()];
    });

    FOR_BIT(positions.knights & positions.whites, {
        hash ^= zobrist::white_knights[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.knights & positions.blacks, {
        hash ^= zobrist::black_knights[bit.trailing_zeroes()];
    });

    FOR_BIT(positions.queens & positions.whites, {
        hash ^= zobrist::white_queens[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.queens & positions.blacks, {
        hash ^= zobrist::black_queens[bit.trailing_zeroes()];
    });

    FOR_BIT(positions.kings & positions.whites, {
        hash ^= zobrist::white_kings[bit.trailing_zeroes()];
    });
    FOR_BIT(positions.kings & positions.blacks, {
        hash ^= zobrist::black_kings[bit.trailing_zeroes()];
    });

    if (castle.bks)hash ^= zobrist::black_kingside;
    if (castle.bqs)hash ^= zobrist::black_queenside;
    if (castle.wks)hash ^= zobrist::white_kingside;
    if (castle.wqs)hash ^= zobrist::white_queenside;

    if (current_active_team() == Team::Black) hash ^= zobrist::black_to_move;

    hash ^= enpassant_hash(enpassant);


    this->zobrist_hash = hash;
}


Game::Team Game::current_active_team() const {
    if (state == Game::State::WhiteToMove)
        return Team::White;
    else if (state == State::BlackToMove)
        return Team::Black;
    else {
        std::cout << "Attempt to get active team from state: "
                  << std::to_string(state) << std::endl;
        exit(1);
    }
}

Game::Piece *Game::fetch_piece(Bitboard position) const{
    if(current_active_team() == Game::Team::White)return fetch_piece<Game::Team::White>(position);
    else return fetch_piece<Game::Team::Black>(position);
}