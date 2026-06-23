/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2022 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

/// When compiling with provided Makefile (e.g. for Linux and OSX), configuration
/// is done automatically. To get started type 'make help'.
///
/// When Makefile is not used (e.g. with Microsoft Visual Studio) some switches
/// need to be set manually:
///
/// -DNDEBUG      | Disable debugging mode. Always use this for release.
///
/// -DNO_PREFETCH | Disable use of prefetch asm-instruction. You may need this to
///               | run on some very old machines.
///
/// -DUSE_POPCNT  | Add runtime support for use of popcnt asm-instruction. Works
///               | only in 64-bit mode and requires hardware with popcnt support.
///
/// -DUSE_PEXT    | Add runtime support for use of pext asm-instruction. Works
///               | only in 64-bit mode and requires hardware with pext support.

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <algorithm>

#if defined(_MSC_VER)
// Disable some silly and noisy warning from MSVC compiler
#pragma warning(disable: 4127) // Conditional expression is constant
#pragma warning(disable: 4146) // Unary minus operator applied to unsigned type
#pragma warning(disable: 4800) // Forcing value to bool 'true' or 'false'
#pragma comment(linker, "/STACK:8000000") // Use 8 MB stack size for MSVC
#pragma comment(lib, "advapi32.lib") // Fix linker error
#endif

/// Predefined macros hell:
///
/// __GNUC__           Compiler is gcc, Clang or Intel on Linux
/// __INTEL_COMPILER   Compiler is Intel
/// _MSC_VER           Compiler is MSVC or Intel on Windows
/// _WIN32             Building on Windows (any)
/// _WIN64             Building on Windows 64 bit

#if defined(__GNUC__ ) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ <= 2)) && defined(_WIN32) && !defined(__clang__)
#define ALIGNAS_ON_STACK_VARIABLES_BROKEN
#endif

#define ASSERT_ALIGNED(ptr, alignment) assert(reinterpret_cast<uintptr_t>(ptr) % alignment == 0)

#if defined(_WIN64) && defined(_MSC_VER) // No Makefile used
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#  define IS_64BIT
#endif

#if defined(USE_POPCNT) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <nmmintrin.h> // Intel and Microsoft header for _mm_popcnt_u64()
#endif

#if !defined(NO_PREFETCH) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <xmmintrin.h> // Intel and Microsoft header for _mm_prefetch()
#endif

#if defined(USE_PEXT)
#  include <immintrin.h> // Header for _pext_u64() intrinsic
#  ifdef LARGEBOARDS
// Classical ray-walking used for LARGEBOARDS; pext not used.
#    define pext(b, m) 0
#  else
#    define pext(b, m) _pext_u64(b, m)
#  endif
#else
#  define pext(b, m) 0
#endif

namespace Stockfish {

#ifdef USE_POPCNT
constexpr bool HasPopCnt = true;
#else
constexpr bool HasPopCnt = false;
#endif

#ifdef USE_PEXT
constexpr bool HasPext = true;
#else
constexpr bool HasPext = false;
#endif

#ifdef IS_64BIT
constexpr bool Is64Bit = true;
#else
constexpr bool Is64Bit = false;
#endif

typedef uint64_t Key;
#ifdef LARGEBOARDS
// 5-word (320-bit) bitboard supporting up to 17x17=289 squares for Dai Dai Shogi.
// w[0]=bits 0-63 (squares 0-63), ..., w[4]=bits 256-319 (squares 256-288).
struct Bitboard {
    uint64_t w[5];

    constexpr Bitboard() : w{0, 0, 0, 0, 0} {}
    constexpr Bitboard(uint64_t v) : w{v, 0, 0, 0, 0} {}
    constexpr Bitboard(uint64_t w0, uint64_t w1, uint64_t w2, uint64_t w3, uint64_t w4)
        : w{w0, w1, w2, w3, w4} {};

    constexpr operator bool() const {
        return w[0] || w[1] || w[2] || w[3] || w[4];
    }
    constexpr operator unsigned() const { return (unsigned)w[0]; }
    constexpr operator long long unsigned() const { return w[0]; }

