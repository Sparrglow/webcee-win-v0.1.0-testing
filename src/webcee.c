#include "webcee.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

// --- Platform Abstraction ---
#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <process.h>
	#ifdef _MSC_VER
	#pragma comment(lib, "ws2_32.lib")
	#endif
	typedef SOCKET wce_socket_t;
	#define WCE_INVALID_SOCKET INVALID_SOCKET
	#define WCE_SOCKET_ERROR SOCKET_ERROR
	#define wce_close_socket closesocket
	#define wce_sleep(ms) Sleep(ms)
	#define WCE_EAGAIN WSAEWOULDBLOCK

	int wce_set_nonblocking(wce_socket_t fd) {
		u_long mode = 1;
		return ioctlsocket(fd, FIONBIO, &mode);
	}

	int wce_get_error(void) {
		return WSAGetLastError();
	}
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <sys/select.h>
	#ifdef __linux__
		#include <sys/epoll.h>
	#endif
	typedef int wce_socket_t;
	#define WCE_INVALID_SOCKET -1
	#define WCE_SOCKET_ERROR -1
	#define wce_close_socket close
	#define wce_sleep(ms) usleep((ms) * 1000)
	#define WCE_EAGAIN EAGAIN

	int wce_set_nonblocking(wce_socket_t fd) {
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) return -1;
		return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}

	int wce_get_error(void) {
		return errno;
	}
#endif

// --- Dynamic Function Registry ---
typedef struct {
    char* name;
    wce_func_t func;
} wce_func_entry_t;

static wce_func_entry_t func_registry[256];
static int func_count = 0;

static char* wce_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = (char*)malloc(len);
    if (new_s) memcpy(new_s, s, len);
    return new_s;
}

void wce_register_function(const char* name, wce_func_t func) {
    if (func_count < 256) {
        func_registry[func_count].name = wce_strdup(name);
        func_registry[func_count].func = func;
        func_count++;
    }
}

// --- Generated Hooks (optional) ---
// If generated code exists and is linked, it will provide these.
// For include-only / no-.wce usage, we provide safe internal stubs.

#if !defined(WEBCEE_HAS_GENERATED)
	#if defined(__has_include)
		#if __has_include("webcee_generated.h")
			#include "webcee_generated.h"
		#endif
	#endif
#endif

#if !defined(WEBCEE_GENERATED_H)
	/*
	 * 生成代码不存在时：
	 * - header-only 模式：提供 static stub，保证“只 include 就能跑”。
	 * - library 模式：只提供声明，避免与 webcee_generated.c 的真实实现冲突。
	 */
    // In library mode (this file), we provide weak or default implementations if not linked.
    // But since this is a C file, we can just provide them as weak symbols or just normal functions
    // if we assume the user won't link generated code that conflicts.
    // However, if the user links webcee_generated.c, we get duplicate symbols.
    // So we need them to be weak.
    // Windows doesn't support weak symbols easily.
    // Strategy: We assume if WEBCEE_HAS_GENERATED is not defined, we provide stubs.
    // But this is compiled once.
    // Let's just provide them for now. If user links generated code, they might need to handle it.
    // Actually, the previous header-only logic handled this via static.
    // Here, we are in a .c file.
    
    // For now, we provide them. If conflict arises, we can revisit.
    // Or we can make them weak if on GCC.
    #if defined(__GNUC__) || defined(__clang__)
    __attribute__((weak))
    #endif
	char* wce_get_list_json(const char* name) { (void)name; return (char*)"[]"; }
    
    #if defined(__GNUC__) || defined(__clang__)
    __attribute__((weak))
    #endif
	void wce_handle_model_update(const char* key, const char* val) { (void)key; (void)val; }
    
    #if defined(__GNUC__) || defined(__clang__)
    __attribute__((weak))
    #endif
	void wce_dispatch_event(const char* event, const char* args) {
        (void)args;
        // Check dynamic registry first
        for (int i = 0; i < func_count; i++) {
            if (strcmp(func_registry[i].name, event) == 0) {
                if (func_registry[i].func) func_registry[i].func();
                return;
            }
        }
    }
#endif

// --- Data Structures ---
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

typedef struct {
	wce_socket_t fd;
	char buffer[BUFFER_SIZE];
	int buf_len;
	int active;
} wce_client_t;

