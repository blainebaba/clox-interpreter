#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

// #define DEBUG_DISASSEMBLE_CHUNK
// #define DEBUG_PRINT_VALUE_STACK
#define DEBUG_PRINT_CODE

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
void printValueStack(Value* stack, Value* stackTop);

#endif