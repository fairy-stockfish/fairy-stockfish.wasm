## fairy-stockfish-nnue.wasm

[![npm version](https://badge.fury.io/js/fairy-stockfish-nnue.wasm.svg)](https://badge.fury.io/js/fairy-stockfish-nnue.wasm)
[![CI](https://github.com/ianfab/fairy-stockfish.wasm/actions/workflows/ci.yml/badge.svg)](https://github.com/ianfab/fairy-stockfish.wasm/actions/workflows/ci.yml)

WebAssembly port of [bookgen](https://github.com/ianfab/bookgen) with NNUE support.

See [bookgen-wasm](https://github.com/ianfab/bookgen-wasm) for a demo.

For development, see [`src/emscripten/README.md`](src/emscripten/README.md).

The original `README.md` follows below.

# bookgen

Bookgen extends Fairy-Stockfish by the possibility of automatically generating opening books for all supported chess variants, including Chess, Xiangqi, Shogi, Crazyhouse, Antichess, Atomic, etc.

Several methods for generation and filtering of positions in FEN format are supported to adapt the book to specific requirements. The generation can be based on perft and multipv searches. The filtering supports a variety of options to define the criteria for evaluation of positions to be included. The output format is EPD. See the [Wiki](https://github.com/fairy-stockfish/bookgen/wiki) for documentation and usage examples.

Built binaries for Windows and Linux can be downloaded from the [releases](https://github.com/fairy-stockfish/bookgen/releases). Development versions can be downloaded from the [release workflow](https://github.com/fairy-stockfish/bookgen/actions/workflows/release.yml) (requires logging in to github).

Instead of downloading it, you can also use the [online book generator](https://bookgen-wasm.vercel.app) ([code](https://github.com/ianfab/bookgen-wasm)), which uses the [WebAssembly port](https://github.com/fairy-stockfish/fairy-stockfish.wasm/tree/bookgen).

There are separate (older) versions based on specialized variant forks, such as [official Stockfish](https://github.com/fairy-stockfish/bookgen/tree/official-stockfish), [multi-variant Stockfish](https://github.com/fairy-stockfish/bookgen/tree/multivariant), [Seirawan-Stockfish](https://github.com/fairy-stockfish/bookgen/tree/seirawan), [Shatranj-Stockfish](https://github.com/fairy-stockfish/bookgen/tree/shatranj), and [Makruk-Stockfish](https://github.com/fairy-stockfish/bookgen/tree/makruk) that can be found on the respective branches, but the Fairy-Stockfish based version supports all of these variants as well.

Also see the [Fairy-Stockfish](https://github.com/fairy-stockfish/Fairy-Stockfish) repository for more info.