static wce_client_t clients[MAX_CLIENTS];
static wce_socket_t server_fd = WCE_INVALID_SOCKET;
static volatile int is_running = 0;
static int server_port = 80;

// KV Store
typedef struct {
	char* key;
	char* value;
} wce_kv_t;

#define MAX_KV_STORE 100
static wce_kv_t kv_store[MAX_KV_STORE];
static int kv_count = 0;

// Minimal embedded UI (served when no web_root found)
// Modified to support Runtime Rendering (SSR from C structure)
static const char* WCE_HTML_HEADER =
	"<!DOCTYPE html><html><head><meta charset='UTF-8'>"
	"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
	"<title>WebCee App</title>"
	"<style>"
	"body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;margin:0;padding:20px;background:#f0f2f5;}"
	".container{max_width:800px;margin:0 auto;}"
	".row{display:flex;flex-wrap:wrap;margin:-10px;}"
	".col{flex:1;padding:10px;min-width:200px;}"
	".card{background:white;border-radius:8px;padding:20px;box-shadow:0 2px 4px rgba(0,0,0,0.1);margin-bottom:20px;}"
	"button{background:#007bff;color:white;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;font-size:14px;}"
	"button:hover{background:#0056b3;}"
	"input{padding:8px;border:1px solid #ddd;border-radius:4px;width:100%;box-sizing:border-box;}"
	"</style></head><body><div id='app'>";

static const char* WCE_HTML_FOOTER =
	"</div>"
	"<script>"
	"function trigger(evt){fetch('/api/trigger?event='+evt,{method:'POST'});}"
	"async function sync(){"
	"  try{const r=await fetch('/api/data');const d=await r.json();"
	"  document.querySelectorAll('[wce-bind]').forEach(el=>{"
	"    const k=el.getAttribute('wce-bind');"
	"    if(d[k]!==undefined) el.textContent=d[k];"
	"  });"
	"  }catch(e){}"
	"}"
	"setInterval(sync, 500); sync();"
	"</script></body></html>";

// --- Helper Functions ---
static void str_append(char** buf, size_t* cap, size_t* len, const char* str) {
    size_t l = strlen(str);
    if (*len + l >= *cap) {
        *cap = (*cap + l) * 2 + 4096;
        *buf = (char*)realloc(*buf, *cap);
    }
    strcpy(*buf + *len, str);
    *len += l;
}

static void wce_render_node_recursive(WceNode* node, char** buf, size_t* cap, size_t* len) {
    if (!node) return;

    // Open tag
    switch (node->type) {
        case WCE_NODE_ROOT: break;
        case WCE_NODE_CONTAINER: str_append(buf, cap, len, "<div class='container'>"); break;
        case WCE_NODE_ROW:       str_append(buf, cap, len, "<div class='row'>"); break;
        case WCE_NODE_COL:       str_append(buf, cap, len, "<div class='col'>"); break;
        case WCE_NODE_CARD:      str_append(buf, cap, len, "<div class='card'>"); break;
        case WCE_NODE_PANEL:     str_append(buf, cap, len, "<div class='panel'>"); break;
        case WCE_NODE_TEXT:
            str_append(buf, cap, len, "<span");
            if (node->value_ref) {
                 str_append(buf, cap, len, " wce-bind='");
                 str_append(buf, cap, len, node->value_ref);
                 str_append(buf, cap, len, "'");
            }
            str_append(buf, cap, len, ">");
            if (node->label) str_append(buf, cap, len, node->label);
            str_append(buf, cap, len, "</span>");
            break;
        case WCE_NODE_BUTTON:
            str_append(buf, cap, len, "<button");
            if (node->event_handler) {
                str_append(buf, cap, len, " onclick=\"trigger('");
                str_append(buf, cap, len, node->event_handler);
                str_append(buf, cap, len, "')\"");
            }
            str_append(buf, cap, len, ">");
            if (node->label) str_append(buf, cap, len, node->label);
            str_append(buf, cap, len, "</button>");
            break;
        default: break;
    }

    // Children
    WceNode* child = node->first_child;
    while (child) {
        wce_render_node_recursive(child, buf, cap, len);
        child = child->next_sibling;
    }

    // Close tag
    switch (node->type) {
        case WCE_NODE_CONTAINER:
        case WCE_NODE_ROW:
        case WCE_NODE_COL:
        case WCE_NODE_CARD:
        case WCE_NODE_PANEL:
            str_append(buf, cap, len, "</div>"); break;
        default: break;
    }
}

