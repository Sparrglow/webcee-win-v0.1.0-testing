#ifndef WEBCEE_H
#define WEBCEE_H

#ifdef __cplusplus
extern "C" {
#endif

#define WEBCEE_API

/* 初始化与启动 */
WEBCEE_API int wce_init(int port);                    // 初始化WebCee服务
WEBCEE_API int wce_start(void);                       // 启动服务（非阻塞）
WEBCEE_API void wce_stop(void);                       // 停止服务

/* 数据同步 (C -> 前端) */
WEBCEE_API void wce_data_set(const char* key, const char* val);     // 更新单个数据

/* 工具函数 */
WEBCEE_API const char* wce_version(void);             // 获取框架版本
WEBCEE_API int wce_is_connected(void);                // 检查前端连接状态

/*
 * 可选：当通过 CMake 的 target_add_webcee_ui() 绑定了 .wce 文件时，
 * 会自动定义 WEBCEE_HAS_GENERATED=1，并把生成目录加入 include path。
 * 这样用户只需 include 本头文件即可获得 WCE_EVT_* 等生成接口声明。
 */
#if defined(WEBCEE_HAS_GENERATED)
#include "webcee_generated.h"
#else
/* Fallback: when compiler supports __has_include, include if present */
#if defined(__has_include)
#if __has_include("webcee_generated.h")
#include "webcee_generated.h"
#endif
#endif
#endif

#include "webcee_build.h"

#ifdef __cplusplus
}
#endif

#endif // WEBCEE_H
