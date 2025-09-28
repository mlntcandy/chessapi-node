/** Player color */
export enum PlayerColor {
  /** The player using white pieces */
  WHITE = 0,
  /** The player using black pieces */
  BLACK = 1,
}
/** Piece type */
export enum PieceType {
  /** A pawn piece */
  PAWN = 1,
  /** A bishop piece */
  BISHOP = 2,
  /** A knight piece */
  KNIGHT = 3,
  /** A rook piece */
  ROOK = 4,
  /** A queen piece */
  QUEEN = 5,
  /** A king piece */
  KING = 6,
}
/** Game play state */
export enum GameState {
  /** Indicates the game has ended in a checkmate */
  GAME_CHECKMATE = -1,
  /** Indicates the game has not ended yet */
  GAME_NORMAL = 0,
  /** Indicates the game has ended in a draw */
  GAME_STALEMATE = 1,
}
/**
 * A BitBoard is a way of representing the spaces of the chess board. Each bit corresponds to
a square on the board, and is on or off depending on what data that BitBoard represents.
For example, if BitBoard `pawns` represents white pawn positions, we can find all pawn attacks with:

* ```
bb_slide_nw(pawns) | bb_slide_ne(pawns);
```

* By convention, the bits are matched to board squares from left-to-right, bottom-to-top.
That is, the LSB corresponds to the a1 square, the 8th lowest bit corresponds to the a8 square, and the MSB corresponds to the h8 square.
Bitboards enable very efficient operations on bulk sets of pieces.
 */
export type BitBoard = bigint;

/** A Move represents a single chess move from a start location to an end location */
export type Move = {
  /** A BitBoard representing the origin of the move */
  from: BitBoard;
  /** A BitBorad representing the target of the move */
  to: BitBoard;
  /** Will be one of {@linkcode PieceType.BISHOP}, {@linkcode PieceType.KNIGHT}, etc. or `null` if not required */
  promotion: PieceType | null;
  /** `true` if this move captures a piece */
  capture: boolean;
  /** `true` if this move is castling */
  castle: boolean;
};

