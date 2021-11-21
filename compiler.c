
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;
Parser parser;
PChunk compilingChunk;
static void initParser(){
	parser.hadError = false;
	parser.panicMode = false;
}
static PChunk currentChunk(){
	return compilingChunk;
}
static void errorAt(Token* token,const char* msg){
	if (parser.panicMode) return;
	fprintf(stderr,"[Line %d] Error", token->line);
	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	} else if (token->type == TOKEN_ERROR) {
    // Nothing.
	} else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}
	fprintf(stderr, ": %s\n", msg);
	parser.hadError = true;
	parser.panicMode = true;
}
static void error(const char* msg){
	errorAt(&parser.previous,msg);
}
static void errorAtCurrent(const char* msg){
	errorAt(&parser.current,msg);
}
static void advance(){
	parser.previous = parser.current;
	
	for(;;){
		parser.current = scanToken();
		if (parser.current.type != TOKEN_ERROR)
			break;
		errorAtCurrent(parser.current.start);
	}
}
static void consume(TokenType type,const char* msg){
	if (parser.current.type == type){
		advance();
		return;
	}
	errorAtCurrent(msg);
}
static void emitByte(uint8_t byte){
	writeChunk(currentChunk(),byte,parser.previous.line);
}
static void emitBytes(uint8_t byte1, uint8_t byte2) {
	emitByte(byte1);
	emitByte(byte2);
}
static void emitConstant(Value value){
	writeConstant(currentChunk(), value,parser.previous.line);
}
static void emitReturn(){
	emitByte(OP_RETURN);
}
static void number(){
	double value = strtod(parser.previous.start,NULL);
	emitConstant(NUMBER_VAL(value));
}
static void string(){
	emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, \
		parser.previous.length -2)));
}
// forward declarations here
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
// for recursion
static void grouping(){
	expression();
	consume(TOKEN_RIGHT_PAREN, \
		"Expect ')' after expression.");
}
static void unary(){
	TokenType operatorType = parser.previous.type;
	parsePrecedence(PREC_UNARY);
	switch (operatorType){
		case TOKEN_BANG:
			emitByte(OP_NOT);break;
		case TOKEN_MINUS:
			emitByte(OP_NEGATE);break;
		default:
			return;
	}
}
static void binary(){
	TokenType operatorType = parser.previous.type;
	PParseRule rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->prec + 1)); // left associative
	switch (operatorType){
		case TOKEN_PLUS: 
			emitByte(OP_ADD); break;
		case TOKEN_MINUS:      
			emitByte(OP_SUBTRACT); break;
		case TOKEN_STAR:      
			emitByte(OP_MULTIPLY); break;
		case TOKEN_SLASH:    
			emitByte(OP_DIVIDE); break;
		case TOKEN_BANG_EQUAL:    
			emitBytes(OP_EQUAL, OP_NOT); break;
		case TOKEN_EQUAL_EQUAL:   
			emitByte(OP_EQUAL); break;
		case TOKEN_GREATER:       
			emitByte(OP_GREATER); break;
		case TOKEN_GREATER_EQUAL: 
			emitBytes(OP_LESS, OP_NOT); break;
		case TOKEN_LESS:          
			emitByte(OP_LESS); break;
		case TOKEN_LESS_EQUAL:    
			emitBytes(OP_GREATER, OP_NOT); break;
		default: 
			return;
	}
}
static void literal(){
	switch(parser.previous.type){
		case TOKEN_FALSE: emitByte(OP_FALSE); break;
		case TOKEN_TRUE: emitByte(OP_TRUE); break;
		case TOKEN_NIL: emitByte(OP_NIL); break;
		default:
			return;
	}
}
static void parsePrecedence(Precedence prec){
	advance();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;
	if (prefixRule == NULL) {
		error("Expect expression.");
		return;
	}
	prefixRule();
	while (prec <= getRule(parser.current.type)->prec) {
		advance();
		ParseFn infixRule = getRule(parser.previous.type)->infix;
		infixRule();
	}
}
static void expression(){
	parsePrecedence(PREC_ASSIGNMENT);
}
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_COMPARISON},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};
static PParseRule getRule(TokenType type){
	return &rules[type];
}
bool compile(const char* source, PChunk chunk){
	compilingChunk = chunk;
	initScanner(source);
	initParser();
	advance();
	expression();
	consume(TOKEN_EOF, "Expect end of expression");
	emitReturn();
	return !parser.hadError;
}