    constexpr Bitboard operator~() const {
        return {~w[0], ~w[1], ~w[2], ~w[3], ~w[4]};
    }
    constexpr Bitboard operator-() const {
        Bitboard r = ~(*this);
        for (int i = 0; i < 5; i++) { r.w[i]++; if (r.w[i]) break; }
        return r;
    }
    constexpr Bitboard operator|(Bitboard x) const {
        return {w[0]|x.w[0], w[1]|x.w[1], w[2]|x.w[2], w[3]|x.w[3], w[4]|x.w[4]};
    }
    constexpr Bitboard operator&(Bitboard x) const {
        return {w[0]&x.w[0], w[1]&x.w[1], w[2]&x.w[2], w[3]&x.w[3], w[4]&x.w[4]};
    }
    constexpr Bitboard operator^(Bitboard x) const {
        return {w[0]^x.w[0], w[1]^x.w[1], w[2]^x.w[2], w[3]^x.w[3], w[4]^x.w[4]};
    }
    constexpr Bitboard operator-(Bitboard x) const {
        uint64_t r[5] = {}, borrow = 0;
        for (int i = 0; i < 5; i++) {
            r[i] = w[i] - x.w[i] - borrow;
            borrow = (w[i] < x.w[i]) || (borrow && w[i] == x.w[i]) ? 1 : 0;
        }
        return {r[0], r[1], r[2], r[3], r[4]};
    }
    constexpr Bitboard operator-(int x) const { return *this - Bitboard((uint64_t)(unsigned)x); }
    inline Bitboard operator*(Bitboard x) const {
        return Bitboard(w[0] * x.w[0]); // low-word product for magic hashing
    }
    constexpr Bitboard operator<<(unsigned n) const {
        if (n >= 320) return Bitboard(0);
        int words = (int)(n / 64), bits = (int)(n % 64);
        uint64_t r[5] = {};
        if (bits == 0) {
            for (int i = words; i < 5; i++) r[i] = w[i - words];
        } else {
            for (int i = words; i < 5; i++) {
                r[i] = w[i - words] << bits;
                if (i > words) r[i] |= w[i - words - 1] >> (64 - bits);
            }
        }
        return {r[0], r[1], r[2], r[3], r[4]};
    }
    constexpr Bitboard operator>>(unsigned n) const {
        if (n >= 320) return Bitboard(0);
        int words = (int)(n / 64), bits = (int)(n % 64);
        uint64_t r[5] = {};
        if (bits == 0) {
            for (int i = 0; i < 5 - words; i++) r[i] = w[i + words];
        } else {
            for (int i = 0; i < 5 - words; i++) {
                r[i] = w[i + words] >> bits;
                if (i + words + 1 < 5) r[i] |= w[i + words + 1] << (64 - bits);
            }
        }
        return {r[0], r[1], r[2], r[3], r[4]};
    }
    constexpr Bitboard operator<<(int n) const { return *this << (unsigned)n; }
    constexpr Bitboard operator>>(int n) const { return *this >> (unsigned)n; }
    inline Bitboard& operator|=(Bitboard x) { for(int i=0;i<5;i++) w[i]|=x.w[i]; return *this; }
    inline Bitboard& operator&=(Bitboard x) { for(int i=0;i<5;i++) w[i]&=x.w[i]; return *this; }
    inline Bitboard& operator^=(Bitboard x) { for(int i=0;i<5;i++) w[i]^=x.w[i]; return *this; }
    constexpr bool operator==(Bitboard x) const {
        return w[0]==x.w[0] && w[1]==x.w[1] && w[2]==x.w[2] && w[3]==x.w[3] && w[4]==x.w[4];
    }
    constexpr bool operator!=(Bitboard x) const { return !(*this == x); }
};
constexpr int SQUARE_BITS = 9;
#else
typedef uint64_t Bitboard;
constexpr int SQUARE_BITS = 6;
#endif

//When defined, move list will be stored in heap. Delete this if you want to use stack to store move list. Using stack can cause overflow (Segmentation Fault) when the search is too deep.
#define USE_HEAP_INSTEAD_OF_STACK_FOR_MOVE_LIST

#ifdef ALLVARS
constexpr int MAX_MOVES = 8192;
#ifdef USE_HEAP_INSTEAD_OF_STACK_FOR_MOVE_LIST
constexpr int MAX_PLY = 246;
#else
constexpr int MAX_PLY = 60;
#endif
/// endif USE_HEAP_INSTEAD_OF_STACK_FOR_MOVE_LIST
#else
constexpr int MAX_MOVES = 1024;
constexpr int MAX_PLY = 246;
#endif
/// endif ALLVARS

/// A move needs 16 bits to be stored
///
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
/// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
/// NOTE: en passant bit is set only when a pawn can be captured
///
/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
/// any normal move destination square is always different from origin square
/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.

#ifdef LARGEBOARDS
enum Move : long long {
#else
enum Move : int {
#endif
  MOVE_NONE,
  MOVE_NULL = 1 + (1 << SQUARE_BITS)
};

enum MoveType : int {
  NORMAL,
  EN_PASSANT          = 1 << (2 * SQUARE_BITS),
  CASTLING           = 2 << (2 * SQUARE_BITS),
  PROMOTION          = 3 << (2 * SQUARE_BITS),
  DROP               = 4 << (2 * SQUARE_BITS),
  PIECE_PROMOTION    = 5 << (2 * SQUARE_BITS),
  PIECE_DEMOTION     = 6 << (2 * SQUARE_BITS),
  SPECIAL            = 7 << (2 * SQUARE_BITS),
  LION_MOVE          = 8 << (2 * SQUARE_BITS),
  LION_DOG_MOVE      = 9 << (2 * SQUARE_BITS),  // triple-move: from→mid1(opt cap)→mid2(opt cap)→to
};

constexpr int MOVE_TYPE_BITS = 4;

enum Color {
  WHITE, BLACK, COLOR_NB = 2
};

enum CastlingRights {
  NO_CASTLING,
  WHITE_OO,
  WHITE_OOO = WHITE_OO << 1,
  BLACK_OO  = WHITE_OO << 2,
  BLACK_OOO = WHITE_OO << 3,

  KING_SIDE      = WHITE_OO  | BLACK_OO,
  QUEEN_SIDE     = WHITE_OOO | BLACK_OOO,
  WHITE_CASTLING = WHITE_OO  | WHITE_OOO,
  BLACK_CASTLING = BLACK_OO  | BLACK_OOO,
  ANY_CASTLING   = WHITE_CASTLING | BLACK_CASTLING,

