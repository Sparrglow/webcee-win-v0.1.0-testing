#include "ast.h"
#include <stdlib.h>
#include <string.h>

WceAstNode* ast_node_create(MemoryPool* pool, AstNodeType type, Token token) {
    if (!pool) return NULL;
    WceAstNode* node = (WceAstNode*)memory_pool_alloc(pool, sizeof(WceAstNode));
    if (node) {
        memset(node, 0, sizeof(WceAstNode));
        node->type = type;
        node->token = token;
    }
    return node;
}

void ast_node_add_child(WceAstNode* parent, WceAstNode* child) {
    if (!parent || !child) return;
    
    if (!parent->first_child) {
        parent->first_child = child;
    } else {
        WceAstNode* current = parent->first_child;
        while (current->next_sibling) {
            current = current->next_sibling;
        }
        current->next_sibling = child;
    }
}

// ast_node_destroy is removed as memory is managed by MemoryPool

