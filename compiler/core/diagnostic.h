#ifndef WEBCEE_DIAGNOSTIC_H
#define WEBCEE_DIAGNOSTIC_H

#include <stdarg.h>

typedef enum {
    DIAG_ERROR,   // 错误，编译中止
    DIAG_WARNING, // 警告，编译继续
    DIAG_INFO,    // 信息
    DIAG_DEBUG    // 调试信息（仅调试模式）
} DiagnosticLevel;

typedef struct {
    char* file_name;         // 源文件名 (使用动态分配或指向静态字符串)
    int line;                // 行号（从1开始）
    int column;              // 列号（从1开始）
    DiagnosticLevel level;
    int error_code;          // 唯一错误码
    char message[256];       // 人类可读的错误信息
} Diagnostic;

typedef struct {
    Diagnostic* items;       // 诊断信息数组
    int capacity;            // 数组容量
    int count;               // 当前数量
    int error_count;         // 错误数
} DiagnosticBag;

// 创建和销毁诊断包
DiagnosticBag* diagnostic_bag_create(void);
void diagnostic_bag_destroy(DiagnosticBag* bag);

// 报告诊断信息
void diagnostic_report(DiagnosticBag* bag,
                      const char* file_name,
                      int line, int column,
                      DiagnosticLevel level,
                      int error_code,
                      const char* fmt, ...);

// 打印所有诊断信息到标准错误输出
void diagnostic_print_all(const DiagnosticBag* bag);

// 检查是否存在错误
int diagnostic_has_errors(const DiagnosticBag* bag);

#endif // WEBCEE_DIAGNOSTIC_H
