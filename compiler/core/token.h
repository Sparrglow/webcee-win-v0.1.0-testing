#ifndef WEBCEE_TOKEN_H
#define WEBCEE_TOKEN_H

typedef enum {
    TOKEN_EOF,
    TOKEN_UNKNOWN,
    
    // 字面量
    TOKEN_IDENTIFIER,   // wce_button, my_var
    TOKEN_STRING,       // "hello"
    TOKEN_NUMBER,       // 123, 3.14
    
    // 符号
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_COMMA,        // ,
    TOKEN_SEMICOLON,    // ;
    
    // 运算符
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_ASSIGN,       // =
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_LE,           // <=
    TOKEN_GE,           // >=
    
    // 关键字 (后续扩展)
    TOKEN_KW_IF,        // wce_if
    TOKEN_KW_ELSE,      // wce_else
    TOKEN_KW_FOR        // wce_for
} TokenType;

typedef struct {
    TokenType type;
    const char* text;   // 指向源码中的起始位置（不拥有内存）
    int length;         // 长度
    int line;
    int column;
} Token;

#endif // WEBCEE_TOKEN_H