  CASTLING_RIGHT_NB = 16
};

enum CheckCount : int {
  CHECKS_0 = 0, CHECKS_NB = 11
};

enum MaterialCounting {
  NO_MATERIAL_COUNTING, JANGGI_MATERIAL, UNWEIGHTED_MATERIAL, WHITE_DRAW_ODDS, BLACK_DRAW_ODDS
};

enum CountingRule {
  NO_COUNTING, MAKRUK_COUNTING, CAMBODIAN_COUNTING, ASEAN_COUNTING
};

enum ChasingRule {
  NO_CHASING, AXF_CHASING
};

enum EnclosingRule {
  NO_ENCLOSING, REVERSI, ATAXX, QUADWRANGLE, SNORT, ANYSIDE, TOP
};

enum WallingRule {
  NO_WALLING, ARROW, DUCK, EDGE, PAST, STATIC
};

enum EndgameEval {
  NO_EG_EVAL, EG_EVAL_CHESS, EG_EVAL_ANTI, EG_EVAL_ATOMIC, EG_EVAL_DUCK, EG_EVAL_MISERE, EG_EVAL_RK, EG_EVAL_NB
};

enum OptBool {
  NO_VALUE, VALUE_FALSE, VALUE_TRUE
};

enum Phase {
  PHASE_ENDGAME,
  PHASE_MIDGAME = 128,
  MG = 0, EG = 1, PHASE_NB = 2
};

enum ScaleFactor {
  SCALE_FACTOR_DRAW    = 0,
  SCALE_FACTOR_NORMAL  = 64,
  SCALE_FACTOR_MAX     = 128,
  SCALE_FACTOR_NONE    = 255
};

enum Bound {
  BOUND_NONE,
  BOUND_UPPER,
  BOUND_LOWER,
  BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

enum Value : int {
  VALUE_ZERO      = 0,
  VALUE_DRAW      = 0,
  VALUE_KNOWN_WIN = 10000,
  VALUE_MATE      = 32000,
  XBOARD_VALUE_MATE = 200000,
  VALUE_VIRTUAL_MATE = 3000,
  VALUE_VIRTUAL_MATE_IN_MAX_PLY = VALUE_VIRTUAL_MATE - MAX_PLY,
  VALUE_INFINITE  = 32001,
  VALUE_NONE      = 32002,

  VALUE_TB_WIN_IN_MAX_PLY  =  VALUE_MATE - 2 * MAX_PLY,
  VALUE_TB_LOSS_IN_MAX_PLY = -VALUE_TB_WIN_IN_MAX_PLY,
  VALUE_MATE_IN_MAX_PLY  =  VALUE_MATE - MAX_PLY,
  VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY,

  PawnValueMg   = 126,   PawnValueEg   = 208,
  KnightValueMg = 781,   KnightValueEg = 854,
  BishopValueMg = 825,   BishopValueEg = 915,
  RookValueMg   = 1276,  RookValueEg   = 1380,
  QueenValueMg  = 2538,  QueenValueEg  = 2682,
  FersValueMg              = 420,   FersValueEg              = 450,
  AlfilValueMg             = 350,   AlfilValueEg             = 330,
  FersAlfilValueMg         = 700,   FersAlfilValueEg         = 650,
  SilverValueMg            = 660,   SilverValueEg            = 640,
  AiwokValueMg             = 2300,  AiwokValueEg             = 2700,
  BersValueMg              = 1800,  BersValueEg              = 1900,
  ArchbishopValueMg        = 2200,  ArchbishopValueEg        = 2200,
  ChancellorValueMg        = 2300,  ChancellorValueEg        = 2600,
  AmazonValueMg            = 2700,  AmazonValueEg            = 2850,
  KnibisValueMg            = 1100,  KnibisValueEg            = 1200,
  BiskniValueMg            = 750,   BiskniValueEg            = 700,
  KnirooValueMg            = 1050,  KnirooValueEg            = 1250,
  RookniValueMg            = 800,   RookniValueEg            = 950,
  ShogiPawnValueMg         =  90,   ShogiPawnValueEg         = 100,
  LanceValueMg             = 400,   LanceValueEg             = 240,
  ShogiKnightValueMg       = 420,   ShogiKnightValueEg       = 290,
  GoldValueMg              = 720,   GoldValueEg              = 700,
  DragonHorseValueMg       = 1550,  DragonHorseValueEg       = 1550,
  ClobberPieceValueMg      = 300,   ClobberPieceValueEg      = 300,
  BreakthroughPieceValueMg = 300,   BreakthroughPieceValueEg = 300,
  ImmobilePieceValueMg     = 50,    ImmobilePieceValueEg     = 50,
  CannonPieceValueMg       = 800,   CannonPieceValueEg       = 700,
  JanggiCannonPieceValueMg = 800,   JanggiCannonPieceValueEg = 600,
  SoldierValueMg           = 200,   SoldierValueEg           = 270,
  HorseValueMg             = 520,   HorseValueEg             = 800,
  ElephantValueMg          = 300,   ElephantValueEg          = 300,
  JanggiElephantValueMg    = 340,   JanggiElephantValueEg    = 350,
  BannerValueMg            = 3400,  BannerValueEg            = 3500,
  WazirValueMg             = 400,   WazirValueEg             = 350,
  CommonerValueMg          = 700,   CommonerValueEg          = 900,
  CentaurValueMg           = 1800,  CentaurValueEg           = 1900,

  MidgameLimit  = 15258, EndgameLimit  = 3915
};

constexpr int PIECE_TYPE_BITS = 7; // PIECE_TYPE_NB = pow(2, PIECE_TYPE_BITS)

enum PieceType {
  NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN,
  FERS, MET = FERS, ALFIL, FERS_ALFIL, SILVER, KHON = SILVER, AIWOK, BERS, DRAGON = BERS,
  ARCHBISHOP, CHANCELLOR, AMAZON, KNIBIS, BISKNI, KNIROO, ROOKNI,
  SHOGI_PAWN, LANCE, SHOGI_KNIGHT, GOLD, DRAGON_HORSE,
  CLOBBER_PIECE, BREAKTHROUGH_PIECE, IMMOBILE_PIECE, CANNON, JANGGI_CANNON,
  SOLDIER, HORSE, ELEPHANT, JANGGI_ELEPHANT, BANNER,
  WAZIR, COMMONER, CENTAUR,

