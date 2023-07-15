#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// operation code
typedef enum {
    OP_RETURN,
    OP_CONSTANT,

    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    OP_TRUE,
    OP_FALSE,
    OP_NIL,

    OP_NOT,
    OP_AND,
    OP_OR,

    // greater equal is translated to not less, same as less equal.
    OP_EQUAL,
    OP_LESS,
    OP_GREATER,
} OpCode;

// byte code chunk
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif