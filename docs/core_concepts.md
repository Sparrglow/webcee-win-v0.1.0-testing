# 核心概念 (Core Concepts)

本页面向 WebCee v0.1，对外只描述“函数作用域 UI DSL”和运行时的数据/事件模型。

## 1) UI：函数作用域 DSL

你在 `.wce` 文件里用 `wce_*` 语句描述 UI 的结构（容器嵌套 + 叶子节点）。构建阶段解析 `.wce`，生成 `web_root/` 静态资源，并可选生成 `webcee_generated.c/.h`。

一个最小示例：

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

可用的基础组件以 [API 参考 (API Reference)](api_reference.md) 为准（以 `include/webcee_build.h` 为真实实现）。

## 2) 数据：C → 浏览器

WebCee 内置一个 KV 存储。你在 C 侧调用：

```c
wce_data_set("cpu_temp", "45.5");
```

浏览器通过 `GET /api/data` 获取当前 KV 的 JSON 快照，并用于渲染/刷新页面。

## 3) 模型更新：浏览器 → C

当页面需要把某个输入值同步回 C 侧时，会调用：

- `POST /api/update?key=K&val=V`

接收更新的入口是函数：

```c
void wce_handle_model_update(const char* key, const char* val);
```

这个函数通常由生成代码提供；如果你不使用生成代码，也可以在你的工程里自己实现它。

## 4) 事件：按钮点击等

按钮点击会触发：

- `POST /api/trigger?event=E&arg=A`

运行时会调用：

```c
void wce_dispatch_event(const char* event, const char* args);
```

其中 `event` 通常来自 `wce_button("...", callback)` 的 `callback` 名称（生成器/前端会把它当作事件名传回）。

## 5) 静态资源与兜底页面

- 如果运行目录里存在 `web_root/`（或 `generated/web_root/` 等搜索路径），WebCee 会直接提供其中的静态文件。
- 如果找不到静态资源，会返回一个内置的兜底页面（页面里会轮询 `/api/data`）。
