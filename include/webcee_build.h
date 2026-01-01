#ifndef WEBCEE_BUILD_H
#define WEBCEE_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// --- Node Types ---
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

// --- Node Structure ---
typedef struct WceNode {
    WceNodeType type;
    char* label;
    char* value_ref;
    char* event_handler;
    struct WceNode* first_child;
    struct WceNode* last_child;
    struct WceNode* next_sibling;
    struct WceNode* parent;
    int col_span;
    char* style;
} WceNode;

// --- Context Management ---
extern WceNode* _wce_current_context(void);
extern void _wce_push_context(WceNode* ctx);
extern void _wce_pop_context(void);
extern WceNode* _wce_node_create(WceNodeType type);
extern void _wce_add_child(WceNode* parent, WceNode* child);

// --- Helper Functions ---

static inline void _wce_begin_container(WceNodeType type) {
    WceNode* node = _wce_node_create(type);
    WceNode* parent = _wce_current_context();
    if (parent) {
        _wce_add_child(parent, node);
    }
    _wce_push_context(node);
}

static inline void _wce_end_container(void) {
    _wce_pop_context();
}

// --- Container Macros ---
#define wce_container_begin() _wce_begin_container(WCE_NODE_CONTAINER)
#define wce_container_end()   _wce_end_container()

#define wce_row_begin() _wce_begin_container(WCE_NODE_ROW)
#define wce_row_end()   _wce_end_container()

#define wce_col_begin() _wce_begin_container(WCE_NODE_COL)
#define wce_col_end()   _wce_end_container()

#define wce_card_begin() _wce_begin_container(WCE_NODE_CARD)
#define wce_card_end()   _wce_end_container()

#define wce_panel_begin() _wce_begin_container(WCE_NODE_PANEL)
#define wce_panel_end()   _wce_end_container()

// --- Leaf Node Macros (Block Support) ---

static inline void wce_text_begin(const char* text) {
    WceNode* node = _wce_node_create(WCE_NODE_TEXT);
    node->label = (char*)text;
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
    _wce_push_context(node);
}
#define wce_text_end() _wce_pop_context()

static inline void wce_button_begin(const char* label) {
    WceNode* node = _wce_node_create(WCE_NODE_BUTTON);
    node->label = (char*)label;
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
    _wce_push_context(node);
}
#define wce_button_end() _wce_pop_context()

static inline void wce_input_begin(const char* placeholder) {
    WceNode* node = _wce_node_create(WCE_NODE_INPUT);
    node->label = (char*)placeholder;
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
    _wce_push_context(node);
}
#define wce_input_end() _wce_pop_context()

// --- Leaf Node Functions (No Block) ---

static inline void wce_text(const char* text) {
    WceNode* node = _wce_node_create(WCE_NODE_TEXT);
    node->label = (char*)text; 
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
}

static inline void wce_button(const char* label) {
    WceNode* node = _wce_node_create(WCE_NODE_BUTTON);
    node->label = (char*)label;
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
}

static inline void wce_input(const char* placeholder) {
    WceNode* node = _wce_node_create(WCE_NODE_INPUT);
    node->label = (char*)placeholder;
    WceNode* parent = _wce_current_context();
    if (parent) _wce_add_child(parent, node);
}

// --- Property Setters ---

static inline void wce_css(const char* style) {
    WceNode* node = _wce_current_context();
    if (node) node->style = (char*)style;
}

static inline void wce_bind(const char* ref) {
    WceNode* node = _wce_current_context();
    if (node) node->value_ref = (char*)ref;
}

static inline void wce_on_click(const char* handler) {
    WceNode* node = _wce_current_context();
    if (node) node->event_handler = (char*)handler;
}

#ifdef __cplusplus
}
#endif

#endif // WEBCEE_BUILD_H
