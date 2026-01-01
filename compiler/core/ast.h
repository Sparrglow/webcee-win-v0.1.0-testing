#ifndef WEBCEE_AST_H
#define WEBCEE_AST_H

#include "token.h"
#include "memory_pool.h"

typedef enum {
    NODE_ROOT,
    NODE_FUNCTION_CALL,
    NODE_STRING_LITERAL,
    NODE_NUMBER_LITERAL,
    NODE_IDENTIFIER,
    NODE_BLOCK,
    NODE_IF,
    NODE_FOR
} AstNodeType;

typedef struct WceAstNode WceAstNode;

struct WceAstNode {
    AstNodeType type;
    Token token; // 关联的Token，用于位置信息
    
    // 简单的多叉树结构
    WceAstNode* first_child;
    WceAstNode* next_sibling;
    
    // 特定节点数据 (可以使用 union 优化，这里先简化)
    char* string_value;
    double number_value;
    
    // For function calls with a trailing block (e.g. wce_row() { ... })
    WceAstNode* block; 
    
    // Control flow specific (could be children, but explicit fields are clearer for some)
    // For simplicity, we'll use children:
    // IF: child[0]=cond, child[1]=then, child[2]=else (optional)
    // FOR: child[0]=init, child[1]=cond, child[2]=inc, child[3]=body
};

// 创建节点
WceAstNode* ast_node_create(MemoryPool* pool, AstNodeType type, Token token);
void ast_node_add_child(WceAstNode* parent, WceAstNode* child);
// void ast_node_destroy(WceAstNode* node); // Managed by MemoryPool

#endif // WEBCEE_AST_H
