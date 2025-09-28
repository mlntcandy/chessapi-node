const chess = require("./js");

while (true) {
    const board = chess.getBoard();
    const moves = board.getLegalMoves();
    chess.push(moves[Math.floor(moves.length * Math.random())]);
    chess.done();
}