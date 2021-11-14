#include <stdio.h>
#include "memory.h"
#include "line.h"

void initLineInfo(PLineInfo arr){
	arr->capacity = 0;
	arr->count = 0;
	arr->lines = NULL;
}
void writeLineInfo(PLineInfo array, int line, int offset) {
  while (array->capacity < line) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->lines = GROW_ARRAY(int, array->lines,
		oldCapacity, array->capacity);
  }
  array->lines[line] = offset;
  array->count++;
}
void freeLineInfo(PLineInfo array) {
  FREE_ARRAY(int, array->lines, array->capacity);
  initLineInfo(array);
}