  CUSTOM_PIECE_1, CUSTOM_PIECE_2, CUSTOM_PIECE_3, CUSTOM_PIECE_4,
  CUSTOM_PIECE_5, CUSTOM_PIECE_6, CUSTOM_PIECE_7, CUSTOM_PIECE_8,

  PIECE_TYPE_NB = 1 << PIECE_TYPE_BITS,
  KING = PIECE_TYPE_NB - 1,

  // Aliases
  CUSTOM_PIECES = CUSTOM_PIECE_1,
  CUSTOM_PIECES_END = KING - 1,
  CUSTOM_PIECES_ROYAL = CUSTOM_PIECES_END,
  CUSTOM_PIECES_NB = CUSTOM_PIECES_END - CUSTOM_PIECES + 1,
  FAIRY_PIECES = QUEEN + 1,
  FAIRY_PIECES_END = CUSTOM_PIECES - 1,
  ALL_PIECES = 0,
};
static_assert(KING < PIECE_TYPE_NB, "KING exceeds PIECE_TYPE_NB.");
static_assert(PIECE_TYPE_BITS <= 7, "PIECE_TYPE_BITS > 7 overflows PieceSet (unsigned __int128, KING must be <= bit 127)");
static_assert(!(PIECE_TYPE_NB & (PIECE_TYPE_NB - 1)), "PIECE_TYPE_NB is not a power of 2");

#ifdef LARGEBOARDS
static_assert(2 * SQUARE_BITS + MOVE_TYPE_BITS + 2 * PIECE_TYPE_BITS <= 64, "Move encoding uses more than 64 bits");
#else
static_assert(2 * SQUARE_BITS + MOVE_TYPE_BITS + 2 * PIECE_TYPE_BITS <= 32, "Move encoding uses more than 32 bits");
#endif

enum Piece {
  NO_PIECE,
  W_PAWN = PAWN,                 W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING = KING,
  B_PAWN = PAWN + PIECE_TYPE_NB, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING = KING + PIECE_TYPE_NB,
  PIECE_NB = 2 * PIECE_TYPE_NB
};

// PieceSet is a 128-bit bitmask over PieceType values (KING can be up to bit 127).
// GCC/Clang extension: enum with unsigned __int128 underlying type is supported.
enum PieceSet : unsigned __int128 {
  NO_PIECE_SET = 0,
  CHESS_PIECES        = ((unsigned __int128)1 << PAWN)   | ((unsigned __int128)1 << KNIGHT)
                      | ((unsigned __int128)1 << BISHOP) | ((unsigned __int128)1 << ROOK)
                      | ((unsigned __int128)1 << QUEEN)  | ((unsigned __int128)1 << KING),
  COMMON_FAIRY_PIECES = ((unsigned __int128)1 << IMMOBILE_PIECE) | ((unsigned __int128)1 << COMMONER)
                      | ((unsigned __int128)1 << ARCHBISHOP)     | ((unsigned __int128)1 << CHANCELLOR),
  SHOGI_PIECES        = ((unsigned __int128)1 << SHOGI_PAWN) | ((unsigned __int128)1 << GOLD)
                      | ((unsigned __int128)1 << SILVER)     | ((unsigned __int128)1 << SHOGI_KNIGHT)
                      | ((unsigned __int128)1 << LANCE)      | ((unsigned __int128)1 << DRAGON)
                      | ((unsigned __int128)1 << DRAGON_HORSE) | ((unsigned __int128)1 << KING),
  COMMON_STEP_PIECES  = ((unsigned __int128)1 << COMMONER) | ((unsigned __int128)1 << FERS)
                      | ((unsigned __int128)1 << WAZIR)    | ((unsigned __int128)1 << BREAKTHROUGH_PIECE),
};

enum RiderType : int {
  NO_RIDER = 0,
  RIDER_BISHOP = 1 << 0,
  RIDER_ROOK_H = 1 << 1,
  RIDER_ROOK_V = 1 << 2,
  RIDER_CANNON_H = 1 << 3,
  RIDER_CANNON_V = 1 << 4,
  RIDER_LAME_DABBABA = 1 << 5,
  RIDER_HORSE = 1 << 6,
  RIDER_ELEPHANT = 1 << 7,
  RIDER_JANGGI_ELEPHANT = 1 << 8,
  RIDER_CANNON_DIAG = 1 << 9,
  RIDER_NIGHTRIDER = 1 << 10,
  RIDER_GRASSHOPPER_H = 1 << 11,
  RIDER_GRASSHOPPER_V = 1 << 12,
  RIDER_GRASSHOPPER_D = 1 << 13,
  HOPPING_RIDERS =  RIDER_CANNON_H | RIDER_CANNON_V | RIDER_CANNON_DIAG
                  | RIDER_GRASSHOPPER_H | RIDER_GRASSHOPPER_V | RIDER_GRASSHOPPER_D,
  LAME_LEAPERS = RIDER_LAME_DABBABA | RIDER_HORSE | RIDER_ELEPHANT | RIDER_JANGGI_ELEPHANT,
  ASYMMETRICAL_RIDERS =  RIDER_HORSE | RIDER_JANGGI_ELEPHANT
                       | RIDER_GRASSHOPPER_H | RIDER_GRASSHOPPER_V | RIDER_GRASSHOPPER_D,
  NON_SLIDING_RIDERS = HOPPING_RIDERS | LAME_LEAPERS | RIDER_NIGHTRIDER,
};

extern Value PieceValue[PHASE_NB][PIECE_NB];
extern Value EvalPieceValue[PHASE_NB][PIECE_NB]; // variant piece values for evaluation
extern Value CapturePieceValue[PHASE_NB][PIECE_NB]; // variant piece values for captures/search

typedef int Depth;

enum : int {
  DEPTH_QS_CHECKS     =  0,
  DEPTH_QS_NO_CHECKS  = -1,
  DEPTH_QS_RECAPTURES = -5,
  DEPTH_QS_MAX        = -32,

