# API 参考 (API Reference)

本页面面向 **WebCee v0.1**，对外仅描述函数作用域写法与当前已实现的运行时接口。

## UI 定义（函数作用域语法）

`.wce` 里使用 `wce_*` 的函数作用域写法来描述 UI 结构（接近 C 代码块风格）。

```c
wce_container({
    wce_row({
        wce_col({
            wce_card({
                wce_text("Hello WebCee");
                wce_button("Click", on_click);
            });
        });
    });
});
```

### 容器与布局
- `wce_container({ ... });`: 居中容器。
- `wce_row({ ... });`: 行布局（Flex Row）。
- `wce_col({ ... });`: 列布局（等价别名：`wce_column({ ... });`）。
- `wce_card({ ... });`: 卡片容器。
- `wce_panel({ ... });`: 面板容器。

### 文本与交互（叶子节点）
- `wce_text(text);`: 文本。
- `wce_button(text, callback);`: 按钮。
- `wce_slider(label, var_ptr, min, max);`: 滑条（当前生成器以注释形式保留参数）。
- `wce_progress(label, var_ptr);`: 进度（当前生成器以注释形式保留参数）。
- `wce_input(label, key);`: 输入框（当前生成器以注释形式保留参数）。

说明：对外文档仅展示函数作用域写法。

## C 运行时 API (Runtime API)

这些函数在您的 C 代码中使用 (`include/webcee.h`)。

- `int wce_init(int port)`: 初始化服务器。
- `int wce_start(void)`: 启动服务器线程。
- `void wce_stop(void)`: 停止服务器。
- `void wce_data_set(const char* key, const char* val)`: 更新数据，触发前端刷新。
- `const char* wce_version(void)`: 获取版本字符串。
- `int wce_is_connected(void)`: 是否有前端连接。

## 生成代码接口（可选）

当你使用 `.wce` 并把生成的 `webcee_generated.c/.h` 参与编译时，生成代码通常会提供以下入口（用于模型更新、事件分发、列表数据等）：

- `void wce_handle_model_update(const char* key, const char* val)`
- `void wce_dispatch_event(const char* event, const char* args)`
- `char* wce_get_list_json(const char* name)`

## JavaScript 接口

虽然通常不需要直接编写 JS，但 WebCee 暴露了以下 API 供高级用户使用：

- `GET /api/data`: 返回 KV 的 JSON 快照。
- `POST /api/update?key=K&val=V`: 触发模型更新（进入 `wce_handle_model_update`）。
- `POST /api/trigger?event=E&arg=A`: 触发事件（进入 `wce_dispatch_event`）。
- `GET /api/list?name=N`: 获取列表 JSON（来自 `wce_get_list_json`）。

说明：这些接口使用 query string 传参，实际项目建议对参数做 URL 编码/解码处理。

