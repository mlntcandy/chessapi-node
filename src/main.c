#define NAPI_VERSION 6
#include <node_api.h>

#include "chessapi/chessapi.h"

// deescalate back to js code asap, an exception is pending already
#define assert_or_null(ex) \
    if (!(ex))             \
        return NULL;
#define assert_or_false(ex) \
    if (!(ex))              \
        return false;

#define DECLARE_NAPI_METHOD(name, func) \
    {name, 0, func, 0, 0, 0, napi_default, 0}

#define DECLARE_NAPI_PROPERTY(name, val) \
    {name, 0, 0, 0, 0, val, napi_default, 0}

// Type conversion helpers

// BitBoard
napi_value wrapBitBoard(napi_env env, BitBoard bb)
{
    napi_status status;

    napi_value val;
    status = napi_create_bigint_uint64(env, bb, &val);
    assert_or_null(status == napi_ok);

    return val;
}
BitBoard unwrapBitBoard(napi_env env, napi_value val)
{
    napi_status status;
    uint64_t bb;
    bool lossless;
    status = napi_get_value_bigint_uint64(env, val, &bb, &lossless);
    if (status != napi_ok)
    {
        return UINT64_MAX;
    }
    if (!lossless)
    {
        napi_throw_error(env, 0, "Failed to convert bigint to BitBoard, value out of bounds");
        return UINT64_MAX;
    }
    return bb;
}
// Move
napi_value wrapMove(napi_env env, Move move)
{
    napi_status status;

    napi_value from = wrapBitBoard(env, move.from);
    assert_or_null(from != NULL);

    napi_value to = wrapBitBoard(env, move.to);
    assert_or_null(to != NULL);

    napi_value promotion;
    if (move.promotion == 0)
        status = napi_get_null(env, &promotion);
    else
        status = napi_create_uint32(env, (uint32_t)move.promotion, &promotion);
    assert_or_null(status == napi_ok);

    napi_value capture;
    status = napi_get_boolean(env, move.capture, &capture);
    assert_or_null(status == napi_ok);

    napi_value castle;
    status = napi_get_boolean(env, move.castle, &castle);
    assert_or_null(status == napi_ok);

    // object
    napi_value obj;
    status = napi_create_object(env, &obj);
    assert_or_null(status == napi_ok);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("from", from),
        DECLARE_NAPI_PROPERTY("to", to),
        DECLARE_NAPI_PROPERTY("promotion", promotion),
        DECLARE_NAPI_PROPERTY("capture", capture),
        DECLARE_NAPI_PROPERTY("castle", castle),
    };
    status = napi_define_properties(env, obj, sizeof(properties) / sizeof(properties[0]), properties);
    assert_or_null(status == napi_ok);

    return obj;
}
bool unwrapMove(napi_env env, napi_value val, Move *move)
{
    napi_status status;

    napi_value from_js;
    status = napi_get_named_property(env, val, "from", &from_js);
    assert_or_false(status == napi_ok);
    BitBoard from = unwrapBitBoard(env, from_js);
    assert_or_false(from != UINT64_MAX);

    napi_value to_js;
    status = napi_get_named_property(env, val, "to", &to_js);
    assert_or_false(status == napi_ok);
    BitBoard to = unwrapBitBoard(env, to_js);
    assert_or_false(to != UINT64_MAX);

    napi_value promotion_js;
    status = napi_get_named_property(env, val, "promotion", &promotion_js);
    assert_or_false(status == napi_ok);
    napi_valuetype promotion_js_type;
    status = napi_typeof(env, promotion_js, &promotion_js_type);
    assert_or_false(status == napi_ok);
    uint32_t promotion_u32;
    if (promotion_js_type == napi_null)
    {
        promotion_u32 = 0;
    }
    else
    {
        status = napi_get_value_uint32(env, promotion_js, &promotion_u32);
        assert_or_false(status == napi_ok);
    }
    uint8_t promotion = (uint8_t)promotion_u32;

    napi_value capture_js;
    status = napi_get_named_property(env, val, "capture", &capture_js);
    assert_or_false(status == napi_ok);
    bool capture;
    status = napi_get_value_bool(env, capture_js, &capture);
    assert_or_false(status == napi_ok);

    napi_value castle_js;
    status = napi_get_named_property(env, val, "castle", &castle_js);
    assert_or_false(status == napi_ok);
    bool castle;
    status = napi_get_value_bool(env, castle_js, &castle);
    assert_or_false(status == napi_ok);

    move->from = from;
    move->to = to;
    move->promotion = promotion;
    move->capture = capture;
    move->castle = castle;

    return true;
}
// Board methods

// i am not writing a proper header file for this shit. this will have to do

Board *unwrapBoard(napi_env env, napi_value this);
napi_value wrapBoard(napi_env env, Board *board);

