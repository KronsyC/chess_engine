#!/usr/bin/env python

import time
import bindings as chess
from flask import Flask, render_template, redirect
from ctypes import *
import webbrowser
import threading
# OPENING_FEN = "k4r2/8/8/8/8/8/8/K3R3 w KQkq - 0 1".encode('utf-8')


app = Flask(__name__, static_folder="static", static_url_path="")

global ACTIVE_GAME
ACTIVE_GAME = None

def fetch_game_weights():
    return {}
    return {
        "king_positioning": chess.wkingpos.get(ACTIVE_GAME),
        "queen_positioning": chess.wqueenpos.get(ACTIVE_GAME),
        "rook_positioning": chess.wrookpos.get(ACTIVE_GAME),
        "knight_positioning": chess.wknightpos.get(ACTIVE_GAME),
        "bishop_positioning": chess.wbishoppos.get(ACTIVE_GAME),
        "pawn_positioning": chess.wpawnpos.get(ACTIVE_GAME),

        "king_val": chess.wkingval.get(ACTIVE_GAME),
        "queen_val": chess.wqueenval.get(ACTIVE_GAME),
        "rook_val": chess.wrookval.get(ACTIVE_GAME),
        "knight_val":chess.wknightval.get(ACTIVE_GAME),
        "bishop_val":chess.wbishopval.get(ACTIVE_GAME),
        "pawn_val": chess.wpawnval.get(ACTIVE_GAME),

        "op.check": chess.wcheck_op.get(ACTIVE_GAME),
        "mg.check": chess.wcheck_mg.get(ACTIVE_GAME),
        "eg.check": chess.wcheck_eg.get(ACTIVE_GAME),

        "op.pawndevel": chess.wpawndevel_op.get(ACTIVE_GAME),
        "mg.pawndevel": chess.wpawndevel_mg.get(ACTIVE_GAME),
        "eg.pawndevel": chess.wpawndevel_eg.get(ACTIVE_GAME),

        "op.vulnerability": chess.wvulnerability_op.get(ACTIVE_GAME),
        "mg.vulnerability": chess.wvulnerability_mg.get(ACTIVE_GAME),
        "eg.vulnerability": chess.wvulnerability_eg.get(ACTIVE_GAME),

        "op.mobility": chess.wmobility_op.get(ACTIVE_GAME),
        "mg.mobility": chess.wmobility_mg.get(ACTIVE_GAME),
        "eg.mobility": chess.wmobility_eg.get(ACTIVE_GAME),

        "op.positioning": chess.wpositioning_op.get(ACTIVE_GAME),
        "mg.positioning": chess.wpositioning_mg.get(ACTIVE_GAME),
        "eg.positioning": chess.wpositioning_eg.get(ACTIVE_GAME),

        "op.material": chess.wmaterial_op.get(ACTIVE_GAME),
        "mg.material": chess.wmaterial_mg.get(ACTIVE_GAME),
        "eg.material": chess.wmaterial_eg.get(ACTIVE_GAME),

    }