  DEPTH_NONE   = -6,

  DEPTH_OFFSET = -7 // value used only for TT entry occupancy check
};

enum Square : int {
#ifdef LARGEBOARDS
  // 17 files (A-Q) x 17 ranks (1-17) = 289 squares; stride = FILE_NB = 17
  SQ_A1,  SQ_B1,  SQ_C1,  SQ_D1,  SQ_E1,  SQ_F1,  SQ_G1,  SQ_H1,  SQ_I1,  SQ_J1,  SQ_K1,  SQ_L1,  SQ_M1,  SQ_N1,  SQ_O1,  SQ_P1,  SQ_Q1,
  SQ_A2,  SQ_B2,  SQ_C2,  SQ_D2,  SQ_E2,  SQ_F2,  SQ_G2,  SQ_H2,  SQ_I2,  SQ_J2,  SQ_K2,  SQ_L2,  SQ_M2,  SQ_N2,  SQ_O2,  SQ_P2,  SQ_Q2,
  SQ_A3,  SQ_B3,  SQ_C3,  SQ_D3,  SQ_E3,  SQ_F3,  SQ_G3,  SQ_H3,  SQ_I3,  SQ_J3,  SQ_K3,  SQ_L3,  SQ_M3,  SQ_N3,  SQ_O3,  SQ_P3,  SQ_Q3,
  SQ_A4,  SQ_B4,  SQ_C4,  SQ_D4,  SQ_E4,  SQ_F4,  SQ_G4,  SQ_H4,  SQ_I4,  SQ_J4,  SQ_K4,  SQ_L4,  SQ_M4,  SQ_N4,  SQ_O4,  SQ_P4,  SQ_Q4,
  SQ_A5,  SQ_B5,  SQ_C5,  SQ_D5,  SQ_E5,  SQ_F5,  SQ_G5,  SQ_H5,  SQ_I5,  SQ_J5,  SQ_K5,  SQ_L5,  SQ_M5,  SQ_N5,  SQ_O5,  SQ_P5,  SQ_Q5,
  SQ_A6,  SQ_B6,  SQ_C6,  SQ_D6,  SQ_E6,  SQ_F6,  SQ_G6,  SQ_H6,  SQ_I6,  SQ_J6,  SQ_K6,  SQ_L6,  SQ_M6,  SQ_N6,  SQ_O6,  SQ_P6,  SQ_Q6,
  SQ_A7,  SQ_B7,  SQ_C7,  SQ_D7,  SQ_E7,  SQ_F7,  SQ_G7,  SQ_H7,  SQ_I7,  SQ_J7,  SQ_K7,  SQ_L7,  SQ_M7,  SQ_N7,  SQ_O7,  SQ_P7,  SQ_Q7,
  SQ_A8,  SQ_B8,  SQ_C8,  SQ_D8,  SQ_E8,  SQ_F8,  SQ_G8,  SQ_H8,  SQ_I8,  SQ_J8,  SQ_K8,  SQ_L8,  SQ_M8,  SQ_N8,  SQ_O8,  SQ_P8,  SQ_Q8,
  SQ_A9,  SQ_B9,  SQ_C9,  SQ_D9,  SQ_E9,  SQ_F9,  SQ_G9,  SQ_H9,  SQ_I9,  SQ_J9,  SQ_K9,  SQ_L9,  SQ_M9,  SQ_N9,  SQ_O9,  SQ_P9,  SQ_Q9,
  SQ_A10, SQ_B10, SQ_C10, SQ_D10, SQ_E10, SQ_F10, SQ_G10, SQ_H10, SQ_I10, SQ_J10, SQ_K10, SQ_L10, SQ_M10, SQ_N10, SQ_O10, SQ_P10, SQ_Q10,
  SQ_A11, SQ_B11, SQ_C11, SQ_D11, SQ_E11, SQ_F11, SQ_G11, SQ_H11, SQ_I11, SQ_J11, SQ_K11, SQ_L11, SQ_M11, SQ_N11, SQ_O11, SQ_P11, SQ_Q11,
  SQ_A12, SQ_B12, SQ_C12, SQ_D12, SQ_E12, SQ_F12, SQ_G12, SQ_H12, SQ_I12, SQ_J12, SQ_K12, SQ_L12, SQ_M12, SQ_N12, SQ_O12, SQ_P12, SQ_Q12,
  SQ_A13, SQ_B13, SQ_C13, SQ_D13, SQ_E13, SQ_F13, SQ_G13, SQ_H13, SQ_I13, SQ_J13, SQ_K13, SQ_L13, SQ_M13, SQ_N13, SQ_O13, SQ_P13, SQ_Q13,
  SQ_A14, SQ_B14, SQ_C14, SQ_D14, SQ_E14, SQ_F14, SQ_G14, SQ_H14, SQ_I14, SQ_J14, SQ_K14, SQ_L14, SQ_M14, SQ_N14, SQ_O14, SQ_P14, SQ_Q14,
  SQ_A15, SQ_B15, SQ_C15, SQ_D15, SQ_E15, SQ_F15, SQ_G15, SQ_H15, SQ_I15, SQ_J15, SQ_K15, SQ_L15, SQ_M15, SQ_N15, SQ_O15, SQ_P15, SQ_Q15,
  SQ_A16, SQ_B16, SQ_C16, SQ_D16, SQ_E16, SQ_F16, SQ_G16, SQ_H16, SQ_I16, SQ_J16, SQ_K16, SQ_L16, SQ_M16, SQ_N16, SQ_O16, SQ_P16, SQ_Q16,
  SQ_A17, SQ_B17, SQ_C17, SQ_D17, SQ_E17, SQ_F17, SQ_G17, SQ_H17, SQ_I17, SQ_J17, SQ_K17, SQ_L17, SQ_M17, SQ_N17, SQ_O17, SQ_P17, SQ_Q17,
#else
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
#endif
  SQ_NONE,

