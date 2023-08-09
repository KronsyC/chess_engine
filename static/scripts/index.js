
const chessboard = document.querySelector(".chessboard")
const teamdisplay = document.querySelector(".active-team-display")
const movedisplay = document.querySelector(".legal-move-display")
const halfmoves = document.querySelector(".halfmove-display")
const fullmoves = document.querySelector(".fullmove-display")
const white_score = document.querySelector(".white-score-display")
const black_score = document.querySelector(".black-score-display")
const white_advantage = document.querySelector(".white-advantage-display")
const black_advantage = document.querySelector(".black-advantage-display")
const best_move = document.querySelector(".best-move")

const BOARD_TOP = chessboard.getBoundingClientRect().top + window.scrollX;
const BOARD_BOTTOM = chessboard.getBoundingClientRect().bottom + window.scrollX;
const BOARD_LEFT = chessboard.getBoundingClientRect().left + window.scrollY;
const BOARD_RIGHT = chessboard.getBoundingClientRect().right + window.scrollY;

const board_width = BOARD_BOTTOM - BOARD_TOP;
const board_height = BOARD_RIGHT - BOARD_LEFT;

function canvas_arrow(context, fromx, fromy, tox, toy) {
    var headlen = 10; // length of head in pixels
    var dx = tox - fromx;
    var dy = toy - fromy;
    var angle = Math.atan2(dy, dx);
    context.moveTo(fromx, fromy);
    context.lineTo(tox, toy);
    context.lineTo(tox - headlen * Math.cos(angle - Math.PI / 6), toy - headlen * Math.sin(angle - Math.PI / 6));
    context.moveTo(tox, toy);
    context.lineTo(tox - headlen * Math.cos(angle + Math.PI / 6), toy - headlen * Math.sin(angle + Math.PI / 6));
}


const x_dim = board_width / 8
const y_dim = board_height / 8
const ctx = chessboard.getContext("2d")

const sprites = {
    "black": {},
    "white": {}
}

//
// Prefetch all of the sprites to
// avoid annoying flickering
//
async function populate_sprites() {
    const pieces = ["pawn", "bishop", "rook", "king", "queen", "knight"]
    let promises = []
    for (p of pieces) {
        const white = new Image
        white.src = "/sprites/white_" + p + ".png"
        const black = new Image
        black.src = "/sprites/black_" + p + ".png"

        promises.push(black.decode())
        promises.push(white.decode())

        sprites["black"][p] = black
        sprites["white"][p] = white
    }
    await Promise.all(promises)
}



const PieceKind = {
    Pawn: 0,
    Rook: 1,
    Knight: 2,
    Queen: 3,
    King: 4,
    Bishop: 5,
};
const PieceTeam = {
    Black: 0,
    White: 1,
}

class Pos {
    x;
    y;
    constructor(x, y) {
        this.x = x
        this.y = y
    }


    alphanum() {
        let ret = ""
        switch (this.x) {
            case 0: ret += 'A'; break
            case 1: ret += 'B'; break
            case 2: ret += 'C'; break
            case 3: ret += 'D'; break
            case 4: ret += 'E'; break
            case 5: ret += 'F'; break
            case 6: ret += 'G'; break
            case 7: ret += 'H'; break
        }
        ret += (7 - this.y + 1).toString()

        return ret;
    }
}

class Move {
    from
    to
    kind

    constructor(from, to, kind) {
        this.from = from
        this.to = to
        this.kind = kind
    }
}

class Piece {
    pos
    attacks;
    team;
    kind;

    constructor(pos, team, kind, attacks) {
        this.attacks = attacks
        this.pos = pos
        this.team = team
        this.kind = kind
    }
}





function get_selected_square(abs_x, abs_y) {
    if (abs_x > BOARD_RIGHT || abs_y > BOARD_BOTTOM) return;
    const x = Math.floor(abs_x - BOARD_LEFT);
    const y = Math.floor(abs_y - BOARD_TOP);


    if (x < 0 || y < 0) return;
    const SELECTED_ROW = Math.floor(y / y_dim)
    const SELECTED_COL = Math.floor(x / x_dim);

    return new Pos(SELECTED_COL, SELECTED_ROW)
}


let BLACK_PIECES = []
let WHITE_PIECES = []
let HIGHLIGHTED_PIECE = undefined;
let ACTIVE_TEAM = undefined;
let BEST_MOVE = undefined;

function ALL_PIECES() {
    return [].concat(WHITE_PIECES, BLACK_PIECES)
}

