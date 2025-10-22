## bookgen.wasm

[![npm version](https://badge.fury.io/js/fairy-stockfish-nnue.wasm.svg)](https://badge.fury.io/js/fairy-stockfish-nnue.wasm)
[![CI](https://github.com/ianfab/fairy-stockfish.wasm/actions/workflows/ci.yml/badge.svg)](https://github.com/ianfab/fairy-stockfish.wasm/actions/workflows/ci.yml)

WebAssembly port of [bookgen](https://github.com/ianfab/bookgen) with NNUE support.

See [bookgen-wasm](https://github.com/ianfab/bookgen-wasm) for a demo.

For development, see [`src/emscripten/README.md`](src/emscripten/README.md).

Current default branch is `bookgen`.

To release a new version:
* Make sure CI passes (update reference bench if required)
* Bump version number in `src/emscripten/public/package.json`
* Create and push a tag with the name corresponding to the version number