  SQUARE_ZERO = 0,
#ifdef LARGEBOARDS
  SQUARE_NB = 289,       // 17 x 17
  SQUARE_BIT_MASK = 511, // (1 << SQUARE_BITS) - 1 = (1 << 9) - 1
#else
  SQUARE_NB = 64,
  SQUARE_BIT_MASK = 63,
#endif
  SQ_MAX = SQUARE_NB - 1,
  SQUARE_NB_CHESS = 64,
  SQUARE_NB_SHOGI = 81,
};

enum Direction : int {
#ifdef LARGEBOARDS
  NORTH =  17, // = FILE_NB; stride for a 17-file board (Dai Dai Shogi max)
#else
  NORTH =  8,
#endif
  EAST  =  1,
  SOUTH = -NORTH,
  WEST  = -EAST,

  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  NORTH_WEST = NORTH + WEST
};

enum File : int {
#ifdef LARGEBOARDS
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
  FILE_I, FILE_J, FILE_K, FILE_L, FILE_M, FILE_N, FILE_O, FILE_P, FILE_Q,
#else
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,
#endif
  FILE_NB,
  FILE_MAX = FILE_NB - 1
};

enum Rank : int {
#ifdef LARGEBOARDS
  RANK_1,  RANK_2,  RANK_3,  RANK_4,  RANK_5,  RANK_6,  RANK_7,  RANK_8,
  RANK_9,  RANK_10, RANK_11, RANK_12, RANK_13, RANK_14, RANK_15, RANK_16, RANK_17,
#else
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
#endif
  RANK_NB,
  RANK_MAX = RANK_NB - 1
};

// Keep track of what a move changes on the board (used by NNUE)
struct DirtyPiece {

  // Number of changed pieces
  int dirty_num;

  // Max 3 pieces can change in one move. A promotion with capture moves
  // both the pawn and the captured piece to SQ_NONE and the piece promoted
  // to from SQ_NONE to the capture square.
  Piece piece[12];
  Piece handPiece[12];
  int handCount[12];

  // From and to squares, which may be SQ_NONE
  Square from[12];
  Square to[12];
};

/// Score enum stores a middlegame and an endgame value in a single integer (enum).
/// The least significant 16 bits are used to store the middlegame value and the
/// upper 16 bits are used to store the endgame value. We have to take care to
/// avoid left-shifting a signed int to avoid undefined behavior.
enum Score : int { SCORE_ZERO };

constexpr Score make_score(int mg, int eg) {
  return Score((int)((unsigned int)eg << 16) + mg);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value eg_value(Score s) {
  union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
  return Value(eg.s);
}

inline Value mg_value(Score s) {
  union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
  return Value(mg.s);
}

#define ENABLE_BIT_OPERATORS_ON(T)                                        \
constexpr T operator~ (T d) { return (T)~(int)d; }                        \
constexpr T operator| (T d1, T d2) { return (T)((int)d1 | (int)d2); }     \
constexpr T operator& (T d1, T d2) { return (T)((int)d1 & (int)d2); }     \
constexpr T operator^ (T d1, T d2) { return (T)((int)d1 ^ (int)d2); }     \
inline T& operator|= (T& d1, T d2) { return (T&)((int&)d1 |= (int)d2); }  \
inline T& operator&= (T& d1, T d2) { return (T&)((int&)d1 &= (int)d2); }  \
inline T& operator^= (T& d1, T d2) { return (T&)((int&)d1 ^= (int)d2); }

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); }    \
constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); }    \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }       \
inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Value)
ENABLE_FULL_OPERATORS_ON(Direction)

ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
ENABLE_INCR_OPERATORS_ON(CheckCount)

ENABLE_BASE_OPERATORS_ON(Score)

ENABLE_BASE_OPERATORS_ON(PieceType)
ENABLE_BIT_OPERATORS_ON(RiderType)
ENABLE_BASE_OPERATORS_ON(RiderType)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON
#undef ENABLE_BIT_OPERATORS_ON

constexpr PieceSet piece_set(PieceType pt) {
  return PieceSet((unsigned __int128)1 << pt);
}

