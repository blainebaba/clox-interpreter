#include <stdio.h>
#include "compiler.h"
#include "common.h"

#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    // if error ever occurred in compiler
    bool hadError;
    // when panic mode is on, skip tokens until next recovery point is reached
    bool panicMode;
} Parser;

Parser parser;
Chunk* compilingChunk;

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

static void errorAtPrevious(const char* msg) {
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
static void match(TokenType type, const char* msg) {
    if (parser.current.type == type) {
        advance();
    } else {
        errorAtCurrent(msg);
    }
}

static Chunk* currentChunk() {
    return compilingChunk;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    // init parser
    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    match(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}

