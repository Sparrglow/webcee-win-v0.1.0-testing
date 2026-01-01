// webcee_build.h - 构建期UI定义API（用户编写.wce时包含此文件）
#ifndef WEBCEE_BUILD_H
#define WEBCEE_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif

// --- 上下文管理（内部使用） ---
typedef struct WceNode WceNode; // Forward declaration

// 上下文管理
extern WceNode* _wce_current_context(void);
extern void _wce_push_context(WceNode* ctx);
extern void _wce_pop_context(void);

// 节点类型定义
typedef enum {
    WCE_NODE_ROOT,
    WCE_NODE_CONTAINER,
    WCE_NODE_ROW,
    WCE_NODE_COL,
    WCE_NODE_CARD,
    WCE_NODE_PANEL,
    WCE_NODE_TEXT,
    WCE_NODE_BUTTON,
    WCE_NODE_SLIDER,
    WCE_NODE_PROGRESS,
    WCE_NODE_INPUT
} WceNodeType;

// 兼容别名：允许使用 WCE_NODE_COLUMN
#ifndef WCE_NODE_COLUMN
#define WCE_NODE_COLUMN WCE_NODE_COL
#endif

// 节点结构
struct WceNode {
    WceNodeType type;
    char* label;
    char* value_ref;
    char* event_handler;
    struct WceNode* first_child;
    struct WceNode* last_child;
    struct WceNode* next_sibling;
    struct WceNode* parent;
    int col_span;
    char* style; // Added for CSS styling
    // ... 其他属性
};

// 节点创建与操作
extern WceNode* _wce_node_create(WceNodeType type);
extern void _wce_add_child(WceNode* parent, WceNode* child);
extern void _wce_node_set_prop(WceNode* node, const char* label, const char* val_ref, const char* evt);
extern void _wce_add_style(const char* style);

// Style macro
#define wce_css(style) _wce_add_style(style)

// --- 核心宏定义 (GNU C Extension for Statement Expressions) ---

#if defined(__GNUC__) || defined(__clang__)
    #define _WCE_CONTAINER(type, ...) \
        __extension__({ \
            WceNode *_node = _wce_node_create(type); \
            _wce_add_child(_wce_current_context(), _node); \
            _wce_push_context(_node); \
            do { __VA_ARGS__; } while(0); \
            _wce_pop_context(); \
            _node; \
        })
#else
    // Fallback for MSVC or standard C (using do-while(0), returns void effectively)
    // Note: This might not work if the macro is used as an expression, but works for statements.
    #define _WCE_CONTAINER(type, ...) \
        do { \
            WceNode *_node = _wce_node_create(type); \
            _wce_add_child(_wce_current_context(), _node); \
            _wce_push_context(_node); \
            do { __VA_ARGS__; } while(0); \
            _wce_pop_context(); \
        } while(0)
#endif

// --- 容器组件 API ---
#define wce_row(...)          _WCE_CONTAINER(WCE_NODE_ROW, __VA_ARGS__)
#define wce_col(...)          _WCE_CONTAINER(WCE_NODE_COL, __VA_ARGS__)
#define wce_column(...)       wce_col(__VA_ARGS__)
#define wce_card(...)         _WCE_CONTAINER(WCE_NODE_CARD, __VA_ARGS__)
#define wce_panel(...)        _WCE_CONTAINER(WCE_NODE_PANEL, __VA_ARGS__)
#define wce_container(...)    _WCE_CONTAINER(WCE_NODE_CONTAINER, __VA_ARGS__)

// --- 叶子组件 API ---

// Helper macros for leaf nodes
#define _WCE_LEAF_TEXT(fmt, ...) \
    do { \
        WceNode* _n = _wce_node_create(WCE_NODE_TEXT); \
        /* 当前实现：仅存储原始字符串 fmt（忽略可变参数格式化） */ \
        _wce_node_set_prop(_n, (fmt), NULL, NULL); \
        _wce_add_child(_wce_current_context(), _n); \
    } while(0)

#define _WCE_LEAF_BUTTON(label, cb) \
    do { \
        WceNode* _n = _wce_node_create(WCE_NODE_BUTTON); \
        _wce_node_set_prop(_n, (label), NULL, #cb); \
        _wce_add_child(_wce_current_context(), _n); \
    } while(0)

#define _WCE_LEAF_SLIDER(label, var_ptr, min, max) \
    do { \
        WceNode* _n = _wce_node_create(WCE_NODE_SLIDER); \
        (void)(min); (void)(max); \
        _wce_node_set_prop(_n, (label), #var_ptr, NULL); \
        _wce_add_child(_wce_current_context(), _n); \
    } while(0)

#define _WCE_LEAF_PROGRESS(label, var_ptr) \
    do { \
        WceNode* _n = _wce_node_create(WCE_NODE_PROGRESS); \
        _wce_node_set_prop(_n, (label), #var_ptr, NULL); \
        _wce_add_child(_wce_current_context(), _n); \
    } while(0)

#define _WCE_LEAF_INPUT(label, key) \
    do { \
        WceNode* _n = _wce_node_create(WCE_NODE_INPUT); \
        _wce_node_set_prop(_n, (label), (key), NULL); \
        _wce_add_child(_wce_current_context(), _n); \
    } while(0)

#define wce_text(fmt, ...)    _WCE_LEAF_TEXT(fmt, ##__VA_ARGS__)
#define wce_button(label, cb) _WCE_LEAF_BUTTON(label, cb)
#define wce_slider(label, var_ptr, min, max) _WCE_LEAF_SLIDER(label, var_ptr, min, max)
#define wce_progress(label, var_ptr)         _WCE_LEAF_PROGRESS(label, var_ptr)
#define wce_input(label, key)                _WCE_LEAF_INPUT(label, key)

// --- 运行时构建 API (Manual Mode) ---
#define wce_ui_begin() \
    do { \
        WceNode* _root = _wce_node_create(WCE_NODE_ROOT); \
        _wce_push_context(_root); \
    } while(0)

#define wce_ui_end() \
    do { \
        _wce_pop_context(); \
    } while(0)

#define WCE_MODEL(key, val) wce_data_set(key, val)
#define WCE_FUNC(name, func) wce_register_function(name, func)

// --- 兼容层：旧宏映射到新函数（保持向后兼容） ---
// 注意：这里利用了宏展开的特性，将 BEGIN 展开为函数调用的前半部分
#define WCE_ROW_BEGIN         wce_row({
#define WCE_ROW_END           });
#define WCE_COL_BEGIN         wce_col({
#define WCE_COL_END           });
#define WCE_CARD_BEGIN        wce_card({
#define WCE_CARD_END          });
#define WCE_CONTAINER_BEGIN   wce_container({
#define WCE_CONTAINER_END     });

#ifdef __cplusplus
}
#endif

#endif // WEBCEE_BUILD_H
