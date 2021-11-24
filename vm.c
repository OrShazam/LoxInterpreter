
#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "memory.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
VM vm;

static void resetStack(){
	vm.stack.count = 0;
}
static void runtimeError(const char* fmt,...){
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
	fputs("\n",stderr);
	int offset = vm.ip - vm.chunk->code - 1; //ip points at next instruction 
	int line = getLine(vm.chunk,offset);
	fprintf(stderr, "[Line %d] in script\n",line);
	resetStack();
}
void initVM(){
	initValueArray(&vm.stack);
	resetStack();
	vm.objects = NULL;
	initTable(&vm.strings);
	initTable(&vm.globals);
}

void freeVM(){
	freeValueArray(&vm.stack);
	freeObjects();
	freeTable(&vm.strings);
}
void push(Value value){
	writeValueArray(&vm.stack, value);
}
Value pop(){
	return vm.stack.values[(vm.stack.count--) - 1]; 
}
static Value peek(int delta){
	return vm.stack.values[vm.stack.count - 1 - delta];
}
static bool ToBoolean(Value value){
	switch(value.type){
		case BOOL:
			return AS_BOOL(value);
		case NIL:
			return false;
		case NUMBER:
			return AS_NUMBER(value) != 0;
		default:
			return false;
	}
}
InterpretResult interpret(const char* source){
	Chunk chunk;
	initChunk(&chunk);
	clock_t time;
	time = clock();
	if (!compile(source, &chunk)){
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}
	initVM();
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;
	InterpretResult result = run();
	time = clock() - time;
	double seconds = ((double)time)/CLOCKS_PER_SEC;
	printf("\tProgram compiled and ran in %fs\n",seconds);
	freeChunk(vm.chunk);
	freeVM();
	return result;
}
static InterpretResult run(){
  #define READ_BYTE() (*vm.ip++)
  #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
  #define READ_CONSTANT_LONG() (vm.chunk->constants.values[READ_BYTE() + READ_BYTE() << 8 + READ_BYTE() << 16])
  #define BINARY_OP(TYPE_VAL,op) \
    do { \
	  if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))){ \
		runtimeError("Operands must be numbers."); \
		return INTERPRET_RUNTIME_ERROR; \
	  } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(TYPE_VAL(a op b)); \
    } while (false)
		
  for (;;) {
	  #ifdef DEBUG_TRACE_EXECUTION
		printf("\tSTACK TRACE: ");
		if (vm.stack.count == 0){
			printf("EMPTY");
		}
		for (Value* slot = vm.stack.values; slot < (vm.stack.values + vm.stack.count); slot++){
			printf("["); printValue(*slot);printf("] ");
		}
		printf("\n");
		
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
	  #endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
	  case OP_CONSTANT: {
		  Value constant = READ_CONSTANT();
		  push(constant);
		  break;
	  }
	  case OP_CONSTANT_LONG: {
		  Value constant = READ_CONSTANT_LONG();
		  push(constant);
		  break;
	  }
	  case OP_GLOBAL_SET: {
		  Value idValue = READ_CONSTANT();
		  Value popped = pop();
		  tableSet(&vm.globals,&idValue,&popped);
		  break;
	  }
	  case OP_GLOBAL_SET_LONG: {
		  Value idValue = READ_CONSTANT_LONG();
		  Value popped = pop();
		  tableSet(&vm.globals,&idValue,&popped);
		  break;
	  }	
	  case OP_GLOBAL_GET: {
		  Value idValue = READ_CONSTANT();
		  Value value;
		  if (!tableGet(&vm.globals,&idValue,&value)){
			  runtimeError("Undefined variable '%s'.",AS_STRING(idValue)->chars);
			  return INTERPRET_RUNTIME_ERROR;
		  }
		  push(value);
		  break;
	  }
	  case OP_GLOBAL_GET_LONG: {
		  Value idValue = READ_CONSTANT_LONG();
		  Value value;
		  if (!tableGet(&vm.globals,&idValue,&value)){
			  runtimeError("Undefined variable '%s'.",AS_STRING(idValue)->chars);
			  return INTERPRET_RUNTIME_ERROR;
		  }
		  push(value);
		  break;
			
	  }
	  case OP_LOCAL_GET: {
		  uint8_t slot = READ_BYTE();
		  push(getValueArrayIndex(&vm.stack,slot));
		  break;
	  }
	  case OP_LOCAL_SET: {
		  uint8_t slot = READ_BYTE();
		  writeValueArrayIndex(&vm.stack, peek(0),slot);
		  // no pop cause assignment is both statement and expression
		  break;
	  }
	  case OP_NIL: push(NIL_VAL());break;
	  case OP_TRUE: push(BOOL_VAL(true)); break;
	  case OP_FALSE: push(BOOL_VAL(false)); break;
	  case OP_EQUAL: {
		  Value b = pop();
		  Value a = pop();
		  push(BOOL_VAL(valuesEqual(&a,&b)));
		  break;	  
	  }
	  case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
	  case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
	  case OP_ADD: {
		  if (IS_STRING(peek(0)) && IS_STRING(peek(1))){
				PObjString result = concat(pop(),pop());
				push (OBJ_VAL(result));
		  }
		  else 
				BINARY_OP(NUMBER_VAL,+); 
		  break;
	  }
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL,-); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL,*); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL,/); break;
	  case OP_NOT:
		push(BOOL_VAL(!ToBoolean(pop()))); break;
	  case OP_NEGATE: 
		if (!IS_NUMBER(peek(0))){
			runtimeError("Operand must be a number");
			return INTERPRET_RUNTIME_ERROR;
		}
		push(NUMBER_VAL(-AS_NUMBER(pop())));
		break;	  
	  case OP_PRINT: {
		  printValue(pop());
		  printf("\n");
		  break;
	  }
	  case OP_POP: pop();break;
	  case OP_POPN: {
		  uint8_t count = READ_BYTE();
		  while (count--)
			  pop();
		  break;
	  }
      case OP_RETURN: {
        return INTERPRET_OK;
      }
	  
    }
  }
  #undef READ_BYTE
  #undef READ_CONSTANT
  #undef READ_CONSTANT_LONG
  #undef BINARY_OP
}