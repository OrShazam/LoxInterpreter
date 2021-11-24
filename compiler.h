
#ifndef clox_compiler_h
#define clox_compiler_h
#include "chunk.h"
#include "common.h"
#include "scanner.h"
#define LOCALS_MAX UINT8_MAX + 1
bool compile(const char*, PChunk);
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;
typedef void (*ParseFn)();
typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence prec;
} ParseRule, *PParseRule;
typedef struct {
	Token name;
	int depth;
} Local;
typedef struct {
	Local locals[LOCALS_MAX];
	int localCount;
	int scopeDepth;
} Compiler, *PCompiler;
#endif