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

static TokenType checkKeyword(const char* keyword, int length, TokenType type) {
    if (scanner.current - scanner.start == length &&
        memcmp(scanner.start, keyword, length) == 0) {
        return type;
    } else {
        return TOKEN_IDENTIFIER;
    }
}

static TokenType identifierType() {
    // match keywords
    switch(scanner.start[0]) {
        case 'a': return checkKeyword("and", 3, TOKEN_AND);
        case 'c': return checkKeyword("class", 5, TOKEN_CLASS);
        case 'e': return checkKeyword("else", 4, TOKEN_ELSE);
        case 'f': 
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return checkKeyword("false", 5, TOKEN_FALSE);
                    case 'o': return checkKeyword("for", 3, TOKEN_FOR);
                    case 'u': return checkKeyword("fun", 3, TOKEN_FUN);
                }
            }
            break;
        case 'i': return checkKeyword("if", 2, TOKEN_IF);
        case 'n': return checkKeyword("nil", 3, TOKEN_NIL);
        case 'o': return checkKeyword("or", 2, TOKEN_OR);
        case 'p': return checkKeyword("print", 5, TOKEN_PRINT);
        case 'r': return checkKeyword("return", 5, TOKEN_RETURN);
        case 's': return checkKeyword("super", 5, TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword("this", 4, TOKEN_THIS);
                    case 'r': return checkKeyword("true", 4, TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword("var", 3, TOKEN_VAR);
        case 'w': return checkKeyword("while", 5, TOKEN_WHILE);
    }

    // if no keywords matched, it is an identifier
    return TOKEN_IDENTIFIER;
}

// scan identifier
static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
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