let active_team = PieceTeam.Black;
function get_piece(x, y) {
    for (let p of ALL_PIECES()) {
        if (p.pos.x == x && p.pos.y == y) {
            return p;
        }
    }
    return null;
}
async function highlight(piece) {
    if (!piece) return;
    HIGHLIGHTED_PIECE = piece;
    await render_board();

}
async function unhighlight() {
    HIGHLIGHTED_PIECE = undefined;
    await render_board();
}
async function highlight_squares(positions) {
    console.log("HIGHLIGHT", positions)
    for (let pos of positions) {
        ctx.fillStyle = "rgb(179, 66, 245)"

        if (get_piece(pos.x, pos.y)) {
            ctx.fillStyle = "rgb(255, 0, 0)"
        }
        const center_x = pos.x * x_dim + Math.floor(x_dim / 2)
        const center_y = pos.y * y_dim + Math.floor(y_dim / 2)
        ctx.beginPath()
        await ctx.ellipse(center_x, center_y, x_dim / 8, x_dim / 8, 0, 0, 2 * Math.PI)
        ctx.fill()
    }
}
function render_square(i) {
    return new Promise(async (res, rej) => {
        const row = Math.floor(i / 8);
        const col = i % 8;
        const piece = get_piece(col, row)
        if ((row + col) % 2 == 0) {
            ctx.fillStyle = "rgb(53, 103, 150)"
        }
        else {
            ctx.fillStyle = "rgb(255,255,255)"
        }

        if (HIGHLIGHTED_PIECE && HIGHLIGHTED_PIECE.pos.x == col && HIGHLIGHTED_PIECE.pos.y == row) {
            if ((col + row) % 2 == 0) {
                ctx.fillStyle = "rgb(200, 90, 256)"
            }
            else {
                ctx.fillStyle = "rgb(242, 245, 66)"
            }
        }
        const start_x = col * x_dim;
        const start_y = row * y_dim;
        ctx.fillRect(start_x, start_y, y_dim, y_dim)

        if (piece) {
            const team_name = Object.keys(PieceTeam)[Object.values(PieceTeam).indexOf(piece.team)].toLowerCase()
            const kind_name = Object.keys(PieceKind)[Object.values(PieceKind).indexOf(piece.kind)].toLocaleLowerCase()
            const URL = "/sprites/" + team_name + "_" + kind_name + ".png"
            const IMG = sprites[team_name][kind_name]
            ctx.drawImage(IMG, start_x, start_y, x_dim, y_dim);
        }
        res(true)

    })

}
async function render_board() {


    // Initial Rendering of the board
    const promises = []
    for (let i = 0; i < 64; i++) {
        promises.push(render_square(i))
    }
    await Promise.all(promises);
    if (HIGHLIGHTED_PIECE) {
        await highlight_squares(HIGHLIGHTED_PIECE.attacks.map(a => a.to))
    }
    if (BEST_MOVE) {
        const start_x = BEST_MOVE.from.x * x_dim + x_dim / 2;
        const start_y = BEST_MOVE.from.y * y_dim + y_dim / 2;
        const end_x = BEST_MOVE.to.x * x_dim + x_dim / 2;
        const end_y = BEST_MOVE.to.y * y_dim + y_dim / 2;

        console.log("DRAW ARROW FROM", start_x, ",", start_y, " TO ", end_x, ",", end_y)
        // await highlight_squares([BEST_MOVE.to, BEST_MOVE.from])
        ctx.beginPath()
        canvas_arrow(ctx, start_x, start_y, end_x, end_y)
        ctx.stroke()
    }
}



function translate_team(teamname) {
    switch (teamname) {
        case "white": return PieceTeam.White;
        case "black": return PieceTeam.Black;
    }
}
function translate_kind(kindname) {
    switch (kindname) {
        case "king": return PieceKind.King;
        case "queen": return PieceKind.Queen;
        case "rook": return PieceKind.Rook;
        case "knight": return PieceKind.Knight;
        case "bishop": return PieceKind.Bishop;
        case "pawn": return PieceKind.Pawn;
        default: throw Error("Failed to parse Piece kind: " + kindname);
    }
}
function translate_bitboard(board) {

    ATTACKS = []

    let rowno = 0;
    for (let row of board) {
        const bin = row.charCodeAt(0);

        for (let i = 0; i < 8; i++) {
            const shift = i;
            const bit = (bin >>> shift) & 0x01;
            if (bit) {
                ATTACKS.push(new Pos(i, 7 - rowno))
            }
        }

        rowno++;
    }

    return ATTACKS
}
async function move_piece(piece, move) {
    console.log(move)
    BEST_MOVE = undefined
    await render_board();
    const oldpos = (7 - piece.pos.y) * 8 + piece.pos.x
    const newpos = (7 - move.to.y) * 8 + move.to.x
    const kind = move.kind

    const payload = oldpos.toString() + "." + newpos.toString() + "." + kind.toString()


    const response = await fetch("/game/move/" + payload);
    if (response.ok) {
        piece.pos.x = newpos.x
        piece.pos.y = newpos.y
        await update_info();
    }
}

