#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize){
	if (newSize == 0){
		free(pointer);
		return NULL;
	}
	void* result = realloc(pointer, newSize);
	if (result == NULL) 
		exit(1);
	return result;
}
static void freeObj(PObj obj){
	switch (obj->type){
		case OBJ_STRING: {
			PObjString str = (PObjString)obj;
			FREE_ARRAY(char,str->chars,str->length+1);
			FREE(ObjString, obj);
			break;
		}
	}
}
void freeObjects(){
	PObj obj = vm.objects;
	PObj next;
	while (obj != NULL){
		next = obj->next;
		freeObj(obj);
		obj = next;
	}
}