def fetch_game_pieces(team, ALL_ATTACKS):
    DATA = []
    for i in range(64):
        row = int(i // 8)
        col = i % 8

        piece = chess.find_piece(ACTIVE_GAME, row, col)

        if piece:
            piece_kind = chess.piece_kind(piece)
            piece_team = chess.piece_team(piece)
            if piece_team != team: continue

            kind_name = chess.kind_name(piece_kind)
            obj = {}
            obj["attacks"] =  [a for a in ALL_ATTACKS if a["from"] == (row*8+col)];
            obj["kind"] = kind_name
            obj["pos"] = row*8 + col
            DATA.append(obj)

            chess.delete_piece(piece)
    return DATA

def move_to_dict(m):
    return {
            "from": chess.move_origin(m),
            "to": chess.move_destination(m),
            "kind": chess.move_kind(m)
            }

def construct_game_info():
    global ACTIVE_GAME
    # ACTIVE_GAME = chess.create_game(OPENING_FEN);
    if not ACTIVE_GAME: return None

    state = chess.get_state(ACTIVE_GAME)

    chess.print_game(ACTIVE_GAME)
    movelist = None

    if state == 0 or state == 1:
        movelist = chess.get_team_moves(ACTIVE_GAME, chess.get_team(ACTIVE_GAME))

    white_score = chess.evaluate_game(ACTIVE_GAME, chess.Team.White, chess.DEFAULT_WEIGHTED_AGENT)
    black_score = chess.evaluate_game(ACTIVE_GAME, chess.Team.Black, chess.DEFAULT_WEIGHTED_AGENT)

    white_advantage = chess.game_advantage(ACTIVE_GAME, chess.Team.White, chess.DEFAULT_WEIGHTED_AGENT)
    black_advantage = chess.game_advantage(ACTIVE_GAME, chess.Team.Black, chess.DEFAULT_WEIGHTED_AGENT)

    
    moves = []
    if movelist:
        for i in range(movelist.count):
            d = move_to_dict(movelist.data[i])
            moves.append(d)
            chess.delete_move(movelist.data[i])

    white_pieces = fetch_game_pieces(chess.Team.White, moves)
    black_pieces = fetch_game_pieces(chess.Team.Black, moves)

    
    return {
        "state" : state,
        "halfmove_count" : chess.halfmoves(ACTIVE_GAME),
        "fullmove_count" : chess.fullmoves(ACTIVE_GAME),
        "whites": white_pieces,
        "blacks": black_pieces,
        "white_score": white_score,
        "black_score": black_score,
        "white_advantage": white_advantage,
        "black_advantage": black_advantage,
        "weights": fetch_game_weights(),
    }

def bitboard_to_chars(bitboard):
    data = ""
    for i in range(8):
        SHIFT = i * 8
        BITS = (bitboard >> SHIFT) & 0x00000000000000FF
        char = chr(BITS)
        data += char
    return data
@app.route("/")
def homepage():
    return redirect("index.html")

@app.route("/game/new")
def new_game():
    print("NEW GAME, TERMINATING OLD ONE")
    game = chess.create_game(chess.OPENING_FEN)
    if not game:
        return "Failed to create a game: Bad FEN string", 400
    global ACTIVE_GAME

    if(ACTIVE_GAME):
        chess.delete_game(ACTIVE_GAME)
    ACTIVE_GAME = game
    return "Successfully created a new game"

@app.route("/game/move/<move>")
def create_move(move):
    [fro, to, kind] = [int(p) for p in move.split(".")]

    moves = chess.get_team_moves(ACTIVE_GAME, chess.get_team(ACTIVE_GAME))

    MOVE = None
    for i in range(moves.count):
        move = moves.data[i]
        mk = chess.move_kind(move)
        mo = chess.move_origin(move)
        md = chess.move_destination(move)


        if mk == kind and mo == fro and md == to:
            MOVE = move
            break


    if not MOVE: return "Failed to find move", 400

    
    chess.move(ACTIVE_GAME, MOVE)

    return "success"

@app.route("/game/info")
def game_info():
    info =  construct_game_info()
    if info: return info
    else: return "You must initialize the game", 400

@app.route("/game/best_move")
def game_best_move():
    # return "AA", 400
    best_move = chess.get_agent_move(ACTIVE_GAME, chess.DEFAULT_WEIGHTED_AGENT)
    best_move_dict = move_to_dict(best_move)
    chess.delete_move(best_move)
    return best_move_dict

@app.route("/game/tune")
def tune_game():
    # TODO: Accept a large json object with tuning values
    #       and apply them to the engine
    return "NOT IMPLEMENTED", 400

def delayed_open_browser():
    time.sleep(3)
    webbrowser.open("localhost:5000")

if __name__ == "__main__":
    t = threading.Thread(target=delayed_open_browser)
    t.start()
    app.run(host="0.0.0.0", port=8080)