char* wce_render_dom() {
    size_t cap = 8192;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    buf[0] = '\0';
    
    str_append(&buf, &cap, &len, WCE_HTML_HEADER);
    
    if (_wce_root) {
        wce_render_node_recursive(_wce_root, &buf, &cap, &len);
    } else {
        str_append(&buf, &cap, &len, "<div class='container'><div class='card'><h3>No UI Defined</h3><p>Use wce_ui_begin() ... wce_ui_end() in main.c</p></div></div>");
    }
    
    str_append(&buf, &cap, &len, WCE_HTML_FOOTER);
    return buf;
}

// --- Runtime UI Construction Implementation ---
static WceNode* _wce_root = NULL;
static WceNode* _wce_ctx_stack[32];
static int _wce_ctx_top = -1;

WceNode* _wce_node_create(WceNodeType type) {
    WceNode* n = (WceNode*)malloc(sizeof(WceNode));
    memset(n, 0, sizeof(WceNode));
    n->type = type;
    return n;
}

void _wce_push_context(WceNode* node) {
    if (_wce_ctx_top < 31) {
        _wce_ctx_stack[++_wce_ctx_top] = node;
    }
    if (!_wce_root) _wce_root = node;
}

void _wce_pop_context(void) {
    if (_wce_ctx_top >= 0) {
        _wce_ctx_top--;
    }
}

WceNode* _wce_current_context(void) {
    if (_wce_ctx_top >= 0) return _wce_ctx_stack[_wce_ctx_top];
    return NULL;
}

void _wce_add_child(WceNode* parent, WceNode* child) {
    if (!parent || !child) return;
    child->parent = parent;
    if (!parent->first_child) {
        parent->first_child = child;
        parent->last_child = child;
    } else {
        parent->last_child->next_sibling = child;
        parent->last_child = child;
    }
}

void _wce_node_set_prop(WceNode* node, const char* label, const char* val_ref, const char* evt) {
    if (label) {
        // Auto-detect binding syntax {{ key }}
        if (strncmp(label, "{{", 2) == 0 && strstr(label, "}}")) {
            char* start = strstr(label, "{{") + 2;
            char* end = strstr(label, "}}");
            size_t len = end - start;
            char* key = (char*)malloc(len + 1);
            strncpy(key, start, len);
            key[len] = '\0';
            
            // Trim spaces
            char* k = key;
            while(*k == ' ') k++;
            char* k_end = k + strlen(k) - 1;
            while(k_end > k && *k_end == ' ') *k_end-- = '\0';

            #ifdef _WIN32
            node->value_ref = _strdup(k);
            #else
            node->value_ref = strdup(k);
            #endif
            free(key);
            node->label = NULL; // Clear label as it is bound
        } else {
            #ifdef _WIN32
            node->label = _strdup(label);
            #else
            node->label = strdup(label);
            #endif
        }
    }
    if (val_ref) {
        #ifdef _WIN32
        node->value_ref = _strdup(val_ref);
        #else
        node->value_ref = strdup(val_ref);
        #endif
    }
    if (evt) {
        #ifdef _WIN32
        node->event_handler = _strdup(evt);
        #else
        node->event_handler = strdup(evt);
        #endif
    }
}

void wce_reset_client(int index) {
	if (clients[index].fd != WCE_INVALID_SOCKET) {
		wce_close_socket(clients[index].fd);
	}
	clients[index].fd = WCE_INVALID_SOCKET;
	clients[index].buf_len = 0;
	clients[index].active = 0;
}

char* read_file_content(const char* path, size_t* out_len) {
	FILE* f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size < 0) { fclose(f); return NULL; }
	char* buffer = (char*)malloc((size_t)size + 1);
	if (!buffer) { fclose(f); return NULL; }
	size_t read = fread(buffer, 1, (size_t)size, f);
	buffer[read] = '\0';
	if (out_len) *out_len = read;
	fclose(f);
	return buffer;
}

void send_response(wce_socket_t client_fd, const char* status, const char* content_type, const char* body, size_t body_len) {
	char header[1024];
	int header_len = snprintf(header, sizeof(header),
		"HTTP/1.1 %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %zu\r\n"
		"Connection: close\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"\r\n",
		status, content_type, body_len);

	send(client_fd, header, header_len, 0);
	if (body && body_len > 0) {
		send(client_fd, body, body_len, 0);
	}
}

