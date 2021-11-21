#include <stdio.h>
#include "memory.h"
#include "value.h"

void initValueArray(PValueArray arr){
	arr->capacity = 0;
	arr->count = 0;
	arr->values = NULL;
}
void writeValueArray(PValueArray array, Value value) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values,
		oldCapacity, array->capacity);
  }
  array->values[array->count++] = value;
}
void freeValueArray(PValueArray array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value){
	switch(value.type){
		case NUMBER:
			printf("%g",AS_NUMBER(value));
			break;
		case BOOL:
			printf(AS_BOOL(value) ? "true" : "false");
			break;
		case NIL:
			printf("null"); break;		
		case OBJ:
			printObject(value);break;
	}
}
bool valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case NIL:    return true;
    case NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
	case OBJ: {
		if (IS_STRING(a)){
			PObjString str_a = AS_STRING(a);
			PObjString str_b = AS_STRING(b);
			return strcmp(str_a->chars,str_b->chars) == 0;
		}
	}
    default:
      return false; 
  }
}
PObj allocateObject(size_t size, ObjType type){
	PObj obj = (PObj)reallocate(NULL,0,size);
	obj->type = type;
	obj->next = vm.objects;
	vm.objects = obj;
	return obj;
}

PObj copyString(const char* start, int len){
	char* mem = ALLOCATE(char, len+1);
	memcpy(mem,start,len);
	mem[len] = '\0';
	PObjString str = ALLOCATE_OBJ(ObjString,OBJ_STRING);
	str->length = len;
	str->chars = mem;
	return (PObj)str;
}
void printObject(Value value){
	switch (OBJ_TYPE(value)){
		case OBJ_STRING:
			printf("%s", AS_CSTRING(value));
			break;
	}
}
PObjString concat(Value a, Value b){
	PObjString str_a = AS_STRING(a);
	PObjString str_b = AS_STRING(b);
	int total_len = str_a->length + str_b->length;
	char* str = ALLOCATE(char, total_len +1);
	memcpy(str,str_a->chars,str_a->length);
	memcpy(str + str_a->length,str_b->chars, str_b->length);
	str[total_len] = '\0';
	PObjString result = ALLOCATE_OBJ(ObjString,OBJ_STRING);
	result->length = total_len;
	result->chars = str;
	return result;
}