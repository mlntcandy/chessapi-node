# chessapi-node

js bindings for [shiro-nya/2025-chess-bot-tournament](https://github.com/shiro-nya/2025-chess-bot-tournament)

- typescript types & jsdoc docs included
- node 14 and up (node-api 6)
- should seamlessly work on Bun too

## installation

1. make sure you have **cmake** and your c toolchain set up
   - on **windows** get Visual Studio Build Tools with C++ Classic App Development - make sure MSVC, CMake and the appropriate SDK (the win10 one may not have `threads.h`? unconfirmed) are selected
   - if you're on **linux** you have probably already figured this out. if not google "how to install cmake and gcc on yourdistro"
   - on **macos** there's no `threads.h` so it won't compile. don't try yet
2. `$ npm i https://github.com/mlntcandy/chessapi-node.git`
   - the post-install script should automatically download node headers & lib and compile the bindings
   - compile errors will occur on this step

## usage

```js
const chess = require("chessapi-node");
// or import ESM style if you have that set up ("type": "module" in package.json)
import chess from "chessapi-node";

// this will spam random valid moves indefinitely
while (true) {
  const board = chess.getBoard();
  const moves = board.getLegalMoves();
  chess.push(moves[Math.floor(moves.length * Math.random())]);
  chess.done();
}
```

### running the bot

add engine to cutechess, set command to `<node-executable-path> <script-path>`. wrap both in quotes on windows

your script will just be a uci engine - you can also run that however else you like
