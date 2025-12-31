# 快速上手 (Quick Start)

这份文档只展示 WebCee v0.1 的“函数作用域”写法，并以“复制命令 → 能跑起来”为目标。

## 0) 你需要准备什么

- **C 编译器**：
  - Windows：建议 MinGW-w64（`gcc` 可用）
  - Linux：`gcc` / `clang`
- **Python 3（可选）**：用于更快的 `.wce` 解析；没有 Python 也能工作（会回退到 C 解析器）。

## 1) 获取源码

```bash
git clone <你的仓库地址>
cd WebCee
```

## 2) 用脚手架创建一个最小工程

### Windows（PowerShell 或 cmd）

```powershell
cd C:\path\to\WebCee
.
create_project.bat ..\my_app
cd ..\my_app
.
\build.bat
```

成功后会自动运行程序，并在控制台看到类似：`Running at http://localhost:8080`。

### Linux/macOS（bash）

```bash
cd /path/to/WebCee
./create_project.sh ../my_app
cd ../my_app
./build.sh
```

## 3) 写一个函数作用域 UI（可选）

在你的工程目录里创建 `ui/main.wce`（路径固定为 `ui/` 只是默认约定，脚本会扫描 `ui/*.wce`）：

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

然后重新运行构建（会自动解析 `.wce` 并生成 `web_root/` 与 `webcee_generated.c/.h`）：

```powershell
\build.bat
```

## 4) 停止程序

- 如果你的 `main.c` 使用 `getchar()` 等待退出：在控制台按 **Enter**。
- 如果你是一直跑的服务进程：按 **Ctrl+C**。

## 下一步

- 需要把 WebCee 集成进自己的工程：看 [集成指南 (Integration Guide)](integration.md)
- 想理解数据同步与事件：看 [核心概念 (Core Concepts)](core_concepts.md)
- 想查所有可用 `wce_*`：看 [API 参考 (API Reference)](api_reference.md)
