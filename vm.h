#ifndef clox_vm_h
#define clox_vm_h
#include "chunk.h"
#include "value.h"
#include "table.h"
typedef struct {
	PChunk chunk;
	uint8_t* ip;
	ValueArray stack;
	PObj objects;
	Table strings;
	Table globals;
} VM;
typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;
extern VM vm;
void initVM();
void freeVM();
InterpretResult interpret(const char*);
static InterpretResult run();
void push(Value);
Value pop();

#endif