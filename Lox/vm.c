
#include "common.h"
#include "vm.h"

VM vm;

void initVM(){
	
}

void freeVM(){
	
}
InterpretResult interpret(PChunk chunk){
	vm.chunk = chunk;
	vm.ip = chunk->code;
	return run();
}
static InterpretResult run(){
  #define READ_BYTE() (*vm.ip++)
  for (;;) {
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_RETURN: {
        return INTERPRET_OK;
      }
    }
  }
  #undef READ_BYTE
}