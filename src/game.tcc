#pragma once
#include "./pseudolegal_move_calculator.h"
#include "./error.h"
#include "bitboard.h"
#include "internal.h"
#include <algorithm>
#include <iostream>

using namespace chess;

// helper to
// iterate over each bit
// of a bitboard
#define FOR_BIT(board, exec)               \
    for (Bitboard bit = 1; bit; bit <<= 1) \
    {                                      \
        if (!(board & bit))                \
        {                                  \
            continue;                      \
        }                                  \
        else                               \
        {                                  \
            exec                           \
        }                                  \
    }

template <Game::Team TEAM>
Bitboard pseudo_attack_board_internal(const Game &game, bool utilizecache)
{
    // Pseudomoves can deliver check(mate) because the state after the
    // theoretical king capture is irrelevant as the other team has already won
    auto board =
        TEAM == Game::Team::White ? game.positions.whites : game.positions.blacks;

    Bitboard all_attacks;
    auto team = game.current_active_team();

    if(team == Game::Team::White){
    FOR_BIT(board, {
        auto pos = bit;
        // auto piece = utilizecache ? *game.fetch_piece<TEAM>(bit)
        //                           : internal::fetch_piece_nocache<TEAM>(game, bit);
        auto piece = internal::fetch_piece_nocache<Game::Team::White>(game, bit);
        auto moves = piece.pseudolegal_moves;

        all_attacks |= moves;
    });
    }
    else{
    FOR_BIT(board, {
        auto pos = bit;
        // auto piece = utilizecache ? *game.fetch_piece<TEAM>(bit)
        //                           : internal::fetch_piece_nocache<TEAM>(game, bit);
        auto piece = internal::fetch_piece_nocache<Game::Team::Black>(game, bit);
        auto moves = piece.pseudolegal_moves;

        all_attacks |= moves;
    });
    }
    return all_attacks;
}

// Early-return boolified version of the prior function (optimization)
template <Game::Team TEAM>
bool pseudo_attack_board_has_bit_internal(const Game &game,
                                          Bitboard mask, bool utilizecache)
{
    auto board =
        TEAM == Game::Team::White ? game.positions.whites : game.positions.blacks;
    auto team = game.current_active_team();

    if(team == Game::Team::White){
    FOR_BIT(board, {
        auto piece = internal::fetch_piece_nocache<Game::Team::White>(game, bit);

        // auto piece = utilizecache ? *game.fetch_piece<TEAM>(bit)
        //                           : internal::fetch_piece_nocache<TEAM>(game, bit);
        // auto piece = fetch_piece_nocache(game, bit);

        auto moves = piece.pseudolegal_moves;
        if (moves & mask)
            return true;
    });}
    else{
        FOR_BIT(board, {
        auto piece = internal::fetch_piece_nocache<Game::Team::Black>(game, bit);

        // auto piece = utilizecache ? *game.fetch_piece<TEAM>(bit)
        //                           : internal::fetch_piece_nocache<TEAM>(game, bit);
        // auto piece = fetch_piece_nocache(game, bit);

        auto moves = piece.pseudolegal_moves;
        if (moves & mask)
            return true;
    });}
    return false;
}
template <Game::Team TEAM>
bool Game::is_mated() const
{
    //
    // Criteria for checkmate:
    // 1. King is in check.
    // 2. All moves are illegal (team has no attacks)
    //
    return is_checked<TEAM>() && !has_attack<TEAM>();
}
template <Game::Team TEAM>
__attribute__((always_inline)) bool
is_checked_internal(const Game &game, bool utilizecache)
{
    // if (team == Game::Team::Black)
    //   return game.pseudo_attack_board(Game::Team::White) & game.positions.kings
    //   &
    //          game.positions.blacks;
    // else
    //   return game.pseudo_attack_board(Game::Team::Black) & game.positions.kings
    //   &
    //          game.positions.whites;

    if (TEAM == Game::Team::Black)
    {
        auto res = pseudo_attack_board_has_bit_internal<Game::Team::White>(
            game, game.positions.kings & game.positions.blacks,
            utilizecache);
        return res;
    }
    else
    {
        auto res = pseudo_attack_board_has_bit_internal<Game::Team::Black>(
            game, game.positions.kings & game.positions.whites,
            utilizecache);
        return res;
    }
}

template <Game::Team CURRENT_TEAM>
Game::Piece *Game::fetch_piece(Bitboard position) const
{
    auto &pcache = mut().piece_cache;
    auto &pcacheinfo = mut().cached_pieces;
    // TODO:
    // Add an assertion so that position may only contain at most one bit

    auto storage_index = position.trailing_zeroes();
    //     if (cached_pieces & position) {
    //       // std::cout << "Fetch piece from cache" << std::endl;
    //       return &pcache[storage_index];
    //     }

    // Check if the piece exists
    if (!((positions.whites | positions.blacks) & position))
        return nullptr;

    pcache[storage_index] = internal::fetch_piece_nocache<CURRENT_TEAM>(*this, position);

    pcacheinfo |= position;
    return &pcache[storage_index];
}

