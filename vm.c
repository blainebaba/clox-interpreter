#include <stdio.h>
#include <stdarg.h>
#include "vm.h"
#include "value.h"
#include "common.h"
#include "debug.h"

// global variable
VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

// peek value stack
static Value peek(int offset) {
    return vm.stackTop[-1-offset];
}

static void push(Value value) {
    *vm.stackTop++ = value;
}

static Value pop() {
    return *--vm.stackTop;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code -1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

// what is considered false.
// nil and false are falsey, anything else is true.
static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static bool toBool(Value value) {
    return !isFalsey(value);
}

static InterpretResult run() {
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONST() (vm.chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(type, op) do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be number."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(type(a op b)); \
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

            case OP_NEGATE: 
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop()))); 
                break;
            case OP_ADD: BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;

            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_NIL: push(NIL_VAL()); break;

            case OP_NOT: push(BOOL_VAL(isFalsey(pop()))); break;
            // TODO
            case OP_AND: break;
            case OP_OR: break;

            case OP_EQUAL:
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valueEqual(a, b)));
                break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
        }
    }

    #undef READ_BYTE
    #undef READ_CONST
    #undef BINARY_OP
}

void initVM() {
    resetStack();
}

void freeVM() {
}

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}