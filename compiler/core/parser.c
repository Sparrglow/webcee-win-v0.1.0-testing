#include "parser.h"
#include "error_codes.h"
#include <stdlib.h>
#include <string.h>

// --- Forward Declarations ---
static void advance(Parser* p);
static int match(Parser* p, TokenType type);
static WceAstNode* parse_statement(Parser* p);
static WceAstNode* parse_expression(Parser* p);
static WceAstNode* parse_assignment(Parser* p);
static WceAstNode* parse_equality(Parser* p);
static WceAstNode* parse_comparison(Parser* p);
static WceAstNode* parse_term(Parser* p);
static WceAstNode* parse_factor(Parser* p);
static WceAstNode* parse_primary(Parser* p);
static WceAstNode* parse_block(Parser* p);
static WceAstNode* parse_if_statement(Parser* p);
static WceAstNode* parse_for_statement(Parser* p);
static WceAstNode* parse_term(Parser* p);
static WceAstNode* parse_factor(Parser* p);
static WceAstNode* parse_primary(Parser* p);
static WceAstNode* parse_block(Parser* p);
static WceAstNode* parse_if_statement(Parser* p);
static WceAstNode* parse_for_statement(Parser* p);

Parser* parser_create(Lexer* lexer, DiagnosticBag* diagnostics, MemoryPool* pool) {
    Parser* p = (Parser*)malloc(sizeof(Parser));
    p->lexer = lexer;
    p->diagnostics = diagnostics;
    p->pool = pool;
    p->panic_mode = 0;
    advance(p); // Load first token
    return p;
}

void parser_destroy(Parser* p) {
    free(p);
}

static void advance(Parser* p) {
    p->current_token = lexer_next_token(p->lexer);
    while (p->current_token.type == TOKEN_UNKNOWN) {
        p->current_token = lexer_next_token(p->lexer);
    }
}

static int match(Parser* p, TokenType type) {
    if (p->current_token.type == type) {
        advance(p);
        return 1;
    }
    return 0;
}

static void synchronize(Parser* p) {
    p->panic_mode = 0;
    while (p->current_token.type != TOKEN_EOF) {
        if (p->current_token.type == TOKEN_SEMICOLON) {
            advance(p);
            return;
        }
        if (p->current_token.type == TOKEN_RBRACE) return;
        advance(p);
    }
}

static void consume(Parser* p, TokenType type, int error_code, const char* msg) {
    if (p->current_token.type == type) {
        advance(p);
        return;
    }
    
    if (!p->panic_mode) {
        diagnostic_report(p->diagnostics, p->lexer->file_name, 
            p->current_token.line, p->current_token.column, 
            DIAG_ERROR, error_code, msg);
        p->panic_mode = 1;
    }
}

// --- Parsing Logic ---

WceAstNode* parser_parse(Parser* p) {
    WceAstNode* root = ast_node_create(p->pool, NODE_ROOT, (Token){0});
    
    while (p->current_token.type != TOKEN_EOF) {
        WceAstNode* stmt = parse_statement(p);
        if (stmt) {
            ast_node_add_child(root, stmt);
        } else {
            if (p->panic_mode) synchronize(p);
            else advance(p);
        }
    }
    return root;
}

static WceAstNode* parse_statement(Parser* p) {
    if (p->current_token.type == TOKEN_LBRACE) {
        return parse_block(p);
    }
    if (p->current_token.type == TOKEN_KW_IF) {
        return parse_if_statement(p);
    }
    if (p->current_token.type == TOKEN_KW_FOR) {
        return parse_for_statement(p);
    }
    
    // Expression statement (function call, assignment, etc.)
    WceAstNode* expr = parse_expression(p);
    if (expr) {
        // If it's a function call followed by a block, it's a container statement
        if (expr->type == NODE_FUNCTION_CALL && p->current_token.type == TOKEN_LBRACE) {
            expr->block = parse_block(p);
            // No semicolon required for block statements usually, but let's see.
            // If user writes wce_row() { ... } we don't expect ;
            // If user writes wce_text("hi"); we expect ;
            // So if block is present, consume optional semicolon?
            // Standard C: if (...) { ... } no semi.
            // Let's assume no semi needed if block exists.
            return expr;
        }
        
        consume(p, TOKEN_SEMICOLON, ERR_PARSER_EXPECTED_SEMI, "Expected ';' after statement");
        return expr;
    }
    
    if (!p->panic_mode) {
        diagnostic_report(p->diagnostics, p->lexer->file_name, 
            p->current_token.line, p->current_token.column, 
            DIAG_ERROR, ERR_PARSER_UNEXPECTED_TOK, "Unexpected token");
        p->panic_mode = 1;
    }
    return NULL;
}