template <Game::Team TEAM>
bool Game::is_checked() const
{
    return is_checked_internal<TEAM>(*this, true);
}
inline bool is_legal_move(const Game &game, Game::Piece piece, Bitboard target_pos)
{
    auto &m = game.mut();

    // Save the state of the game
    auto initial_state = m.positions;
    auto initial_s = m.state;
    auto bk = game.castle.bks;
    auto bq = game.castle.bqs;
    auto wk = game.castle.wks;
    auto wq = game.castle.wqs;
    auto zob = game.zobrist_hash;

    if(piece.team == Game::Team::White)m.state = Game::State::BlackToMove;
    else m.state = Game::State::WhiteToMove;
    // Delete/Kill the captured piece (when applicable)

    if (piece.kind == Game::PieceKind::Pawn &&
        piece.team == game.current_active_team() && target_pos & game.enpassant)
    {
        // If the move was an enpassant its slightly different

        const auto capture_white = game.enpassant.down();
        const auto capture_black = game.enpassant.up();

        auto cap = game.fetch_piece(
            piece.team == Game::Team::White ? capture_white : capture_black);
        m.remove_piece(*cap);
    }
    else
    {

        auto cap = game.fetch_piece(target_pos);
        if (cap)
        {
            m.remove_piece(*cap);
        }
    }

    // Make the move
    m.remove_piece(piece);
    m.add_piece(target_pos, piece.kind, piece.team);
    // Check for check

    bool legal = piece.team == Game::Team::White ? !is_checked_internal<Game::Team::White>(game, false) : !is_checked_internal<Game::Team::Black>(game, false);

    // Restore the state
    m.positions = initial_state;
    m.castle.wqs = wq;
    m.castle.wks = wk;
    m.castle.bqs = bq;
    m.castle.bks = bk;
    m.zobrist_hash = zob;
    m.state = initial_s;
    return legal;
}

template <Game::Team TEAM>
Bitboard Game::attack_board() const
{
    auto board = TEAM == Team::White ? positions.whites : positions.blacks;
    Bitboard all_attacks;
    // Get each position as a pos-bit
    FOR_BIT(board, {
        auto old_pos = bit;
        auto piece = fetch_piece(bit);
        auto moves = piece->pseudolegal_moves & ~all_attacks;

        FOR_BIT(moves, {
            auto new_pos = bit;
            auto captured_piece = fetch_piece(new_pos);
            if (is_legal_move(*this, *piece, new_pos))
            {
                all_attacks |= new_pos;
            }
        });
    });

    return all_attacks;
}

//
// Returns true if the team has a legal move
//
//
template <Game::Team TEAM>
bool Game::has_attack() const
{
    auto board = TEAM == Team::White ? positions.whites : positions.blacks;

    auto &mutthis = mut();
    // Get each position as a pos-bit
    FOR_BIT(board, {
        auto old_pos = bit;
        auto piece = fetch_piece(bit);

        auto moves = piece->pseudolegal_moves;

        FOR_BIT(moves, {
            auto new_pos = bit;
            auto captured_piece = fetch_piece(new_pos);
            if (is_legal_move(*this, *piece, new_pos))
                return true;
        });
    });
    return false;
}

template <Game::Team TEAM>
Bitboard Game::pseudo_attack_board() const
{
    auto attacks = pseudo_attack_board_internal<TEAM>(*this, true);

    return attacks;
}
template <Game::Team TEAM>
Bitboard Game::danger_board() const
{
    return attack_board < TEAM == Team::White ? Team::Black : Team::White > ();
}
template <Game::Team TEAM>
Bitboard Game::pseudo_danger_board() const
{
    return pseudo_attack_board_internal < TEAM == Team::White ? Team::Black : Team::White > (*this, true);
}

inline bool can_castle_kingside(Game::Team team, Bitboard world, Bitboard attacked,
                                bool castling_rights)
{
    if (!castling_rights)
        return false;

    if (team == Game::Team::White)
    {
        if (!(attacked & Bitboard::WhiteKingsideCastleMustBeSafe) &&
            !(world & Bitboard::WhiteKingsideCastleMustBeEmpty))
            return true;
    }
    else
    {
        if (!(attacked & Bitboard::BlackKingsideCastleMustBeSafe) &&
            !(world & Bitboard::BlackKingsideCastleMustBeEmpty))
            return true;
    }
    return false;
}

inline bool can_castle_queenside(Game::Team team, Bitboard world, Bitboard attacked,
                                 bool castling_rights)
{
    if (!castling_rights)
        return false;

    if (team == Game::Team::White)
    {
        return !(attacked & Bitboard::WhiteQueensideCastleMustBeSafe) &&
               !(world & Bitboard::WhiteQueensideCastleMustBeEmpty);
    }
    else
    {
        return !(attacked & Bitboard::BlackQueensideCastleMustBeSafe) &&
               !(world & Bitboard::BlackQueensideCastleMustBeEmpty);
    }
}

template <Game::Team TEAM>
Bitboard Game::attack_board_incl_castles() const
{
    // The rules for castling are a little annoying.
    // 1. King must not be under attack
    // 2. All positions that the king must cross, must not be under attack
    // 3. All positions between king and rook must not be occupied
    auto attacks = attack_board<TEAM>();
    auto attacked = danger_board<TEAM>();
    auto world = positions.whites | positions.blacks;

    auto can_kingside = can_castle_kingside(
        TEAM, world, attacked, TEAM == Team::White ? castle.wks : castle.bks);
    auto can_queenside = can_castle_queenside(
        TEAM, world, attacked, TEAM == Team::White ? castle.wqs : castle.bqs);

    auto row = TEAM == Team::White ? Bitboard::Row1 : Bitboard::Row8;

    if (can_kingside)
        attacks |= row & Bitboard::Col7;
    if (can_queenside)
        attacks |= row & Bitboard::Col3;
    return attacks;
}
