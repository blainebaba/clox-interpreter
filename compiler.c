#include <stdio.h>
#include "compiler.h"
#include "common.h"
#include "scanner.h"

void compile(const char* source) {
    initScanner(source);

    // TEMP
    int line = -1;
    for (;;) {
        Token token = scanToken();

        if (token.type == TOKEN_EOF) {
            break;
        }

        if (token.line == line) {
            printf("   | ");
        } else {
            printf("%4d ", token.line);
            line = token.line;
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);
    }
}