function make_pieces(pieces, team) {
    let ret = []
    for (let info of pieces) {
        const x = info.pos % 8;
        const y = Math.floor(info.pos / 8);
        if (!info) {
            continue;
        }
        const kind = translate_kind(info.kind)
        const pos = new Pos(x, 7 - y)
        const attacks = info.attacks.map(a => new Move(pos, new Pos(a.to % 8, 7 - Math.floor(a.to / 8)), a.kind))

        ret.push(new Piece(pos, team, kind, attacks))
    }
    return ret;
}

function find_total_moves() {
    let accum = 0;
    for (p of ALL_PIECES()) {
        if (p.team != ACTIVE_TEAM) continue;
        accum += p.attacks.length
    }
    return accum;
}

let controller = new AbortController();
let loading = false
async function update_info() {


    if(loading){
        console.log("ABORTING")
        loading = false
        controller.abort();
        controller = new AbortController();
    }

    console.log("FETCH")
    const res = await fetch("/game/info")
    const data = await res.json()

    console.log("FETCHED")
    // White to move, or black to move
    if (data.state == 0 || data.state == 1) {
        const whites = make_pieces(data.whites, PieceTeam.White)
        const blacks = make_pieces(data.blacks, PieceTeam.Black)
        WHITE_PIECES = whites;
        BLACK_PIECES = blacks;
        ACTIVE_TEAM = (data.state == 1) ? PieceTeam.Black : PieceTeam.White;


        loading = true
        fetch("/game/best_move", {
            signal: controller.signal,
        })
        .then(async data => {
            const obj = await data.json();
            BEST_MOVE = new Move(new Pos(obj.from%8, 7-Math.floor(obj.from/8)), new Pos(obj.to%8, 7-Math.floor(obj.to/8)), obj.kind)
            // console.log(obj)
            await render_board();
        })
        .catch(()=>{
        })
        .finally(()=>{
            loading = false;
        })

    }
    else if(data.state == 2){
        alert("CHECKMATE: White Wins")
    }
    else if(data.state == 3){
        alert("CHECKMATE: Black Wins")
    }
    else{
        alert("STALEMATE: You Drew")
    }

    const legal_move_count = find_total_moves()

    teamdisplay.innerHTML = ACTIVE_TEAM == PieceTeam.White ? "White to move" : "Black to move";
    movedisplay.innerHTML = "There are " + legal_move_count + " moves that can be made";
    halfmoves.innerHTML = "halfmoves: " + data.halfmove_count;
    fullmoves.innerHTML = "fullmoves: " + data.fullmove_count;
    white_advantage.innerHTML = "white evaluation: " + data.white_score.toFixed(2)
    black_advantage.innerHTML = "black evaluation: " + data.black_score.toFixed(2)
    console.log(best_move)
}

window.onload = async () => {
    await populate_sprites()
    await restartgame();

}


chessboard.addEventListener("click", async (event) => {

    let moved = false;
    if (HIGHLIGHTED_PIECE) {
        const pos = get_selected_square(event.clientX, event.clientY);

        // Check if this position is a current attack point
        for (let atk of HIGHLIGHTED_PIECE.attacks) {
            if (atk.to.x == pos.x && atk.to.y == pos.y) {
                await move_piece(HIGHLIGHTED_PIECE, atk);
                moved = true
                break;
            }
        }
    }
    const pos = get_selected_square(event.clientX, event.clientY);
    if (HIGHLIGHTED_PIECE && pos.x == HIGHLIGHTED_PIECE.pos.x && pos.y == HIGHLIGHTED_PIECE.pos.y) {
        await unhighlight();
        return;
    }
    await unhighlight();
    if (!moved) {
        if (!pos) return
        const select_piece = get_piece(pos.x, pos.y)

            if (!select_piece) return;
            console.log(ACTIVE_TEAM)
            if (select_piece.team != ACTIVE_TEAM) return
            await highlight(select_piece);
        

    }

})

async function restartgame() {
  ACTIVE_TEAM = undefined
  HIGHLIGHTED_PIECE = undefined
  BEST_MOVE = undefined
  HIGHLIGHT = undefined
    await fetch("/game/new")
    await update_info()
    await render_board()
}


function fetch_tuning(){
    return {
        "depth": 5,
    }
}