napi_value BoardClone(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    return wrapBoard(env, chess_clone_board(board));
}
napi_value BoardGetLegalMoves(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    int len;
    Move *moves = chess_get_legal_moves(board, &len);

    napi_value arr;
    status = napi_create_array_with_length(env, len, &arr);
    assert_or_null(status == napi_ok);

    for (int i = 0; i < len; i++)
    {
        napi_value move = wrapMove(env, moves[i]);
        assert_or_null(move != NULL);
        status = napi_set_element(env, arr, i, move);
        assert_or_null(status == napi_ok);
    }

    chess_free_moves_array(moves);

    return arr;
}
napi_value BoardIsWhiteTurn(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_get_boolean(env, chess_is_white_turn(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardIsBlackTurn(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_get_boolean(env, chess_is_black_turn(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardSkipTurn(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    chess_skip_turn(board);

    return NULL;
}
napi_value BoardInCheck(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_get_boolean(env, chess_in_check(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardInCheckmate(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_get_boolean(env, chess_in_checkmate(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardInDraw(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_get_boolean(env, chess_in_draw(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardCanKingsideCastle(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    int color;
    status = napi_get_value_int32(env, argv[0], &color);
    assert_or_null(status == napi_ok);

    napi_value res;
    status = napi_get_boolean(env, chess_can_kingside_castle(board, (PlayerColor)color), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardCanQueensideCastle(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    int color;
    status = napi_get_value_int32(env, argv[0], &color);
    assert_or_null(status == napi_ok);

    napi_value res;
    status = napi_get_boolean(env, chess_can_queenside_castle(board, (PlayerColor)color), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetGameState(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_create_int32(env, chess_get_game_state(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardZobristKey(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_create_bigint_uint64(env, chess_zobrist_key(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardMakeMove(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    Move move;
    assert_or_null(unwrapMove(env, argv[0], &move));

    chess_make_move(board, move);

    return NULL;
}
napi_value BoardUndoMove(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    chess_undo_move(board);

    return NULL;
}
napi_value BoardGetBitboard(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 2;
    napi_value argv[2];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 2)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 2 args");
        return NULL;
    }

    int color;
    status = napi_get_value_int32(env, argv[0], &color);
    assert_or_null(status == napi_ok);
    int piece_type;
    status = napi_get_value_int32(env, argv[1], &piece_type);
    assert_or_null(status == napi_ok);

    napi_value res = wrapBitBoard(env, chess_get_bitboard(board, (PlayerColor)color, (PieceType)piece_type));
    assert_or_null(res != NULL);

    return res;
}
napi_value BoardGetFullMoves(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_create_int32(env, chess_get_full_moves(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetHalfMoves(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    status = napi_get_cb_info(env, info, NULL, NULL, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    napi_value res;
    status = napi_create_int32(env, chess_get_half_moves(board), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetPieceFromIndex(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    int index;
    status = napi_get_value_int32(env, argv[0], &index);
    assert_or_null(status == napi_ok);

    napi_value res;
    status = napi_create_int32(env, chess_get_piece_from_index(board, index), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetPieceFromBitboard(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    BitBoard bb = unwrapBitBoard(env, argv[0]);
    assert_or_null(bb != UINT64_MAX);

    napi_value res;
    status = napi_create_int32(env, chess_get_piece_from_bitboard(board, bb), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetColorFromIndex(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    int index;
    status = napi_get_value_int32(env, argv[0], &index);
    assert_or_null(status == napi_ok);

    napi_value res;
    status = napi_create_int32(env, chess_get_color_from_index(board, index), &res);
    assert_or_null(status == napi_ok);

    return res;
}
napi_value BoardGetColorFromBitboard(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value this_arg;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &this_arg, NULL);
    assert_or_null(status == napi_ok);
    Board *board = unwrapBoard(env, this_arg);
    assert_or_null(board != NULL);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    BitBoard bb = unwrapBitBoard(env, argv[0]);
    assert_or_null(bb != UINT64_MAX);

    napi_value res;
    status = napi_create_int32(env, chess_get_color_from_bitboard(board, bb), &res);
    assert_or_null(status == napi_ok);

    return res;
}

// board helpers
void finalize_board(napi_env env, void *finalize_data, void *finalize_hint)
{
    Board *board = (Board *)finalize_data;
    chess_free_board(board);
}
Board *unwrapBoard(napi_env env, napi_value this)
{
    Board *board;
    napi_status status = napi_unwrap(env, this, (void **)&board);
    if (status != napi_ok)
        return NULL;
    if (board == NULL)
    {
        napi_throw_error(env, "BADCHESS", "Got null pointer when unwrapping Board");
        return NULL;
    }
    return board;
}
napi_value wrapBoard(napi_env env, Board *board)
{
    napi_status status;

    napi_value obj;
    status = napi_create_object(env, &obj);
    assert_or_null(status == napi_ok);

    napi_property_descriptor methods[] = {
        DECLARE_NAPI_METHOD("clone", BoardClone),
        DECLARE_NAPI_METHOD("getLegalMoves", BoardGetLegalMoves),
        DECLARE_NAPI_METHOD("isWhiteTurn", BoardIsWhiteTurn),
        DECLARE_NAPI_METHOD("isBlackTurn", BoardIsBlackTurn),
        DECLARE_NAPI_METHOD("skipTurn", BoardSkipTurn),
        DECLARE_NAPI_METHOD("inCheck", BoardInCheck),
        DECLARE_NAPI_METHOD("inCheckmate", BoardInCheckmate),
        DECLARE_NAPI_METHOD("inDraw", BoardInDraw),
        DECLARE_NAPI_METHOD("canKingsideCastle", BoardCanKingsideCastle),
        DECLARE_NAPI_METHOD("canQueensideCastle", BoardCanQueensideCastle),
        DECLARE_NAPI_METHOD("getGameState", BoardGetGameState),
        DECLARE_NAPI_METHOD("zobristKey", BoardZobristKey),
        DECLARE_NAPI_METHOD("makeMove", BoardMakeMove),
        DECLARE_NAPI_METHOD("undoMove", BoardUndoMove),
        DECLARE_NAPI_METHOD("getBitboard", BoardGetBitboard),
        DECLARE_NAPI_METHOD("getFullMoves", BoardGetFullMoves),
        DECLARE_NAPI_METHOD("getHalfMoves", BoardGetHalfMoves),
        DECLARE_NAPI_METHOD("getPieceFromIndex", BoardGetPieceFromIndex),
        DECLARE_NAPI_METHOD("getPieceFromBitboard", BoardGetPieceFromBitboard),
        DECLARE_NAPI_METHOD("getColorFromIndex", BoardGetColorFromIndex),
        DECLARE_NAPI_METHOD("getColorFromBitboard", BoardGetColorFromBitboard),
    };

    status = napi_define_properties(env, obj, sizeof(methods) / sizeof(methods[0]), methods);
    assert_or_null(status == napi_ok);

    status = napi_wrap(env, obj, board, finalize_board, NULL, NULL);
    assert_or_null(status == napi_ok);

    return obj;
}

// api functions
napi_value GetBoard(napi_env env, napi_callback_info info)
{
    return wrapBoard(env, chess_get_board());
}
napi_value Push(napi_env env, napi_callback_info info)
{
    napi_status status;

    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    assert_or_null(status == napi_ok);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }
    Move move;
    assert_or_null(unwrapMove(env, argv[0], &move));

    chess_push(move);

    return NULL;
}
napi_value Done(napi_env env, napi_callback_info info)
{
    chess_done();
    return NULL;
}
napi_value GetTimeMillis(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value ms;
    status = napi_create_uint32(env, (uint32_t)chess_get_time_millis(), &ms);
    assert_or_null(status == napi_ok);

    return ms;
}
napi_value GetOpponentTimeMillis(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value ms;
    status = napi_create_uint32(env, (uint32_t)chess_get_opponent_time_millis(), &ms);
    assert_or_null(status == napi_ok);

    return ms;
}
napi_value GetElapsedTimeMillis(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value ms;
    status = napi_create_uint32(env, (uint32_t)chess_get_elapsed_time_millis(), &ms);
    assert_or_null(status == napi_ok);

    return ms;
}
napi_value GetIndexFromBitboard(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value idx;

    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    assert_or_null(status == napi_ok);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }

    BitBoard bb = unwrapBitBoard(env, argv[0]);
    assert_or_null(bb != UINT64_MAX);

    status = napi_create_uint32(env, chess_get_index_from_bitboard(bb), &idx);
    assert_or_null(status == napi_ok);

    return idx;
}
napi_value GetBitboardFromIndex(napi_env env, napi_callback_info info)
{
    napi_status status;

    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    assert_or_null(status == napi_ok);

    if (argc < 1)
    {
        napi_throw_type_error(env, "BADCHESS", "Expected at least 1 arg");
        return NULL;
    }
    int idx;
    status = napi_get_value_int32(env, argv[0], &idx);

    return wrapBitBoard(env, chess_get_bitboard_from_index(idx));
}
napi_value GetOpponentMove(napi_env env, napi_callback_info info)
{
    return wrapMove(env, chess_get_opponent_move());
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("getBoard", GetBoard),
        DECLARE_NAPI_METHOD("push", Push),
        DECLARE_NAPI_METHOD("done", Done),
        DECLARE_NAPI_METHOD("getTimeMillis", GetTimeMillis),
        DECLARE_NAPI_METHOD("getOpponentTimeMillis", GetOpponentTimeMillis),
        DECLARE_NAPI_METHOD("getElapsedTimeMillis", GetElapsedTimeMillis),
        DECLARE_NAPI_METHOD("getIndexFromBitboard", GetIndexFromBitboard),
        DECLARE_NAPI_METHOD("getBitboardFromIndex", GetBitboardFromIndex),
        DECLARE_NAPI_METHOD("getOpponentMove", GetOpponentMove),
    };
    status = napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    assert_or_null(status == napi_ok);
    return exports;
}

NAPI_MODULE(chessapi, Init)