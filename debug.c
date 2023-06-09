#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constantInstruction(const char* name, int offset, Chunk* chunk) {
    int index = chunk->code[offset + 1];
    Value constant = chunk->constants.values[index];
    printf("%-16s %4d '", name, index);
    printValue(constant);
    printf("'\n");
    return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset-1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_RETURN: return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT: return constantInstruction("OP_CONSTANT", offset, chunk);

        case OP_NEGATE: return simpleInstruction("OP_NEGATE", offset);
        case OP_ADD: return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT: return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY: return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE: return simpleInstruction("OP_DIVIDE", offset);

        case OP_TRUE: return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE: return simpleInstruction("OP_FALSE", offset);
        case OP_NIL: return simpleInstruction("OP_NIL", offset);

        case OP_NOT: return simpleInstruction("OP_NOT", offset);
        case OP_AND: return simpleInstruction("OP_AND", offset);
        case OP_OR: return simpleInstruction("OP_OR", offset);

        case OP_EQUAL: return simpleInstruction("OP_EQUAL", offset);
        case OP_LESS: return simpleInstruction("OP_LESS", offset);
        case OP_GREATER: return simpleInstruction("OP_GREATER", offset);

        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

void printValueStack(Value* stack, Value* stackTop) {
    printf("Value Stack: [ ");
    for (Value* cur = stack; cur < stackTop; cur++) {
        printValue(*cur);
    }
    printf("]\n");
}
