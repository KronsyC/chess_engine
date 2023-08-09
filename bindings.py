from ctypes import *
import ctypes
from enum import Enum
import pathlib

from find_bin import find_binary


OPENING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1".encode('utf-8')
BINARY_PATH = find_binary()

lib = CDLL(str(BINARY_PATH))

class Team:
    Black = 0
    White = 1
class Kind:
    Pawn = 0
    Bishop = 1
    Rook = 2
    Knight = 3
    Queen = 4
    King = 5


class MoveList(Structure):
    _fields_ = [
            ("data", ctypes.POINTER(ctypes.c_void_p)),
            ("count", c_uint64)
            ]


def kind_name(kind):
    if kind == Kind.King: return "king"
    elif kind == Kind.Queen: return "queen"
    elif kind == Kind.Rook: return "rook"
    elif kind == Kind.Bishop: return "bishop"
    elif kind == Kind.Knight: return "knight"
    elif kind == Kind.Pawn: return "pawn"



move_origin = lib.chess__move_from
move_destination = lib.chess__move_to
move_kind = lib.chess__move_kind

move_origin.argtypes = [c_void_p]
move_destination.argtypes = [c_void_p]
move_kind.argtypes = [c_void_p]

move_origin.restype = c_uint8
move_destination.restype = c_uint8
move_kind.restype = c_int

is_mated = lib.chess__game_mated
is_checked = lib.chess__game_checked
is_stalemated = lib.chess__game_stalemated

get_state = lib.chess__game_state
get_state.restype = ctypes.c_int
get_state.argtypes = [ctypes.c_void_p]



halfmoves = lib.chess__game_halfmoves
fullmoves = lib.chess__game_fullmoves

get_team_moves = lib.chess__game_moves
move       = lib.chess__piece_move
get_team   = lib.chess__game_get_team

print_board = lib.chess__print_board

find_piece = lib.chess__piece_find
piece_team = lib.chess__piece_get_team
piece_kind = lib.chess__piece_get_kind
delete_piece = lib.chess__piece_delete
get_team_pieces = lib.chess__game_pieces

get_knights  = lib.chess__game_knights
get_bishops = lib.chess__game_bishops
get_rooks = lib.chess__game_rooks
get_queens = lib.chess__game_queens
get_kings = lib.chess__game_kings
get_pawns = lib.chess__game_pawns

create_game = lib.chess__game_create
delete_game = lib.chess__game_delete
print_game = lib.chess__game_print


create_game.restype = c_void_p
create_game.argtypes = [c_void_p]

delete_game.argtypes = [c_void_p]
print_game.argtypes = [c_void_p]

get_pawns.restype = c_uint64
get_rooks.restype = c_uint64
get_bishops.restype = c_uint64
get_knights.restype = c_uint64
get_queens.restype = c_uint64
get_kings.restype = c_uint64

get_pawns.argtypes = [c_void_p]
get_rooks.argtypes = [c_void_p]
get_bishops.argtypes = [c_void_p]
get_knights.argtypes = [c_void_p]
get_queens.argtypes = [c_void_p]
get_kings.argtypes = [c_void_p]

get_team_pieces.restype = c_uint64
get_team_pieces.argtypes = [c_void_p, c_int]

find_piece.restype = c_void_p
find_piece.argtypes = [c_void_p, c_uint8, c_uint8]

delete_piece.argtypes = [c_void_p]

piece_team.restype = c_int
piece_kind.restype = c_int
piece_team.argtypes = [c_void_p]
piece_kind.argtypes = [c_void_p]


get_team.restype = c_int
get_team.argtypes = [c_void_p]

move.argtypes = [c_void_p, c_void_p]

get_team_moves.argtypes = [c_void_p, c_int]
get_team_moves.restype = MoveList


is_mated.restype = c_bool
is_checked.restype = c_bool
is_stalemated.restype = c_bool

is_mated.argtypes = [c_void_p, c_int]
is_checked.argtypes = [c_void_p, c_int]
is_stalemated.argtypes = [c_void_p, c_int]


print_board.argtypes = [c_uint64]

halfmoves.restype = c_uint32
fullmoves.restype = c_uint32

halfmoves.argtypes = [c_void_p]
fullmoves.argtypes = [c_void_p]


evaluate_game = lib.chess__game_evaluation
game_advantage = lib.chess__game_advantage

evaluate_game.argtypes = [c_void_p, c_int, c_void_p]
evaluate_game.restype = c_float

game_advantage.argtypes = [c_void_p, c_int, c_void_p]
game_advantage.restype = c_float


get_agent_move = lib.chess__game_agent_move
delete_move = lib.chess__delete_move

create_weighted_agent = lib.chess__create_weighted_agent
create_random_agent = lib.chess__create_random_agent

delete_weighted_agent = lib.chess__delete_weighted_agent
delete_random_agent = lib.chess__delete_random_agent

get_agent_move.argtypes = [c_void_p, c_void_p]
get_agent_move.restype = c_void_p

delete_move.argtypes = [c_void_p]

create_weighted_agent.restype = c_void_p
create_random_agent.restype = c_void_p

delete_random_agent.argtypes =[c_void_p]
delete_weighted_agent.argtypes = [c_void_p]

DEFAULT_WEIGHTED_AGENT = create_weighted_agent()


###### EVALUATOR WEIGHTS ########
def EW(name : str):
    _get = lib["chess__weight_get_"+name]
    _set = lib["chess__weight_set_"+name]

    _get.argtypes = [c_void_p]
    _set.argtypes = [c_void_p, c_float]

    _get.restype = c_float

    class WeightSet:
        def get(game):
            return _get(game)
        def set(game, val):
            _set(game, val)

    return WeightSet

def EWA(name : str):
    return (
        EW("op_"+name),
        EW("mg_"+name),
        EW("eg_"+name)
    )

# wkingpos = EW("king_position")
# wqueenpos = EW("queen_position")
# wrookpos = EW("rook_position")
# wknightpos = EW("knight_position")
# wbishoppos = EW("bishop_position")
# wpawnpos = EW("pawn_position")

# wkingval = EW("king_value")
# wqueenval = EW("queen_value")
# wrookval = EW("rook_value")
# wknightval = EW("knight_value")
# wbishopval = EW("bishop_value")
# wpawnval = EW("pawn_value")

# (
#     wcheck_op,
#     wcheck_mg,
#     wcheck_eg
# ) = EWA("check")

# (
#     wmobility_op,
#     wmobility_mg,
#     wmobility_eg
# ) = EWA("mobility")

# (
#     wvulnerability_op,
#     wvulnerability_mg,
#     wvulnerability_eg,
# ) = EWA("vulnerability")

# (
#     wpawndevel_op,
#     wpawndevel_mg,
#     wpawndevel_eg
# ) = EWA("pawn_devel")

# (
#     wpositioning_op,
#     wpositioning_mg,
#     wpositioning_eg,
# ) = EWA("positioning")

# (
#     wmaterial_op,
#     wmaterial_mg,
#     wmaterial_eg
# ) = EWA("material")
