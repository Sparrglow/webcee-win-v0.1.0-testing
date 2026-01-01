#include "diagnostic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DiagnosticBag* diagnostic_bag_create(void) {
    DiagnosticBag* bag = (DiagnosticBag*)malloc(sizeof(DiagnosticBag));
    if (!bag) return NULL;
    bag->capacity = 16;
    bag->count = 0;
    bag->error_count = 0;
    bag->items = (Diagnostic*)malloc(sizeof(Diagnostic) * bag->capacity);
    return bag;
}

void diagnostic_bag_destroy(DiagnosticBag* bag) {
    if (!bag) return;
    if (bag->items) {
        for (int i = 0; i < bag->count; i++) {
            if (bag->items[i].file_name) {
                free(bag->items[i].file_name);
            }
        }
        free(bag->items);
    }
    free(bag);
}

void diagnostic_report(DiagnosticBag* bag,
                      const char* file_name,
                      int line, int column,
                      DiagnosticLevel level,
                      int error_code,
                      const char* fmt, ...) {
    if (!bag) return;

    // 扩容
    if (bag->count >= bag->capacity) {
        int new_cap = bag->capacity * 2;
        Diagnostic* new_items = (Diagnostic*)realloc(bag->items, sizeof(Diagnostic) * new_cap);
        if (!new_items) return; // 内存不足，忽略此错误报告
        bag->items = new_items;
        bag->capacity = new_cap;
    }

    Diagnostic* d = &bag->items[bag->count++];
    
    // 复制文件名
    if (file_name) {
        #ifdef _WIN32
        d->file_name = _strdup(file_name);
        #else
        d->file_name = strdup(file_name);
        #endif
    } else {
        d->file_name = NULL;
    }

    d->line = line;
    d->column = column;
    d->level = level;
    d->error_code = error_code;

    // 格式化消息
    va_list args;
    va_start(args, fmt);
    vsnprintf(d->message, sizeof(d->message), fmt, args);
    va_end(args);

    if (level == DIAG_ERROR) {
        bag->error_count++;
    }
}

void diagnostic_print_all(const DiagnosticBag* bag) {
    if (!bag) return;

    for (int i = 0; i < bag->count; i++) {
        Diagnostic* d = &bag->items[i];
        const char* level_str = "INFO";

        // 简单的 ANSI 颜色（如果在支持的终端）
        // Windows CMD 可能默认不支持，这里暂时仅用文本区分
        switch (d->level) {
            case DIAG_ERROR:   level_str = "ERROR"; break;
            case DIAG_WARNING: level_str = "WARN"; break;
            case DIAG_INFO:    level_str = "INFO"; break;
            case DIAG_DEBUG:   level_str = "DEBUG"; break;
        }

        fprintf(stderr, "%s(%d,%d): %s WCE%03d: %s\n",
            d->file_name ? d->file_name : "<unknown>",
            d->line, d->column,
            level_str,
            d->error_code,
            d->message);
    }
}

int diagnostic_has_errors(const DiagnosticBag* bag) {
    return bag && bag->error_count > 0;
}
