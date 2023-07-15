#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "common.h"
#include "debug.h"

#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    // if error ever occurred in compiler
    bool hadError;
    // when panic mode is on, skip tokens until next recovery point is reached
    bool panicMode;
} Parser;

// see precedence summarized in jlox: https://github.com/blainebaba/jlox-interpreter/blob/mainline/language-spec.md
typedef enum {
   PREC_NONE,
   PREC_ASSIGNMENT,
   PREC_OR,
   PREC_AND,
   PREC_EQUAL,
   PREC_COMP,
   PREC_ADD_TERM,
   PREC_MUL_TERM,
   PREC_UNARY,
   PREC_CALL,
   PREC_GET,
   PREC_PRIMARY,
} Precedence;

// function pointer type
typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk* compilingChunk;

////////////////////
// Read token
///////////////////

// report error on token
static void errorAt(const char* msg, Token* token) {
    if (parser.panicMode) {
        // ignore errors in panic mode
        return;
    }

    // print error msg
    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", msg);
    

    parser.hadError = true;
    // when found an error, enter panic mode, skip following tokens until reaches recovery point.
    parser.panicMode = true;
}

static void errorAtCurrent(const char* msg) {
    errorAt(msg, &parser.current);
}

static void error(const char* msg) {
    errorAt(msg, &parser.previous);
}

// read one valid token
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type == TOKEN_ERROR) {
            // the token points to error message
            errorAtCurrent(parser.current.start);
        } else {
            // valid token, stop loopping
            break;
        }
    }
}

// match a token type and advance, otherwise fail
static void consume(TokenType type, const char* msg) {
    if (parser.current.type == type) {
        advance();
    } else {
        errorAtCurrent(msg);
    }
}

////////////////////
// Emit bytecode
///////////////////

static Chunk* currentChunk() {
    return compilingChunk;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

// constant stored in an array in chunk, return index of constant
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
    #ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
    #endif
}

////////////////////
// Compiler
///////////////////

static ParseRule* getRule(TokenType type);

// parse expression at given precedence or higher
static void parsePrecedence(Precedence precedence) {
    advance();

    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }
    prefixRule();

    // execute rules with precedence same or higher than we specified.
    // while loop handles a chains of operators, e.g. 1+1+1+1
    // this algorithm is called Pratt Parsing.
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static void expression() {
    // parse everything
    parsePrecedence(PREC_ASSIGNMENT);
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// assumption for all compiling function is that the initial token is already
// consumed.
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// operator emits after operand, we use stack-based bytecode.
static void unary() {
    TokenType operator = parser.previous.type;
    // operand
    // also use unary here to support nested unary operator.
    parsePrecedence(PREC_UNARY);

    switch(operator) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        case TOKEN_BANG: emitByte(OP_NOT); break;
        default: return;
    }
}

static void binary() {
    TokenType operator = parser.previous.type;
    ParseRule* rule = getRule(operator);
    parsePrecedence((Precedence)(rule->precedence+1));
    switch(operator) {
        case TOKEN_PLUS: emitByte(OP_ADD); break;
        case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH: emitByte(OP_DIVIDE); break;

        case TOKEN_AND: emitByte(OP_AND); break;
        case TOKEN_OR: emitByte(OP_OR); break;

        case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
        case TOKEN_BANG_EQUAL: emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_LESS: emitByte(OP_LESS); break;
        case TOKEN_GREATER: emitByte(OP_GREATER); break;
        // convert less equal to not greater
        case TOKEN_LESS_EQUAL: emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        default: return;
    }
}

static void literal() {
    TokenType type = parser.previous.type;
    switch(type) {
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL: emitByte(OP_NIL); break;
        default: return;
    }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] =    {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] =   {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] =    {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] =   {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] =         {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] =           {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS] =         {unary, binary, PREC_ADD_TERM},
    [TOKEN_PLUS] =          {NULL, binary, PREC_ADD_TERM},
    [TOKEN_SEMICOLON] =     {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = 		{NULL, binary, PREC_MUL_TERM},
    [TOKEN_STAR] = 			{NULL, binary, PREC_MUL_TERM},
    [TOKEN_BANG] = 		    {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = 	{NULL, binary, PREC_EQUAL},
    [TOKEN_EQUAL] = 	    {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = 	{NULL, binary, PREC_EQUAL},
    [TOKEN_GREATER] = 	    {NULL, binary, PREC_COMP},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMP},
    [TOKEN_LESS] = 	        {NULL, binary, PREC_COMP},
    [TOKEN_LESS_EQUAL] = 	{NULL, binary, PREC_COMP},
    [TOKEN_IDENTIFIER] = 	{NULL, NULL, PREC_NONE},
    [TOKEN_STRING] = 	    {NULL, NULL, PREC_NONE},
    [TOKEN_NUMBER] = 	    {number, NULL, PREC_NONE},
    [TOKEN_AND] = 		    {NULL, binary, PREC_AND},
    [TOKEN_CLASS] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = 		    {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = 		{literal, NULL, PREC_NONE},
    [TOKEN_FOR] = 			{NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = 			{NULL, NULL, PREC_NONE},
    [TOKEN_IF] = 			{NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = 			{literal, NULL, PREC_NONE},
    [TOKEN_OR] = 			{NULL, binary, PREC_OR},
    [TOKEN_PRINT] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = 			{NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = 			{literal, NULL, PREC_NONE},
    [TOKEN_VAR] = 			{NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = 		{NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = 			{NULL, NULL, PREC_NONE},
};

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

////////////////////
// Public methods
///////////////////

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    // init parser
    parser.hadError = false;
    parser.panicMode = false;

    // prime compiler
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}

