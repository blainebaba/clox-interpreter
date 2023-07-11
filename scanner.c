#include <string.h>
#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}
static Token makeErrorToken(const char* msg) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = (int)strlen(msg);
    token.line = scanner.line;
    return token;
}

static bool isAtEnd() {
    // dot has higher precedence than star
    return *scanner.current == '\0';
}

// move current pointer forward
static char advance() {
    char ch = *scanner.current;
    scanner.current++;
    return ch;
}

// read char at current pointer
static char peek() {
    return *scanner.current;
}

// peek char next to current pointer
static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

// if match, consume and return true
static bool match(char c) {
    if (isAtEnd()) return false;

    if (*scanner.current == c) {
        scanner.current ++;
        return true;
    } else {
        return false;
    }
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch(c) {
            case '\n':
                scanner.line ++;
            case ' ':
            case '\t':
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                    break;
                }
            default:
                return;
        }
    }
}

// scan string
static Token string() {
    while (!isAtEnd()) {
        char c = advance();
        if (c == '"') {
            return makeToken(TOKEN_STRING);
        } else if (c == '\n') {
            return makeErrorToken("Unfinished string.");
        }
    }
    return makeErrorToken("Unfinished string.");
}

// scan number
static Token number() {
    while (isDigit(peek())) advance();
    if (match('.')) {
        while (isDigit(peek())) advance();
    }
    return makeToken(TOKEN_NUMBER);
}

// scan identifier
static Token identifier() {
    return makeErrorToken("NOT IMPLEMENTED");
}

Token scanToken() {
    // this function must return a valid token, hence all meaningless tokens must be skipped
    // at the start.
    skipWhitespace();

    scanner.start = scanner.current;

    if(isAtEnd()) return makeToken(TOKEN_EOF);

    char ch = advance();

    if (isDigit(ch)) return number();
    if (isAlpha(ch)) return identifier();
    

    switch(ch) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case '*': return makeToken(TOKEN_STAR);
        case '/': return makeToken(TOKEN_SLASH);
        case '!': return match('=') ? makeToken(TOKEN_BANG_EQUAL) : makeToken(TOKEN_BANG);
        case '>': return match('=') ? makeToken(TOKEN_GREATER_EQUAL) : makeToken(TOKEN_GREATER);
        case '<': return match('=') ? makeToken(TOKEN_LESS_EQUAL) : makeToken(TOKEN_LESS);
        case '=': return match('=') ? makeToken(TOKEN_EQUAL_EQUAL) : makeToken(TOKEN_EQUAL);
        case '"': return string();
    }

    return makeErrorToken("Unexpected character.");
}