#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Lexer* lexer_create(const char* source, const char* file_name, DiagnosticBag* diagnostics) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source; // 注意：Lexer不拥有source内存，调用者需保证source有效
    lexer->file_name = file_name;
    lexer->position = 0;
    lexer->length = (int)strlen(source);
    lexer->line = 1;
    lexer->column = 1;
    lexer->diagnostics = diagnostics;
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    free(lexer);
}

static char peek(Lexer* l, int offset) {
    int index = l->position + offset;
    if (index >= l->length) return '\0';
    return l->source[index];
}

static char current(Lexer* l) {
    return peek(l, 0);
}

static void advance(Lexer* l) {
    if (l->position < l->length) {
        l->position++;
        l->column++;
    }
}

Token lexer_next_token(Lexer* l) {
    // 1. 跳过空白字符
    while (l->position < l->length) {
        char c = current(l);
        if (c == ' ' || c == '\t' || c == '\r') {
            advance(l);
        } else if (c == '\n') {
            l->position++;
            l->line++;
            l->column = 1;
        } else {
            break;
        }
    }

    Token token;
    token.line = l->line;
    token.column = l->column;
    token.text = l->source + l->position;
    token.length = 0;

    if (l->position >= l->length) {
        token.type = TOKEN_EOF;
        return token;
    }

    char c = current(l);

    // 2. 解析标识符和关键字
    if (isalpha(c) || c == '_') {
        int start = l->position;
        while (isalnum(current(l)) || current(l) == '_') {
            advance(l);
        }
        token.length = l->position - start;
        token.type = TOKEN_IDENTIFIER;
        
        // 简单的关键字检查 (后续可以用哈希表优化)
        // 注意：这里只是简单比较，实际需要更严谨的字符串比较
        // 由于 token.text 不是以 \0 结尾的，需要用 strncmp
        if (strncmp(token.text, "wce_if", token.length) == 0 && token.length == 6) token.type = TOKEN_KW_IF;
        else if (strncmp(token.text, "wce_else", token.length) == 0 && token.length == 8) token.type = TOKEN_KW_ELSE;
        else if (strncmp(token.text, "wce_for", token.length) == 0 && token.length == 7) token.type = TOKEN_KW_FOR;
        
        return token;
    }

    // 3. 解析数字
    if (isdigit(c)) {
        int start = l->position;
        while (isdigit(current(l))) {
            advance(l);
        }
        // 支持小数
        if (current(l) == '.' && isdigit(peek(l, 1))) {
            advance(l); // consume '.'
            while (isdigit(current(l))) {
                advance(l);
            }
        }
        token.length = l->position - start;
        token.type = TOKEN_NUMBER;
        return token;
    }

    // 4. 解析字符串
    if (c == '"') {
        advance(l); // skip start quote
        int start = l->position;
        while (current(l) != '"' && current(l) != '\0') {
            if (current(l) == '\n') {
                l->line++;
                l->column = 1;
            }
            advance(l);
        }
        
        if (current(l) == '"') {
            token.length = l->position - start; // 内容长度
            token.text = l->source + start;     // 指向内容起始
            token.type = TOKEN_STRING;
            advance(l); // skip end quote
            return token;
        } else {
            // 错误：字符串未闭合
            diagnostic_report(l->diagnostics, l->file_name, token.line, token.column, DIAG_ERROR, 2, "Unterminated string literal");
            token.type = TOKEN_UNKNOWN;
            return token;
        }
    }

    // 5. 解析符号
    int start = l->position;
    advance(l); // Consume first char
    token.length = 1;

    switch (c) {
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case ',': token.type = TOKEN_COMMA; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        
        case '=':
            if (current(l) == '=') {
                advance(l);
                token.type = TOKEN_EQ;
                token.length = 2;
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '!':
            if (current(l) == '=') {
                advance(l);
                token.type = TOKEN_NEQ;
                token.length = 2;
            } else {
                token.type = TOKEN_UNKNOWN;
                diagnostic_report(l->diagnostics, l->file_name, token.line, token.column - 1, DIAG_ERROR, 1, "Unexpected character '%c'", c);
            }
            break;
        case '<':
            if (current(l) == '=') {
                advance(l);
                token.type = TOKEN_LE;
                token.length = 2;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (current(l) == '=') {
                advance(l);
                token.type = TOKEN_GE;
                token.length = 2;
            } else {
                token.type = TOKEN_GT;
            }
            break;
            
        default:
            token.type = TOKEN_UNKNOWN;
            diagnostic_report(l->diagnostics, l->file_name, token.line, token.column - 1, DIAG_ERROR, 1, "Unexpected character '%c'", c);
            break;
    }

    return token;
}
