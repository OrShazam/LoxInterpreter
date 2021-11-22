

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "table.h"

void initTable(PTable table){
	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;
}
void freeTable(PTable table){
	FREE_ARRAY(Entry, table->entries, table->capacity);
	initTable(table);
}
static PEntry findEntry(PEntry entries, int capc, Value key){
	PEntry entry;
	PEntry tombstone = NULL;
	uint32_t hash = calcHash((void*)&key.as,sizeof(Value) - sizeof(ValueType));
	uint32_t idx = hash % capc;
	for (;;){
		entry = &entries[idx];
		if (IS_NIL(entry->key)){
			if (IS_NIL(entry->value)){
				return tombstone != NULL ? tombstone : entry;
				// overwrite tombstone instead of empty entries
			} else {
				if (tombstone == NULL)
					tombstone = entry;
			}
			
		} else if (valuesEqual(key,entry->key)){
			return entry;
		}
		idx = (idx + 1) % capc;
	}
}
static void adjustCapacity(Table* table, int capc) {
	// indexes are modulo capacity, so we need to do something more complicated 
	// than just realloc when capacity changes 
	Entry* entries = ALLOCATE(Entry, capc);
	for (int i = 0; i < capc; i++) {
		entries[i].key = NIL_VAL();
		entries[i].value = NIL_VAL();
	}
	table->count = 0; // removing tombstones, so recount is needed 
	for (int i = 0; i < table->capacity; i++){
		PEntry src = &table->entries[i];
		if (IS_NIL(src->key))
			continue;
		PEntry dest = findEntry(entries,capc,src->key);
		dest->key = src->key;
		dest->value = src->value;	
		table->count++; 
	}
	FREE_ARRAY(Entry,table->entries,table->capacity);
	table->entries = entries;
	table->capacity = capc;
}
bool tableGet(PTable table, Value key, PValue value){
	if(table->count == 0)
		return false;
	PEntry entry = findEntry(table->entries,table->capacity,key);
	if (IS_NIL(entry->key))
		return false;
	*value = entry->value;
	return true;
}

bool tableSet(PTable table, Value key, Value value){
	if (table->count + 1 > table->capacity * TABLE_MAX_LOAD){
		int capc = GROW_CAPACITY(table->capacity);
		adjustCapacity(table,capc);
	}
	PEntry entry = findEntry(table->entries,table->capacity,key);
	bool isNew = IS_NIL(entry->key);
	if (isNew && IS_NIL(entry->value)) // tombstone
		table->count++;
	entry->key = key;
	entry->value = value;
	return isNew;
}
bool tableDelete(PTable table, Value key){
	if (table->count == 0)
		return false;
	PEntry entry = findEntry(table->entries,table->capacity,key);
	if (IS_NIL(entry->key))
		return false;
	 // tombstone
	entry->key = NIL_VAL();
	entry->value = BOOL_VAL(true);
	return true;
}
void tableCopy(PTable src, PTable dst){
	for (int i = 0; i < src->capacity; i++){
		PEntry entry = &src->entries[i];
		if (!IS_NIL(entry->key)){
			tableSet(dst,entry->key,entry->value);
		}
	}
}