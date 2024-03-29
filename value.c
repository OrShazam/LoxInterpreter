
#include "memory.h"
#include "value.h"
#include "common.h"

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
Value getValueArrayIndex(PValueArray array, int index){
	if (index < 0 || index > array->count)
		return NIL_VAL();
	return array->values[index];
}
void writeValueArrayIndex(PValueArray array, Value value, int index){
	if (index < 0 || index > array->count)
		return;
	array->values[index] = value;
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
bool valuesEqual(PValue _a, PValue _b) {
  Value a = *_a;
  Value b = *_b;
  if (a.type != b.type) return false;
  switch (a.type) {
    case BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case NIL:    return true;
    case NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
	case OBJ: return AS_OBJ(a) == AS_OBJ(b); // thanks to string interneding
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
PObjString allocateObjStr(int size){
	PObj obj = allocateObject(sizeof(ObjString) + size + 16, OBJ_STRING); 
	return (PObjString)obj;
}
static PObjString tableFindString(PTable table,const char* start, int len, uint32_t hash){
	if (table->count == 0)
		return NULL;
	PEntry entry;
	PObjString key;
	uint32_t idx = hash % table->capacity;
	for(;;){
		entry = &table->entries[idx];
		key = AS_STRING(entry->key);
		if (key == NULL){
			if (IS_NIL(entry->value)){
				return NULL;
			}
		}else if (key->length == len && 
			key->hash == hash && memcmp(&(key->chars[0]), start,len) == 0){
			return key;
		}
		idx = (idx + 1) % table->capacity;
	}
}
PObjString copyString(const char* start, int len){
	uint32_t hash = calcHash((void*)start,len);
	PObjString interned = tableFindString(&vm.strings,start,len,hash);
	if (interned != NULL){
		return interned;
	}
	PObjString str = allocateObjStr(len+1);
	str->length = len;
	memcpy(str->chars,start,len);
	str->chars[len] = '\0';
	str->hash = hash;
	Value strValue = OBJ_VAL(str);
	Value nil = NIL_VAL();
	tableSet(&vm.strings,&strValue,&nil);
	return str;
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
	PObjString result = allocateObjStr(total_len+1);
	result->length = total_len;
	memcpy(&(result->chars[0]),&(str_a->chars[0]),str_a->length);
	memcpy(&(result->chars[0]) + str_a->length,&(str_b->chars[0]), str_b->length);
	result->chars[total_len] = '\0';
	uint32_t hash = calcHash((void*)&(result->chars[0]),result->length);
	PObjString interned = tableFindString(&vm.strings,&(result->chars[0]),result->length,hash);
	if (interned != NULL){
		free(result);
		return interned;
	}
	result->hash = hash;
	Value value = OBJ_VAL(result);
	Value nil = NIL_VAL();
	tableSet(&vm.strings,&value,&nil);
	return result;
}
uint32_t calcHash(const void* key, int len){
	char* chkey = (char*)key;
	uint32_t hash = 2166136261u;
	for (int i = 0; i < len; i++){
		hash ^= chkey[i];
		hash *= 16777619;
	}
	return hash;
}
uint32_t calcHashGeneric(PValue key){
	switch(key->type){
		case BOOL: {
			bool actual = AS_BOOL(*key);
			return calcHash((void*)&actual,sizeof(bool));
		}
		case NUMBER: {
			double actual = AS_NUMBER(*key);
			return calcHash((void*)&actual,sizeof(double));
		}
		case OBJ: {
			if (IS_STRING(*key)){
				PObjString str = AS_STRING(*key);
				return calcHash((void*)&str->chars[0],str->length);
			}
		}
		default: {
			return 0;
		}
	}
}