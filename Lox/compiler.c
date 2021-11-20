
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
static void emitReturn(){
	emitByte(OP_RETURN);
}
static void number(){
	double value = strtod(parser.previous.start,NULL);
	writeConstant(currentChunk(),value,parser.previous.line);
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
		default: 
			return;
	}
}
static void parsePrecedence(Precedence prec){
	advance();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;
	if (prefixRule == NULL) {
		//error("Expect expression.");
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
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
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