module.exports = Object.assign(require("bindings")("chessapi-node.node"), {
    PlayerColor: {
        WHITE: 0,
        BLACK: 1,
    },
    PieceType: {
        PAWN: 1,
        BISHOP: 2,
        KNIGHT: 3,
        ROOK: 4,
        QUEEN: 5,
        KING: 6,
    },
    GameState: {
        GAME_CHECKMATE: -1,
        GAME_NORMAL: 0,
        GAME_STALEMATE: 1,
    }
});