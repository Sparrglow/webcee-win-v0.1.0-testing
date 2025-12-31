# 技术架构 (Technical Architecture)

WebCee v0.1 由两部分组成：**构建期生成**（把 `.wce` 变成可运行资源）与 **运行时服务**（一个最小 HTTP 服务器 + 数据/事件接口）。

## 1) 构建期：从 `.wce` 到可执行资源

构建时会运行解析器（优先 Python，缺失时回退到 C 解析器），产出两类结果：

- `web_root/`：`index.html` / `style.css` / `app.js` 等静态资源
- `webcee_generated.c/.h`（可选）：用于把“模型更新/事件分发”等逻辑连接到你的 C 代码

## 2) 运行时：HTTP 服务器

运行时库在 `src/webcee.c` 中实现：

- 提供静态资源：从 `web_root/`、`generated/web_root/` 等目录搜索并返回文件
- 如果找不到静态资源：返回内置的兜底页面（页面会轮询 `/api/data`）

## 3) 数据与事件接口

运行时暴露的接口包括：

- `GET /api/data`：返回 KV 数据的 JSON 快照
- `POST /api/update?key=K&val=V`：把模型更新传回 C（由 `wce_handle_model_update()` 处理）
- `POST /api/trigger?event=E&arg=A`：触发事件（由 `wce_dispatch_event()` 处理）
- `GET /api/list?name=LIST`：请求列表数据（由 `wce_get_list_json()` 提供 JSON）

## 4) 平台差异

- Windows：使用 `select()`
- Linux：优先使用 `epoll`（在 `__linux__` 下启用）

目标是保持 API 与行为一致，平台差异只在 IO 多路复用实现层。
