#ifndef clox_memory_h
#define clox_memory_h
#include "common.h"
#include "value.h"
#include "vm.h"

#define GROW_CAPACITY(capacity) \
		((capacity) < 8 ? 8 : (capacity) * 2)
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
		(type*)reallocate(pointer,sizeof(type) * (oldCount), \
			sizeof(type) * (newCount))
#define FREE_ARRAY(type, pointer, oldCount) \
		reallocate(pointer,sizeof(type) * (oldCount), 0)
		
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define ALLOCATE_OBJ(type, objectType) \
	(type*)allocateObject(sizeof(type),objectType)
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)
#define FREE_CONST(size, pointer) reallocate(pointer,size,0)

void* reallocate(void*, size_t, size_t);
void freeObjects();
#endif