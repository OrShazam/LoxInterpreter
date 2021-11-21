#ifndef clox_value_h
#define clox_value_h
#include "common.h"

typedef enum {
	OBJ_STRING
} ObjType;

typedef struct _Obj{
	ObjType type;
	struct _Obj* next;
} Obj, *PObj;

typedef struct {
	Obj obj;
	int length;
	char chars[];
} __attribute__((packed, aligned(1))) ObjString, *PObjString;

typedef enum {
	BOOL,
	NIL,
	NUMBER,
	OBJ
} ValueType;

typedef struct _Value{
	ValueType type;
	union {
		bool boolean;
		double number;
		PObj obj;
	} as;
} Value;


#define IS_BOOL(value) ((value).type == BOOL)
#define AS_BOOL(value) ((value).as.boolean)
#define BOOL_VAL(value) ((Value){BOOL,{.boolean = value}})
#define IS_NUMBER(value) ((value).type == NUMBER)
#define AS_NUMBER(value) ((value).as.number)
#define NUMBER_VAL(value) ((Value){NUMBER,{.number = value}})
#define IS_NIL(value) ((value).type == NIL)
#define NIL_VAL() ((Value){NIL,{.number=0}})
#define IS_OBJ(value) ((value).type == OBJ)
#define AS_OBJ(value) ((value).as.obj)
#define OBJ_VAL(value) ((Value){OBJ, {.obj = (PObj)value}})


#define OBJ_TYPE(value) (AS_OBJ(value)->type)


static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && OBJ_TYPE(value) == type;
} // not a macro because a macro just copies text, which means value will get evaluated twice

#define IS_STRING(value) 		isObjType(value,OBJ_STRING)
#define AS_STRING(value)       ((PObjString)AS_OBJ(value))
#define AS_CSTRING(value)      (((PObjString)AS_OBJ(value))->chars)

PObj copyString(const char*, int);
void printObject(Value);
PObjString concat(Value, Value);


typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray, *PValueArray;
void initValueArray(PValueArray);
void writeValueArray(PValueArray, Value);
void freeValueArray(PValueArray);
void printValue(Value);
bool valuesEqual(Value, Value);
#endif