#ifndef WEBCEE_CODEGEN_H
#define WEBCEE_CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Generate C code from AST
void codegen_generate(WceAstNode* root, FILE* out);

#endif // WEBCEE_CODEGEN_H
