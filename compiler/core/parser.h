#ifndef WEBCEE_PARSER_H
#define WEBCEE_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "diagnostic.h"
#include "memory_pool.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
    DiagnosticBag* diagnostics;
    MemoryPool* pool;
    int panic_mode; // 错误恢复模式标志
} Parser;

Parser* parser_create(Lexer* lexer, DiagnosticBag* diagnostics, MemoryPool* pool);
void parser_destroy(Parser* parser);

// 解析入口
WceAstNode* parser_parse(Parser* parser);

#endif // WEBCEE_PARSER_H