constexpr PieceSet operator~ (PieceSet ps) { return (PieceSet)~(unsigned __int128)ps; }
constexpr PieceSet operator| (PieceSet ps1, PieceSet ps2) { return (PieceSet)((unsigned __int128)ps1 | (unsigned __int128)ps2); }
constexpr PieceSet operator| (PieceSet ps, PieceType pt) { return ps | piece_set(pt); }
constexpr PieceSet operator& (PieceSet ps1, PieceSet ps2) { return (PieceSet)((unsigned __int128)ps1 & (unsigned __int128)ps2); }
constexpr PieceSet operator& (PieceSet ps, PieceType pt) { return ps & piece_set(pt); }
constexpr PieceSet operator^ (PieceSet ps1, PieceSet ps2) { return (PieceSet)((unsigned __int128)ps1 ^ (unsigned __int128)ps2); }
constexpr PieceSet operator^ (PieceSet ps, PieceType pt) { return ps ^ piece_set(pt); }
inline PieceSet& operator|= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((unsigned __int128&)ps1 |= (unsigned __int128)ps2); }
inline PieceSet& operator|= (PieceSet& ps, PieceType pt) { return ps |= piece_set(pt); }
inline PieceSet& operator&= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((unsigned __int128&)ps1 &= (unsigned __int128)ps2); }
//inline PieceSet& operator&= (PieceSet& ps, PieceType pt) does not make sense
inline PieceSet& operator^= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((unsigned __int128&)ps1 ^= (unsigned __int128)ps2); }
inline PieceSet& operator^= (PieceSet& ps, PieceType pt) { return ps ^= piece_set(pt); }

static_assert(piece_set(PAWN) & PAWN);
static_assert(piece_set(KING) & KING);

/// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

/// Only declared but not defined. We don't want to multiply two scores due to
/// a very high risk of overflow. So user should explicitly convert to integer.
Score operator*(Score, Score) = delete;

/// Division of a Score must be handled separately for each term
inline Score operator/(Score s, int i) {
  return make_score(mg_value(s) / i, eg_value(s) / i);
}

/// Multiplication of a Score by an integer. We check for overflow in debug mode.
inline Score operator*(Score s, int i) {

  Score result = Score(int(s) * i);

  assert(eg_value(result) == (i * eg_value(s)));
  assert(mg_value(result) == (i * mg_value(s)));
  assert((i == 0) || (result / i) == s);

  return result;
}

/// Multiplication of a Score by a boolean
inline Score operator*(Score s, bool b) {
  return b ? s : SCORE_ZERO;
}

constexpr Color operator~(Color c) {
  return Color(c ^ BLACK); // Toggle color
}

constexpr Square flip_rank(Square s, Rank maxRank = RANK_8) { // Swap A1 <-> A8
  return Square(s + NORTH * (maxRank - 2 * (s / NORTH)));
}

constexpr Square flip_file(Square s, File maxFile = FILE_H) { // Swap A1 <-> H1
  return Square(s + maxFile - 2 * (s % NORTH));
}

constexpr Piece operator~(Piece pc) {
  return Piece(pc ^ PIECE_TYPE_NB);  // Swap color of piece B_KNIGHT <-> W_KNIGHT
}

constexpr CastlingRights operator&(Color c, CastlingRights cr) {
  return CastlingRights((c == WHITE ? WHITE_CASTLING : BLACK_CASTLING) & cr);
}

constexpr Value mate_in(int ply) {
  return VALUE_MATE - ply;
}

constexpr Value mated_in(int ply) {
  return -VALUE_MATE + ply;
}

constexpr Value convert_mate_value(Value v, int ply) {
  return  v ==  VALUE_MATE ? mate_in(ply)
        : v == -VALUE_MATE ? mated_in(ply)
        : v;
}

