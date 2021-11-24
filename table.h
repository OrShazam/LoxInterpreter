
#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

typedef struct _Entry {
	Value key;
	Value value;
} Entry, *PEntry;

typedef struct _Table {
	int count;
	int capacity;
	PEntry entries;
} Table, *PTable;

void initTable(PTable);
void freeTable(PTable);
bool tableSet(PTable,PValue,PValue);
bool tableGet(PTable,PValue,PValue);
bool tableDelete(PTable, PValue);
void tableCopy(PTable src, PTable dst);

#endif