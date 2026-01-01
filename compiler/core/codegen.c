#include "codegen.h"
#include "token.h"

static void generate_expression(WceAstNode* node, FILE* out);
static void generate_statement(WceAstNode* node, FILE* out, int indent);

static void print_indent(FILE* out, int indent) {
    for (int i = 0; i < indent; i++) fprintf(out, "    ");
}

void codegen_generate(WceAstNode* root, FILE* out) {
    if (!root) return;
    
    // Root children are statements
    WceAstNode* child = root->first_child;
    while (child) {
        generate_statement(child, out, 0);
        child = child->next_sibling;
    }
}

static void generate_expression(WceAstNode* node, FILE* out) {
    if (!node) return;
    switch (node->type) {
        case NODE_IDENTIFIER:
            if (node->first_child) { // Binary op (assignment or comparison)
                generate_expression(node->first_child, out);
                fprintf(out, " %s ", node->string_value);
                generate_expression(node->first_child->next_sibling, out);
            } else {
                fprintf(out, "%s", node->string_value);
            }
            break;
        case NODE_STRING_LITERAL:
            fprintf(out, "\"%s\"", node->string_value);
            break;
        case NODE_NUMBER_LITERAL:
            fprintf(out, "%g", node->number_value);
            break;
        case NODE_FUNCTION_CALL:
            fprintf(out, "%s(", node->string_value);
            WceAstNode* arg = node->first_child;
            while (arg) {
                generate_expression(arg, out);
                arg = arg->next_sibling;
                if (arg) fprintf(out, ", ");
            }
            fprintf(out, ")");
            break;
        default:
            break;
    }
}

static void generate_statement(WceAstNode* node, FILE* out, int indent) {
    if (!node) return;
    
    if (node->type == NODE_BLOCK) {
        print_indent(out, indent);
        fprintf(out, "{\n");
        WceAstNode* child = node->first_child;
        while (child) {
            generate_statement(child, out, indent + 1);
            child = child->next_sibling;
        }
        print_indent(out, indent);
        fprintf(out, "}\n");
        return;
    }
    
    if (node->type == NODE_IF) {
        print_indent(out, indent);
        fprintf(out, "if (");
        generate_expression(node->first_child, out);
        fprintf(out, ")\n");
        
        WceAstNode* then_branch = node->first_child->next_sibling;
        generate_statement(then_branch, out, indent);
        
        WceAstNode* else_branch = then_branch ? then_branch->next_sibling : NULL;
        if (else_branch) {
            print_indent(out, indent);
            fprintf(out, "else\n");
            generate_statement(else_branch, out, indent);
        }
        return;
    }
    
    if (node->type == NODE_FOR) {
        print_indent(out, indent);
        fprintf(out, "for (");
        WceAstNode* init = node->first_child;
        generate_expression(init, out);
        fprintf(out, "; ");
        
        WceAstNode* cond = init ? init->next_sibling : NULL;
        generate_expression(cond, out);
        fprintf(out, "; ");
        
        WceAstNode* inc = cond ? cond->next_sibling : NULL;
        generate_expression(inc, out);
        fprintf(out, ")\n");
        
        WceAstNode* body = inc ? inc->next_sibling : NULL;
        generate_statement(body, out, indent);
        return;
    }
    
    // Expression statement
    print_indent(out, indent);
    
    // Special handling for function calls with blocks (Containers)
    if (node->type == NODE_FUNCTION_CALL && node->block) {
        // Generate: func_name_begin(args);
        fprintf(out, "%s_begin(", node->string_value);
        WceAstNode* arg = node->first_child;
        while (arg) {
            generate_expression(arg, out);
            arg = arg->next_sibling;
            if (arg) fprintf(out, ", ");
        }
        fprintf(out, ");\n");
        
        // Generate block body
        generate_statement(node->block, out, indent);
        
        // Generate: func_name_end();
        print_indent(out, indent);
        fprintf(out, "%s_end();\n", node->string_value);
        return;
    }

    generate_expression(node, out);
    fprintf(out, ";\n");
}