export interface Board {
  /**
   * @returns A clone of this board
   */
  clone(): Board;
  /**
   * @returns An array of legal moves on this board
   */
  getLegalMoves(): Move[];
  /**
   * See also: {@linkcode Board.isBlackTurn()}
   * @returns `true` if it is white to move
   */
  isWhiteTurn(): boolean;
  /**
   * See also: {@linkcode Board.isWhiteTurn()}
   * @returns `true` if it is black to move
   */
  isBlackTurn(): boolean;
  /**
   * Skips your turn on this board.
   *
   * You can't actually skip your turn, but it's useful for some search techniques.
   *
   * Can be un-done using {@linkcode Board.undoMove()} as usual.
   */
  skipTurn(): void;
  /**
   * @returns `true` if the current player is in check
   */
  inCheck(): boolean;
  /**
   * @returns `true` if the current player is in checkmate
   */
  inCheckmate(): boolean;
  /**
   * This function considers positions with no legal moves, the 50-move rule, and threefold repetition as draws.
   * @returns `true` if the current player is in a draw for any reason
   */
  inDraw(): boolean;
  /**
   * You lose kingside castling rights if you move your king or the kingside rook
   *
   * See also: {@linkcode Board.canQueensideCastle()}
   * @param color The player to consider
   * @returns `true` if the current player has kingside castling rights
   */
  canKingsideCastle(color: PlayerColor): boolean;
  /**
   * You lose queenside castling rights if you move your king or the queenside rook
   *
   * See also: {@linkcode Board.canKingsideCastle()}
   * @param color The player to consider
   * @returns `true` if the player has queenside castling rights
   */
  canQueensideCastle(color: PlayerColor): boolean;
  /**
   * Returns one of the {@link GameState} members for the given board
   *
   * The GameState members indicate whether the game is in checkmate, stalemate or neither (if the game is ongoing)
   * This is about the same cost as calls to {@linkcode inCheck()}, {@linkcode inDraw()} etc. so if you plan to check multiple you may wish to use this
   * @returns The current `GameState`
   */
  getGameState(): GameState;
  /**
   * Returns the Zobrist hash that represents the board.
   *
   * Zobrist hashes are not guaranteed to be unique for all boards. Collisions are unlikely, but if you consider enough boards you should expect a collision.
   *
   * The hashes consider en passant and castling possibilities as part of the hash, these will create different hashes for otherwise visually identical positions
   */
  zobristKey(): bigint;
  /**
   * Performs a move on the board
   *
   * See also: {@linkcode Board.undoMove()}
   * @param move The move to perform
   */
  makeMove(move: Move): void;
  /**
   * Undo the previous move on the board
   *
   * This function can be invoked multiple times to undo a sequence of moves
   *
   * It is an error to call this function on a board which has not had any moves played on it
   */
  undoMove(): void;
  /**
   * Returns the BitBoard for the given color and piece type from the board.
   * @param color The player color whose pieces to get
   * @param pieceType The type of piece to get
   * @returns A BitBoard with bits set to 1 for all squares containing the described piece
   */
  getBitboard(color: PlayerColor, pieceType: PieceType): BitBoard;
  /**
   * Returns the full move counter for the board.
   *
   * This number starts at 1, and increments each time black moves.
   */
  getFullMoves(): number;
  /**
   * Returns the half move counter for the board.
   *
   * This number starts at 0, and increments each time black or white move.
   * It resets to zero each time a pawn is moved or a capture occurs.
   * This is mainly used for tracking progress to the 50-move draw rule.
   */
  getHalfMoves(): number;
  /**
   * Returns the type of piece on the square at the given index.
   *
   * Square index travels from 0 left-to-right, bottom-to-top from white's perspective.
   * That is, index 0 is a1, index 7 is h1, index 63 is h8.
   *
   * See also: {@linkcode getPieceFromBitboard()}
   * @param index The index of the square.
   * @returns A {@link PieceType} member representing the type of piece on that square.
   */
  getPieceFromIndex(index: number): PieceType;
  /**
   * Returns the type of piece on the square set on the given bitboard.
   *
   * This function expects a bitboard with a single bit set, such as the kind you would get from a {@linkcode Move} object.
   *
   * See also: {@linkcode getPieceFromIndex()}
   * @param bitboard The bitboard of the square.
   * @returns A {@link PieceType} member representing the type of piece on that square.
   */
  getPieceFromBitboard(bitboard: BitBoard): PieceType;
  /**
   * Returns the color of piece on the square at the given index.
   *
   * Square index travels from 0 left-to-right, bottom-to-top from white's perspective.
   * That is, index 0 is a1, index 7 is h1, index 63 is h8.
   *
   * See also: {@linkcode getColorFromBitboard()}
   * @param index The index of the square.
   * @returns A {@link PlayerColor} member representing the color of piece on that square.
   */
  getColorFromIndex(index: number): PlayerColor;
  /**
   * Returns a square index equivalent to the square indicated by the given bitboard.
   *
   * This function expects a bitboard with a single bit set, such as the kind you would get from a {@linkcode Move} object.
   *
   * See also: {@linkcode getColorFromIndex()}
   * @param bitboard The bitboard of the square.
   * @returns A {@link PlayerColor} member representing the color of piece on that square.
   */
  getColorFromBitboard(bitboard: BitBoard): PlayerColor;
}

/**
 * @returns The current board being played in the chess match
 */
export function getBoard(): Board;
/**
 * Submit a new move to the chess server to play.
 *
 * You can call this more than once per turn.
 * The latest move pushed by the bot will be played by the server once {@link done()} is called.
 * @param move The move to submit
 */
export function push(move: Move): void;
/**
 * Ends the current turn.
 *
 * The latest move pushed will be played by the server.
 * This method will block until the opponent's turn has passed.
 *
 * See also: {@link push()}
 */
export function done(): void;
/**
 * Returns the remaining time this bot had at the start of its turn, in ms.
 */
export function getTimeMillis(): number;
/**
 * Returns the remaining time the opponent bot had at the start of its turn, in ms.
 */
export function getOpponentTimeMillis(): number;
/**
 * Returns how much time has elapsed this turn, in ms.
 */
export function getElapsedTimeMillis(): number;
/**
 * Returns a square index equivalent to the square indicated by the given bitboard.
 *
 * This function expects a bitboard with a single bit set, such as the kind you would get from a {@linkcode Move} object.
 * @param bitboard The bitboard to get the square index of.
 * @returns An index from 0-63 indicating the set square.
 */
export function getIndexFromBitboard(bitboard: BitBoard): number;
/**
 * Returns a bitboard equivalent to the square indicated by the given index.
 *
 * Square index travels from 0 left-to-right, bottom-to-top from white's perspective.
 * That is, index 0 is a1, index 7 is h1, index 63 is h8.
 * @param index The square index to get the bitboard of.
 * @returns A bitboard with a single bit set on the associated square.
 */
export function getBitboardFromIndex(index: number): BitBoard;
/**
 * Returns the last move made by the opponent.
 *
 * If no moves have been made yet, the returned move will have all its fields set to zero.
 * @returns The move made by the opponent on the last play.
 */
export function getOpponentMove(): Move;