void process_request(int client_idx) {
	wce_client_t* c = &clients[client_idx];
	c->buffer[c->buf_len] = '\0';

	char method[16], path[256];
	if (sscanf(c->buffer, "%15s %255s", method, path) != 2) {
		return;
	}

	// API: List Data
	if (strncmp(path, "/api/list", 9) == 0 && strcmp(method, "GET") == 0) {
		char* q = strchr(path, '?');
		if (q) {
			char* name_param = strstr(q, "name=");
			if (name_param) {
				char* list_name = name_param + 5;
				char* end = strchr(list_name, '&');
				if (end) *end = '\0';
				char* json = wce_get_list_json(list_name);
				send_response(c->fd, "200 OK", "application/json", json, strlen(json));
				return;
			}
		}
		send_response(c->fd, "400 Bad Request", "text/plain", "Missing name param", 18);
		return;
	}

	// API: Model Update
	if (strncmp(path, "/api/update", 11) == 0 && strcmp(method, "POST") == 0) {
		char* q = strchr(path, '?');
		if (q) {
			char* key_param = strstr(q, "key=");
			char* val_param = strstr(q, "val=");
			if (key_param && val_param) {
				char* key = key_param + 4;
				char* end_key = strchr(key, '&');
				if (end_key) *end_key = '\0';
				char* val = val_param + 4;
				char* end_val = strchr(val, '&');
				if (end_val) *end_val = '\0';
				wce_handle_model_update(key, val);
				send_response(c->fd, "200 OK", "text/plain", "OK", 2);
				return;
			}
		}
		send_response(c->fd, "400 Bad Request", "text/plain", "Missing params", 14);
		return;
	}

	// API: Data Sync
	if (strcmp(path, "/api/data") == 0 && strcmp(method, "GET") == 0) {
		char json[4096] = "{";
		for (int i = 0; i < kv_count; i++) {
			if (i > 0) strcat(json, ",");
			char entry[256];
			snprintf(entry, sizeof(entry), "\"%s\":\"%s\"", kv_store[i].key, kv_store[i].value);
			strcat(json, entry);
		}
		strcat(json, "}");
		send_response(c->fd, "200 OK", "application/json", json, strlen(json));
		return;
	}

	// API: Event Trigger
	if (strncmp(path, "/api/trigger", 12) == 0 && strcmp(method, "POST") == 0) {
		char* q = strchr(path, '?');
		if (q) {
			char* event_param = strstr(q, "event=");
			if (event_param) {
				char* event_name = event_param + 6;
				char* end = strchr(event_name, '&');
				if (end) *end = '\0';
				char* arg = "";
				char* arg_param = strstr(q, "arg=");
				if (arg_param) {
					arg = arg_param + 4;
					char* end_arg = strchr(arg, '&');
					if (end_arg) *end_arg = '\0';
				}
				wce_dispatch_event(event_name, arg);
				send_response(c->fd, "200 OK", "text/plain", "OK", 2);
				return;
			}
		}
		send_response(c->fd, "400 Bad Request", "text/plain", "Missing event param", 19);
		return;
	}

	// Static Assets
	const char* file_path = path;
	if (strcmp(path, "/") == 0) file_path = "/index.html";

	const char* search_paths[] = {
		"web_root",
		"./web_root",
		"generated/web_root",
		"../web_root",
		"../../web_root",
		NULL
	};

	char full_path[512];
	char* content = NULL;
	size_t len = 0;

	for (int i = 0; search_paths[i] != NULL; i++) {
		snprintf(full_path, sizeof(full_path), "%s%s", search_paths[i], file_path);
		content = read_file_content(full_path, &len);
		if (content) break;
	}

	if (content) {
		const char* type = "text/plain";
		if (strstr(path, ".html") || strcmp(path, "/") == 0) type = "text/html";
		else if (strstr(path, ".css")) type = "text/css";
		else if (strstr(path, ".js")) type = "application/javascript";
		send_response(c->fd, "200 OK", type, content, len);
		free(content);
		return;
	}

	// Embedded fallback for include-only usage
	if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
		char* html = wce_render_dom();
		send_response(c->fd, "200 OK", "text/html", html, strlen(html));
		free(html);
		return;
	}

	send_response(c->fd, "404 Not Found", "text/plain", "Not Found", 9);
}