static WceAstNode* parse_block(Parser* p) {
    WceAstNode* node = ast_node_create(p->pool, NODE_BLOCK, p->current_token);
    consume(p, TOKEN_LBRACE, ERR_PARSER_EXPECTED_LBRACE, "Expected '{'");
    
    while (p->current_token.type != TOKEN_RBRACE && p->current_token.type != TOKEN_EOF) {
        WceAstNode* stmt = parse_statement(p);
        if (stmt) {
            ast_node_add_child(node, stmt);
        } else {
            if (p->panic_mode) synchronize(p);
            else advance(p);
        }
    }
    
    consume(p, TOKEN_RBRACE, ERR_PARSER_EXPECTED_RBRACE, "Expected '}'");
    return node;
}

static WceAstNode* parse_if_statement(Parser* p) {
    WceAstNode* node = ast_node_create(p->pool, NODE_IF, p->current_token);
    advance(p); // consume 'if'
    
    consume(p, TOKEN_LPAREN, ERR_PARSER_EXPECTED_LPAREN, "Expected '('");
    WceAstNode* cond = parse_expression(p);
    if (cond) ast_node_add_child(node, cond);
    consume(p, TOKEN_RPAREN, ERR_PARSER_EXPECTED_RPAREN, "Expected ')'");
    
    WceAstNode* then_branch = parse_statement(p);
    if (then_branch) ast_node_add_child(node, then_branch);
    
    if (match(p, TOKEN_KW_ELSE)) {
        WceAstNode* else_branch = parse_statement(p);
        if (else_branch) ast_node_add_child(node, else_branch);
    }
    
    return node;
}

static WceAstNode* parse_for_statement(Parser* p) {
    WceAstNode* node = ast_node_create(p->pool, NODE_FOR, p->current_token);
    advance(p); // consume 'for'
    
    consume(p, TOKEN_LPAREN, ERR_PARSER_EXPECTED_LPAREN, "Expected '('");
    
    // Init
    if (p->current_token.type != TOKEN_SEMICOLON) {
        WceAstNode* init = parse_expression(p);
        if (init) ast_node_add_child(node, init);
    }
    consume(p, TOKEN_SEMICOLON, ERR_PARSER_EXPECTED_SEMI, "Expected ';'");
    
    // Condition
    if (p->current_token.type != TOKEN_SEMICOLON) {
        WceAstNode* cond = parse_expression(p);
        if (cond) ast_node_add_child(node, cond);
    }
    consume(p, TOKEN_SEMICOLON, ERR_PARSER_EXPECTED_SEMI, "Expected ';'");
    
    // Increment
    if (p->current_token.type != TOKEN_RPAREN) {
        WceAstNode* inc = parse_expression(p);
        if (inc) ast_node_add_child(node, inc);
    }
    consume(p, TOKEN_RPAREN, ERR_PARSER_EXPECTED_RPAREN, "Expected ')'");
    
    WceAstNode* body = parse_statement(p);
    if (body) ast_node_add_child(node, body);
    
    return node;
}

static WceAstNode* parse_expression(Parser* p) {
    return parse_assignment(p);
}

static WceAstNode* parse_assignment(Parser* p) {
    WceAstNode* expr = parse_equality(p);
    
    if (p->current_token.type == TOKEN_ASSIGN) {
        Token op = p->current_token;
        advance(p);
        WceAstNode* value = parse_assignment(p);
        
        if (expr->type == NODE_IDENTIFIER) {
            WceAstNode* node = ast_node_create(p->pool, NODE_IDENTIFIER, op);
            node->string_value = (char*)memory_pool_alloc(p->pool, 2);
            if (node->string_value) {
                node->string_value[0] = '=';
                node->string_value[1] = '\0';
            }
            ast_node_add_child(node, expr);
            ast_node_add_child(node, value);
            return node;
        }
        
        diagnostic_report(p->diagnostics, p->lexer->file_name, op.line, op.column, DIAG_ERROR, 300, "Invalid assignment target");
    }
    
    return expr;
}

static WceAstNode* parse_equality(Parser* p) {
    WceAstNode* expr = parse_comparison(p);
    while (p->current_token.type == TOKEN_EQ || p->current_token.type == TOKEN_NEQ) {
        Token op = p->current_token;
        advance(p);
        WceAstNode* right = parse_comparison(p);
        WceAstNode* node = ast_node_create(p->pool, NODE_FUNCTION_CALL, op); // Use CALL for op for now? Or new type?
        // Let's use IDENTIFIER for op for now, or add NODE_BINARY_OP
        // For simplicity, I'll use NODE_IDENTIFIER with the op text
        node->string_value = (char*)memory_pool_alloc(p->pool, op.length + 1);
        if (node->string_value) {
            strncpy(node->string_value, op.text, op.length);
            node->string_value[op.length] = '\0';
        }
        ast_node_add_child(node, expr);
        ast_node_add_child(node, right);
        expr = node;
    }
    return expr;
}

