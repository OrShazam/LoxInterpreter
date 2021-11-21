#ifndef clox_line_h
#define clox_line_h

typedef struct {
	int capacity;
	int count;
	int* lines;
} LineInfo, *PLineInfo;

void initLineInfo(PLineInfo);
void writeLineInfo(PLineInfo, int, int);
void freeLineInfo(PLineInfo);

#endif