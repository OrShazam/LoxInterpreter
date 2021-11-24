
#include "compiler.h"

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;
Parser parser;
PChunk compilingChunk;
bool canAssign;
PCompiler current = NULL;
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
static bool check(TokenType type){
	return parser.current.type == type;
}
static bool match(TokenType type){
	if(!check(type)) 
		return false;
	advance();
	return true;
}
static void emitByte(uint8_t byte){
	writeChunk(currentChunk(),byte,parser.previous.line);
}
static void emitBytes(uint8_t byte1, uint8_t byte2) {
	emitByte(byte1);
	emitByte(byte2);
}
static int emitConstant(Value value){
	return writeConstant(currentChunk(), value,parser.previous.line);
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
static void statement();
static void declaration();
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
	canAssign = prec <= PREC_ASSIGNMENT;
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
static void printStatement(){
	expression();
	consume(TOKEN_SEMICOLON,"Expect ';' after value.");
	emitByte(OP_PRINT);
}
static void emitGlobal(int global, bool set){
	if (global < 256){
		if (set)
			emitBytes(OP_GLOBAL_SET,global & 0xff);
		else 
			emitBytes(OP_GLOBAL_GET,global & 0xff);
	} else {
		if (set){
			emitByte(OP_GLOBAL_SET_LONG);
			emitByte(OP_POP);
		}
		else 
			emitByte(OP_GLOBAL_GET_LONG);
		emitByte(global & 0xff);
		emitByte((global >> 8) & 0xff);
		emitByte((global >> 16) & 0xff);
	}
}
static void emitLocal(int offset, bool set){
	if (set)
		emitBytes(OP_LOCAL_SET,offset & 0xff);
	else 
		emitBytes(OP_LOCAL_GET,offset & 0xff);
}
static bool idEqual(Token a, Token b){
	return a.length == b.length && memcmp(&a.start,&b.start,a.length) == 0;
}

static int resolveLocal(PCompiler compiler,Token name){
	for (int i = compiler->localCount-1; i >= 0; i--){
		// decremental for loop for local shadowing
		if (idEqual(name,compiler->locals[i].name))
			return i;
	}
	return -1;
}
static void varRead(){
	Token name = parser.previous;
	int idx = emitConstant(OBJ_VAL(copyString(name.start,name.length)));
	int stackOffset; bool set = false;
	if (match(TOKEN_EQUAL)){
		if (canAssign){
			expression();
			set = true;
		} else {
			error("Invalid assignment target.");
		}
	} 
	stackOffset = resolveLocal(current,name);
	if (stackOffset != -1){
		emitLocal(stackOffset, set);
		return;
	}
	
	emitGlobal(idx,set);	
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
  [TOKEN_IDENTIFIER]    = {varRead,     NULL,   PREC_NONE},
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
static void endCompiler(){
	emitByte(OP_RETURN);
}
static void expressionStatement(){
	expression();
	consume(TOKEN_SEMICOLON,"Expect ';' after expression.");
	emitByte(OP_POP);
}
static void beginScope(){
	current->scopeDepth++;
}
static void endScope(){
	int prevCount = current->localCount;
	while (current->localCount > 0 &&
		current->locals[current->localCount-1].depth == current->scopeDepth){
		current->localCount--;	
	}
	int delta = prevCount - current->localCount;
	if (delta)
		emitBytes(OP_POPN, delta & 0xff);
	current->scopeDepth--;
}
static void block(){
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
		declaration();
	}
	consume(TOKEN_RIGHT_BRACE,"Expect '}' after block.");
}
static void statement(){
	if(match(TOKEN_PRINT)){
		printStatement();
	} else if (match(TOKEN_LEFT_BRACE)){
		beginScope();
		block();
		endScope();
	} else {
		expressionStatement();
	}
}
static void synchronize(){
	parser.panicMode = false;
	while (parser.current.type != TOKEN_EOF){
		if (parser.previous.type == TOKEN_SEMICOLON)
			return;
		 switch (parser.current.type) {
			case TOKEN_CLASS:
			case TOKEN_FUN:
			case TOKEN_VAR:
			case TOKEN_FOR:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
				return;
			default: ;
		}
		advance();
	}
}
static void addLocal(Token name){
	if (current->localCount == LOCALS_MAX){
		error("Too many local variables, slow down!");
		return;
	}
	if (resolveLocal(current, name) != -1)
		error("Variables with the same name in the same scope.");
	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = current->scopeDepth;
}
static int parseVar(const char* errMsg){
	consume(TOKEN_IDENTIFIER,errMsg);
	Token name = parser.previous;
	if (current->scopeDepth > 0){
		return -1;
	}
	return emitConstant(OBJ_VAL(copyString(name.start,name.length)));
}
static void varDecl(){
	// in case of locals, we just let the value for the variable to be pushed to the stack 
	// we don't emit the identifier constant or the global get/set opcode
	int global = parseVar("Expect variable name.");
	if (match(TOKEN_EQUAL)){
		expression();
	} else {
		emitByte(OP_NIL);
	}
	if (current->scopeDepth > 0)
		return;
	consume(TOKEN_SEMICOLON,"Expect ';' after variable declaration.");
	bool set = true;
	emitGlobal(global, set);
	
}
static void declaration(){
	if (match(TOKEN_VAR))
		varDecl();
	else 
		statement();
	if (parser.panicMode)
		synchronize();
}
static void initCompiler(PCompiler compiler){
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	current = compiler;
	
}
bool compile(const char* source, PChunk chunk){
	Compiler compiler;
	initCompiler(&compiler);
	compilingChunk = chunk;
	initScanner(source);
	initParser();
	advance();
	while (!match(TOKEN_EOF)){
		declaration();
	}
	endCompiler();
	return !parser.hadError;
}