static WceAstNode* parse_comparison(Parser* p) {
    WceAstNode* expr = parse_term(p);
    while (p->current_token.type == TOKEN_LT || p->current_token.type == TOKEN_LE ||
           p->current_token.type == TOKEN_GT || p->current_token.type == TOKEN_GE) {
        Token op = p->current_token;
        advance(p);
        WceAstNode* right = parse_term(p);
        WceAstNode* node = ast_node_create(p->pool, NODE_IDENTIFIER, op); // Reuse ID for op
        node->string_value = (char*)memory_pool_alloc(p->pool, op.length + 1);
        if (node->string_value) {
            strncpy(node->string_value, op.text, op.length);
            node->string_value[op.length] = '\0';
        }
        ast_node_add_child(node, expr);
        ast_node_add_child(node, right);
        expr = node;
    }
    return expr;
}

static WceAstNode* parse_term(Parser* p) {
    WceAstNode* expr = parse_factor(p);
    while (p->current_token.type == TOKEN_PLUS || p->current_token.type == TOKEN_MINUS) {
        Token op = p->current_token;
        advance(p);
        WceAstNode* right = parse_factor(p);
        WceAstNode* node = ast_node_create(p->pool, NODE_IDENTIFIER, op);
        node->string_value = (char*)memory_pool_alloc(p->pool, op.length + 1);
        if (node->string_value) {
            strncpy(node->string_value, op.text, op.length);
            node->string_value[op.length] = '\0';
        }
        ast_node_add_child(node, expr);
        ast_node_add_child(node, right);
        expr = node;
    }
    return expr;
}

static WceAstNode* parse_factor(Parser* p) {
    WceAstNode* expr = parse_primary(p);
    while (p->current_token.type == TOKEN_STAR || p->current_token.type == TOKEN_SLASH) {
        Token op = p->current_token;
        advance(p);
        WceAstNode* right = parse_primary(p);
        WceAstNode* node = ast_node_create(p->pool, NODE_IDENTIFIER, op);
        node->string_value = (char*)memory_pool_alloc(p->pool, op.length + 1);
        if (node->string_value) {
            strncpy(node->string_value, op.text, op.length);
            node->string_value[op.length] = '\0';
        }
        ast_node_add_child(node, expr);
        ast_node_add_child(node, right);
        expr = node;
    }
    return expr;
}

static WceAstNode* parse_primary(Parser* p) {
    Token t = p->current_token;
    WceAstNode* node = NULL;
    
    if (t.type == TOKEN_STRING) {
        node = ast_node_create(p->pool, NODE_STRING_LITERAL, t);
        node->string_value = (char*)memory_pool_alloc(p->pool, t.length + 1);
        if (node->string_value) {
            strncpy(node->string_value, t.text, t.length);
            node->string_value[t.length] = '\0';
        }
        advance(p);
    } else if (t.type == TOKEN_NUMBER) {
        node = ast_node_create(p->pool, NODE_NUMBER_LITERAL, t);
        char buf[64];
        int len = t.length < 63 ? t.length : 63;
        strncpy(buf, t.text, len);
        buf[len] = '\0';
        node->number_value = atof(buf);
        advance(p);
    } else if (t.type == TOKEN_IDENTIFIER) {
        Token name = t;
        advance(p);
        
        if (p->current_token.type == TOKEN_LPAREN) {
            // Function call
            node = ast_node_create(p->pool, NODE_FUNCTION_CALL, name);
            node->string_value = (char*)memory_pool_alloc(p->pool, name.length + 1);
            if (node->string_value) {
                strncpy(node->string_value, name.text, name.length);
                node->string_value[name.length] = '\0';
            }
            
            consume(p, TOKEN_LPAREN, ERR_PARSER_EXPECTED_LPAREN, "Expected '('");
            if (p->current_token.type != TOKEN_RPAREN) {
                do {
                    WceAstNode* arg = parse_expression(p);
                    if (arg) ast_node_add_child(node, arg);
                    else break;
                } while (match(p, TOKEN_COMMA));
            }
            consume(p, TOKEN_RPAREN, ERR_PARSER_EXPECTED_RPAREN, "Expected ')'");
        } else {
            // Variable
            node = ast_node_create(p->pool, NODE_IDENTIFIER, name);
            node->string_value = (char*)memory_pool_alloc(p->pool, name.length + 1);
            if (node->string_value) {
                strncpy(node->string_value, name.text, name.length);
                node->string_value[name.length] = '\0';
            }
        }
    } else if (t.type == TOKEN_LPAREN) {
        advance(p);
        node = parse_expression(p);
        consume(p, TOKEN_RPAREN, ERR_PARSER_EXPECTED_RPAREN, "Expected ')'");
    } else {
        // Error handled by caller or here?
        // If we return NULL, caller might handle it.
    }
    return node;
}


