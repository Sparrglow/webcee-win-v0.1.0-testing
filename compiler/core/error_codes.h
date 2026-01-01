#ifndef WEBCEE_ERROR_CODES_H
#define WEBCEE_ERROR_CODES_H

// Lexer Errors (001-099)
#define ERR_LEX_UNKNOWN_CHAR       1
#define ERR_LEX_UNTERMINATED_STR   2

// Parser Errors (100-199)
#define ERR_PARSER_EXPECTED_RBRACE 101 // 缺少右大括号 `}`
#define ERR_PARSER_EXPECTED_RPAREN 102 // 缺少右括号 `)`
#define ERR_PARSER_EXPECTED_LPAREN 103 // 缺少左括号 `(`
#define ERR_PARSER_EXPECTED_ID     104 // 期望标识符
#define ERR_PARSER_EXPECTED_SEMI   105 // 期望分号
#define ERR_PARSER_UNEXPECTED_TOK  106 // 意外的 Token
#define ERR_PARSER_EXPECTED_LBRACE 107 // 缺少左大括号 `{`

// Semantic Errors (200-299)
#define ERR_SEMANTIC_UNDEFINED     201 // 未定义的组件或函数
#define ERR_SEMANTIC_ARG_COUNT     202 // 参数数量不匹配
#define ERR_SEMANTIC_TYPE_MISMATCH 203 // 参数类型不匹配

#endif // WEBCEE_ERROR_CODES_H