constexpr Square make_square(File f, Rank r) {
  return Square(r * FILE_NB + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
  return Piece((c << PIECE_TYPE_BITS) + pt);
}

constexpr PieceType type_of(Piece pc) {
  return PieceType(pc & (PIECE_TYPE_NB - 1));
}

inline Color color_of(Piece pc) {
  assert(pc != NO_PIECE);
  return Color(pc >> PIECE_TYPE_BITS);
}

constexpr bool is_ok(Square s) {
  return s >= SQ_A1 && s <= SQ_MAX;
}

constexpr File file_of(Square s) {
  return File(s % FILE_NB);
}

constexpr Rank rank_of(Square s) {
  return Rank(s / FILE_NB);
}

constexpr Rank relative_rank(Color c, Rank r, Rank maxRank = RANK_8) {
  return Rank(c == WHITE ? r : maxRank - r);
}

constexpr Rank relative_rank(Color c, Square s, Rank maxRank = RANK_8) {
  return relative_rank(c, rank_of(s), maxRank);
}

constexpr Square relative_square(Color c, Square s, Rank maxRank = RANK_8) {
  return make_square(file_of(s), relative_rank(c, s, maxRank));
}

constexpr Direction pawn_push(Color c) {
  return c == WHITE ? NORTH : SOUTH;
}

constexpr MoveType type_of(Move m) {
  return MoveType(m & (15 << (2 * SQUARE_BITS)));
}

constexpr Square to_sq(Move m) {
  return Square(m & SQUARE_BIT_MASK);
}

constexpr Square from_sq(Move m) {
  return type_of(m) == DROP ? SQ_NONE : Square((m >> SQUARE_BITS) & SQUARE_BIT_MASK);
}

inline int from_to(Move m) {
 return to_sq(m) + (from_sq(m) << SQUARE_BITS);
}

inline PieceType promotion_type(Move m) {
  return type_of(m) == PROMOTION ? PieceType((m >> (2 * SQUARE_BITS + MOVE_TYPE_BITS)) & (PIECE_TYPE_NB - 1)) : NO_PIECE_TYPE;
}

inline PieceType gating_type(Move m) {
  return PieceType((m >> (2 * SQUARE_BITS + MOVE_TYPE_BITS)) & (PIECE_TYPE_NB - 1));
}

inline Square gating_square(Move m) {
  return Square((m >> (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS)) & SQUARE_BIT_MASK);
}

inline bool is_gating(Move m) {
  return gating_type(m) && (type_of(m) == NORMAL || type_of(m) == CASTLING);
}

inline bool is_pass(Move m) {
  return type_of(m) == SPECIAL && from_sq(m) == to_sq(m);
}

constexpr Move make_move(Square from, Square to) {
  return Move((from << SQUARE_BITS) + to);
}

template<MoveType T>
inline Move make(Square from, Square to, PieceType pt = NO_PIECE_TYPE) {
  return Move((pt << (2 * SQUARE_BITS + MOVE_TYPE_BITS)) + T + (from << SQUARE_BITS) + to);
}

constexpr Move make_drop(Square to, PieceType pt_in_hand, PieceType pt_dropped) {
  return Move((pt_in_hand << (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS)) + (pt_dropped << (2 * SQUARE_BITS + MOVE_TYPE_BITS)) + DROP + to);
}

constexpr Move reverse_move(Move m) {
  return make_move(to_sq(m), from_sq(m));
}

template<MoveType T>
constexpr Move make_gating(Square from, Square to, PieceType pt, Square gate) {
  // Cast gate to Move before shifting — in LARGEBOARDS, SQUARE_BITS=9 and the gate
  // shift reaches bit 29+, which overflows 32-bit int (undefined behaviour).
  return Move((Move(gate) << (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS)) + (pt << (2 * SQUARE_BITS + MOVE_TYPE_BITS)) + T + (from << SQUARE_BITS) + to);
}

constexpr PieceType dropped_piece_type(Move m) {
  return PieceType((m >> (2 * SQUARE_BITS + MOVE_TYPE_BITS)) & (PIECE_TYPE_NB - 1));
}

constexpr PieceType in_hand_piece_type(Move m) {
  return PieceType((m >> (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS)) & (PIECE_TYPE_NB - 1));
}

inline bool is_custom(PieceType pt) {
  return pt >= CUSTOM_PIECES && pt <= CUSTOM_PIECES_END;
}

inline bool is_ok(Move m) {
  return from_sq(m) != to_sq(m) || type_of(m) == PROMOTION || type_of(m) == SPECIAL
      || type_of(m) == LION_MOVE || type_of(m) == LION_DOG_MOVE;
}

// Lion double-move helpers.
// mid_sq / gating_type are reused: gating_square = mid square, gating_type = piece type captured at mid.
inline Square mid_sq(Move m) { return gating_square(m); }  // mid1_sq for both LION_MOVE and LION_DOG_MOVE
inline PieceType mid_type(Move m) { return gating_type(m); }  // mid1_pt for both LION_MOVE and LION_DOG_MOVE
inline bool has_mid_capture(Move m) { return (type_of(m) == LION_MOVE || type_of(m) == LION_DOG_MOVE) && mid_type(m) != NO_PIECE_TYPE; }
inline Move make_lion(Square from, Square to, Square mid, PieceType midCapturedPt = NO_PIECE_TYPE) {
    // Use Move() casts to avoid 32-bit int overflow at shift 29 in LARGEBOARDS builds.
    // (make_lion_dog uses the same pattern; make_gating had UB here.)
    return Move(LION_MOVE
               | (Move(mid) << (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS))
               | (Move(midCapturedPt) << (2 * SQUARE_BITS + MOVE_TYPE_BITS))
               | (Move(from) << SQUARE_BITS) | to);
}

// Lion Dog triple-move helpers.
// Bit layout (LARGEBOARDS 64-bit): to[0..8] from[9..17] type[18..21] mid1_pt[22..27] mid1_sq[28..36]
//                                   mid2_pt[37..42] mid2_sq[43..51]
constexpr int LD_MID2_PT_SHIFT = 2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS + SQUARE_BITS;
constexpr int LD_MID2_SQ_SHIFT = 2 * SQUARE_BITS + MOVE_TYPE_BITS + 2 * PIECE_TYPE_BITS + SQUARE_BITS;
inline PieceType mid2_type(Move m) { return PieceType((m >> LD_MID2_PT_SHIFT) & (PIECE_TYPE_NB - 1)); }
inline Square    mid2_sq(Move m)   { return Square((m >> LD_MID2_SQ_SHIFT) & SQUARE_BIT_MASK); }
inline bool has_mid2_capture(Move m) { return type_of(m) == LION_DOG_MOVE && mid2_type(m) != NO_PIECE_TYPE; }
inline Move make_lion_dog(Square from, Square to,
                          Square mid1, PieceType mid1Pt,
                          Square mid2 = SQ_NONE, PieceType mid2Pt = NO_PIECE_TYPE) {
    return Move(LION_DOG_MOVE | (Move(mid2Pt) << LD_MID2_PT_SHIFT) | (Move(mid2) << LD_MID2_SQ_SHIFT)
                              | (Move(mid1) << (2 * SQUARE_BITS + MOVE_TYPE_BITS + PIECE_TYPE_BITS))
                              | (Move(mid1Pt) << (2 * SQUARE_BITS + MOVE_TYPE_BITS))
                              | (Move(from) << SQUARE_BITS) | to);
}

inline int dist(Direction d) {
  return std::abs(d % NORTH) < NORTH / 2 ? std::max(std::abs(d / NORTH), int(std::abs(d % NORTH)))
      : std::max(std::abs(d / NORTH) + 1, int(NORTH - std::abs(d % NORTH)));
}

/// Based on a congruential pseudo random number generator
constexpr Key make_key(uint64_t seed) {
  return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

} // namespace Stockfish

#endif // #ifndef TYPES_H_INCLUDED

#include "tune.h" // Global visibility to tuning setup
