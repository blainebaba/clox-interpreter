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
    #define BINARY_OP(op) do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while(false)


    for (;;) {
        #ifdef DEBUG_PRINT_VALUE_STACK
            printValueStack(vm.stack, vm.stackTop);
        #endif

        #ifdef DEBUG_DISASSEMBLE_CHUNK
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction = READ_BYTE();

        switch(instruction) {
            case OP_RETURN:
                printValue(pop());
                printf("\n");
                return INTERPRET_SUCCESS;
            // arithmetic
            case OP_CONSTANT: push(READ_CONST()); break;
            case OP_NEGATE: push(-pop()); break;
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
        }
    }

    #undef READ_BYTE
    #undef READ_CONST
    #undef BINARY_OP
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