void server_loop(void) {
	while (is_running) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(server_fd, &readfds);
		wce_socket_t max_fd = server_fd;

		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clients[i].active) {
				FD_SET(clients[i].fd, &readfds);
				if (clients[i].fd > max_fd) max_fd = clients[i].fd;
			}
		}

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		int activity = select((int)max_fd + 1, &readfds, NULL, NULL, &tv);
		if (activity < 0) {
			continue;
		}

		if (FD_ISSET(server_fd, &readfds)) {
			struct sockaddr_in addr;
			#ifdef _WIN32
				int addrlen = sizeof(addr);
			#else
				socklen_t addrlen = sizeof(addr);
			#endif
			wce_socket_t client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
			if (client_fd != WCE_INVALID_SOCKET) {
				wce_set_nonblocking(client_fd);
				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (!clients[i].active) {
						clients[i].fd = client_fd;
						clients[i].buf_len = 0;
						clients[i].active = 1;
						break;
					}
				}
			}
		}

		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (!clients[i].active) continue;
			if (!FD_ISSET(clients[i].fd, &readfds)) continue;

			int bytes = recv(clients[i].fd, clients[i].buffer, BUFFER_SIZE - 1, 0);
			if (bytes <= 0) {
				wce_reset_client(i);
				continue;
			}
			clients[i].buf_len = bytes;
			process_request(i);
			wce_reset_client(i);
		}
	}
}

#ifdef _WIN32
unsigned __stdcall server_thread(void* arg) {
	(void)arg;
	server_loop();
	return 0;
}
#else
void* server_thread(void* arg) {
	(void)arg;
	server_loop();
	return NULL;
}
#endif

int wce_init(int port) {
	server_port = port;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].fd = WCE_INVALID_SOCKET;
		clients[i].buf_len = 0;
		clients[i].active = 0;
	}

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return -1;
	}
#endif

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == WCE_INVALID_SOCKET) return -1;

	int opt = 1;
	#ifdef _WIN32
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	#else
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	#endif

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons((unsigned short)server_port);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == WCE_SOCKET_ERROR) {
		wce_close_socket(server_fd);
		server_fd = WCE_INVALID_SOCKET;
		return -1;
	}

	if (listen(server_fd, 16) == WCE_SOCKET_ERROR) {
		wce_close_socket(server_fd);
		server_fd = WCE_INVALID_SOCKET;
		return -1;
	}

	wce_set_nonblocking(server_fd);
	return 0;
}

int wce_start(void) {
	if (server_fd == WCE_INVALID_SOCKET) return -1;
	if (is_running) return 0;
	is_running = 1;

#ifdef _WIN32
	uintptr_t handle = _beginthreadex(NULL, 0, server_thread, NULL, 0, NULL);
	if (!handle) { is_running = 0; return -1; }
	CloseHandle((HANDLE)handle);
#else
	pthread_t thread;
	if (pthread_create(&thread, NULL, server_thread, NULL) != 0) { is_running = 0; return -1; }
	pthread_detach(thread);
#endif
	return 0;
}

void wce_stop(void) {
	is_running = 0;
	if (server_fd != WCE_INVALID_SOCKET) {
		wce_close_socket(server_fd);
		server_fd = WCE_INVALID_SOCKET;
	}

#ifdef _WIN32
	WSACleanup();
#endif
}

void wce_data_set(const char* key, const char* val) {
	if (!key || !val) return;
	for (int i = 0; i < kv_count; i++) {
		if (strcmp(kv_store[i].key, key) == 0) {
			free(kv_store[i].value);
			#ifdef _WIN32
				kv_store[i].value = _strdup(val);
			#else
				kv_store[i].value = strdup(val);
			#endif
			return;
		}
	}
	if (kv_count >= MAX_KV_STORE) return;
	#ifdef _WIN32
		kv_store[kv_count].key = _strdup(key);
		kv_store[kv_count].value = _strdup(val);
	#else
		kv_store[kv_count].key = strdup(key);
		kv_store[kv_count].value = strdup(val);
	#endif
	kv_count++;
}

const char* wce_version(void) {
	return "0.1";
}

int wce_is_connected(void) {
	// Current HTTP-only transport: return whether server is running.
	return is_running ? 1 : 0;
}
