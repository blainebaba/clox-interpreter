#include <stdio.h>
#include "vm.h"
#include "value.h"
#include "common.h"
#include "debug.h"

// global variable
VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {
}

static InterpretResult run() {
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONST() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
        #ifdef DEBUG_PRINT_VALUE_STACK
            printValueStack(vm.stack, vm.stackTop);
        #endif

        #ifdef DEBUG_DISASSEMBLE_CHUNK
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction = READ_BYTE();

        switch(instruction) {
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_SUCCESS;
            }
            case OP_CONSTANT: {
                Value constant = READ_CONST();
                push(constant);
                break;
            }
        }
    }

    #undef READ_BYTE
}

InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = chunk->code;
    return run();
}

void push(Value value) {
    *vm.stackTop++ = value;
}

Value pop() {
    return *--vm.stackTop;
}