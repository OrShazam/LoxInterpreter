#include "common.h"
#include "chunk.h"
#include "memory.h"
#include "debug.h"
#include "vm.h"
int main(int argc, char* argv[]){
	initVM();
	Chunk chunk;
	initChunk(&chunk);
	writeConstant(&chunk, 1.2, 1);
    writeChunk(&chunk, OP_RETURN,2);
	disassembleChunk(&chunk,"test chunk");
	freeChunk(&chunk);
	freeVM();
	return 0;
}