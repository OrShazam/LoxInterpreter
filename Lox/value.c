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
	printf("%g",value);
}