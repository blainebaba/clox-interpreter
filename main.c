#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {

    Chunk chunk;

    initChunk(&chunk);
    initVM();

    int index = addConstant(&chunk, 999);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, index, 123);

    writeChunk(&chunk, OP_RETURN, 123);

    interpret(&chunk);

    freeVM();
    freeChunk(&chunk);

    return 0;
}
