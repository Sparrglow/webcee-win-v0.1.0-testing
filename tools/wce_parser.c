#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define strdup _strdup
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

// --- AST Definitions (New Syntax Support) ---

typedef enum {
    WCE_AST_ROOT,
    WCE_AST_CALL,       // wce_text("...")
    WCE_AST_CONTAINER,  // wce_row({ ... })
    WCE_AST_UNKNOWN
} WceAstType;

typedef struct WceAstNode {
    WceAstType type;
    char* name;         // Function name e.g. "wce_row"
    char* args;         // Raw arguments inside ()
    int is_container;
    struct WceAstNode* parent;
    struct WceAstNode* children;
    struct WceAstNode* next; // Sibling
} WceAstNode;

typedef struct {
    WceAstNode* stack[32];
    int depth;
} WceBuildStack;

WceBuildStack g_build_stack = {0};

// --- Helper Functions ---

void create_dir(const char* path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(size + 1);
    if (buf) {
        fread(buf, 1, size, f);
        buf[size] = '\0';
    }
    fclose(f);
    return buf;
}

// --- AST Management ---

WceAstNode* create_ast_node(WceAstType type, const char* name) {
    WceAstNode* node = (WceAstNode*)malloc(sizeof(WceAstNode));
    memset(node, 0, sizeof(WceAstNode));
    node->type = type;
    if (name) node->name = strdup(name);
    return node;
}

void add_child(WceAstNode* parent, WceAstNode* child) {
    if (!parent || !child) return;
    child->parent = parent;
    if (!parent->children) {
        parent->children = child;
    } else {
        WceAstNode* sib = parent->children;
        while (sib->next) sib = sib->next;
        sib->next = child;
    }
}

void push_build_context(WceAstNode* node) {
    if (g_build_stack.depth < 32) {
        g_build_stack.stack[g_build_stack.depth++] = node;
    }
}

void pop_build_context() {
    if (g_build_stack.depth > 0) {
        g_build_stack.depth--;
    }
}

WceAstNode* current_context() {
    if (g_build_stack.depth > 0) {
        return g_build_stack.stack[g_build_stack.depth - 1];
    }
    return NULL;
}

// --- Parsing Logic ---

void skip_whitespace(const char** p) {
    while (**p && (isspace(**p) || **p == '\n' || **p == '\r')) (*p)++;
}

void skip_comments(const char** p) {
    if (**p == '/' && *(*p + 1) == '/') {
        while (**p && **p != '\n') (*p)++;
    }
}

int parse_identifier(const char** p, char* out_buf) {
    skip_whitespace(p);
    if (!isalpha(**p) && **p != '_') return 0;
    int i = 0;
    while (isalnum(**p) || **p == '_') {
        if (i < 63) out_buf[i++] = **p;
        (*p)++;
    }
    out_buf[i] = '\0';
    return 1;
}

char peek_char(const char** p) {
    return **p;
}

void expect_char(const char** p, char c) {
    skip_whitespace(p);
    if (**p == c) (*p)++;
}

WceAstType func_name_to_type(const char* name) {
    if (strstr(name, "row") || strstr(name, "col") || strstr(name, "card") || strstr(name, "container")) {
        return WCE_AST_CONTAINER;
    }
    return WCE_AST_CALL;
}

WceAstNode* parse_statement(const char** input);

WceAstNode* parse_legacy_syntax(const char** input, const char* func_name) {
    // Standard function call: wce_text("val")
    WceAstNode* node = create_ast_node(WCE_AST_CALL, func_name);
    
    // Parse args until ')'
    char args[256] = {0};
    int i = 0;
    int paren_depth = 0;
    while (**input && (paren_depth > 0 || **input != ')')) {
        if (**input == '(') paren_depth++;
        if (**input == ')') paren_depth--;
        if (i < 255) args[i++] = **input;
        (*input)++;
    }
    args[i] = '\0';
    node->args = strdup(args);
    
    if (**input == ')') (*input)++;
    skip_whitespace(input);
    if (**input == ';') (*input)++;
    
    WceAstNode* parent = current_context();
    if (parent) add_child(parent, node);
    
    return node;
}

// 新增：解析函数调用+大括号作用域
static WceAstNode* parse_function_scope(const char** input) {
    // 1. 解析函数名：wce_row, wce_card等
    char func_name[32];
    if (!parse_identifier(input, func_name)) return NULL;
    
    // 2. 解析参数列表和大括号
    expect_char(input, '(');
    skip_whitespace(input);
    
    // 检查是否为新语法：{ 开头
    if (peek_char(input) == '{') {
        // 新语法：函数作用域
        expect_char(input, '{');
        WceAstNode* node = create_ast_node(func_name_to_type(func_name), func_name);
        node->is_container = 1;
        
        // If we are inside a container, add this as child
        WceAstNode* parent = current_context();
        if (parent) add_child(parent, node);

        // 进入新的作用域
        push_build_context(node);
        
        // 解析大括号内的所有语句
        while (peek_char(input) != '}' && peek_char(input) != '\0') {
            parse_statement(input);
            skip_whitespace(input);
            if (peek_char(input) == ';') (*input)++; // 跳过语句分隔符
        }
        
        expect_char(input, '}');
        expect_char(input, ')');
        if (peek_char(input) == ';') (*input)++; // 可选的语句结束
        
        // 退出作用域
        pop_build_context();
        return node;
    } else {
        // 旧语法或参数列表，保持原有解析逻辑
        return parse_legacy_syntax(input, func_name);
    }
}

WceAstNode* parse_statement(const char** p) {
    skip_whitespace(p);
    skip_comments(p);
    if (**p == '\0' || **p == '}') return NULL;

    // We assume it starts with an identifier
    return parse_function_scope(p);
}

// --- Code Generation ---

void generate_html_recursive(WceAstNode* node, FILE* f, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) fprintf(f, "  ");
    
    if (node->type == WCE_AST_CONTAINER || node->is_container) {
        fprintf(f, "<div class=\"wce-%s\">\n", node->name + 4); // skip wce_
        WceAstNode* child = node->children;
        while (child) {
            generate_html_recursive(child, f, indent + 1);
            child = child->next;
        }
        for (int i = 0; i < indent; i++) fprintf(f, "  ");
        fprintf(f, "</div>\n");
    } else if (node->type == WCE_AST_CALL) {
        fprintf(f, "<!-- %s(%s) -->\n", node->name, node->args ? node->args : "");
    }
}

// --- Main ---

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: wce_parser <input_file>\n");
        return 1;
    }

    char* content = read_file(argv[1]);
    if (!content) {
        printf("Error: Could not read %s\n", argv[1]);
        return 1;
    }

    const char* p = content;
    WceAstNode* root = create_ast_node(WCE_AST_ROOT, "root");
    push_build_context(root);

    printf("Parsing %s...\n", argv[1]);
    while (*p) {
        skip_whitespace(&p);
        if (*p == '\0') break;
        parse_statement(&p);
    }

    // Generate output
    FILE* f = fopen("webcee_output.html", "w");
    if (f) {
        fprintf(f, "<!-- Generated by WebCee C Parser -->\n");
        WceAstNode* child = root->children;
        while (child) {
            generate_html_recursive(child, f, 0);
            child = child->next;
        }
        fclose(f);
        printf("Generated webcee_output.html\n");
    }

    free(content);
    return 0;
}
