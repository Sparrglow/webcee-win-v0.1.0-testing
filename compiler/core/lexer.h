#ifndef WEBCEE_LEXER_H
#define WEBCEE_LEXER_H

#include "token.h"
#include "diagnostic.h"

typedef struct {
    const char* source;      // 源代码字符串
    const char* file_name;   // 文件名
    int position;            // 当前位置（字符索引）
    int length;              // 源码总长度
    int line;                // 当前行
    int column;              // 当前列
    DiagnosticBag* diagnostics; // 关联的诊断包
} Lexer;

// 创建Lexer
Lexer* lexer_create(const char* source, const char* file_name, DiagnosticBag* diagnostics);
void lexer_destroy(Lexer* lexer);

// 获取下一个Token
Token lexer_next_token(Lexer* lexer);

#endif // WEBCEE_LEXER_H
