# WebCee (Web for C Embedded)

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.1-green.svg)

**WebCee** 是一个专为嵌入式工程师设计的 Web UI 框架：用接近 C 代码块的“函数作用域”写法定义 UI，并由构建步骤生成网页资源，设备端用极简 HTTP 服务对外提供页面与 API。

## 🚀 核心特性

- **函数作用域 UI DSL**: 在 `.wce` 里编写 `wce_container({ ... });` / `wce_row({ ... });` 等结构化语句，生成 HTML/CSS/JS。
- **高性能网络核心**: 基于 **Reactor 模式 (epoll/select)** 的 v0.1 核心，单线程处理高并发。
- **数据接口**: 内置 `GET /api/data` 返回当前 Key-Value 快照（可用于前端轮询展示）。
- **零依赖**: 仅依赖标准 C 库和系统 Socket API，适合嵌入式 Linux 和 RTOS。

## 📚 文档

详细文档请查阅 [docs/](docs/) 目录：

- [快速上手 (Quick Start)](docs/getting_started.md)
- [核心概念 (Core Concepts)](docs/core_concepts.md)
- [技术架构 (Architecture)](docs/architecture.md)
- [API 参考 (API Reference)](docs/api_reference.md)
- [集成指南 (Integration Guide)](docs/integration.md)

## 🛠️ 快速演示

构建依赖：**零依赖**。
- **推荐**：安装 **Python 3**，使用功能最完整的 Python 解析器。
- **回退**：如果未检测到 Python，构建系统会自动编译并使用内置的 C 解析器（对用户透明）。

Windows（PowerShell）最短路径：

```powershell
# 1) 克隆
git clone <你的仓库地址>
cd WebCee

# 2) 创建一个新工程（在上级目录生成 MyApp）
.\create_project.bat ..\MyApp
cd ..\MyApp

# 3) 生成并运行（先编译，再运行）
gcc -g -std=c11 -I .\lib .\main.c .\lib\webcee.c -o .\main.exe -lws2_32
.\main.exe
```

浏览器打开：`http://localhost:8080`

停止服务：在运行 `main.exe` 的终端按 `Ctrl+C`（或如果程序在等待输入，直接按回车结束）。

## 📦 集成 (Integration)

WebCee 提供了极简的 CMake 集成支持。只需一行代码即可将 UI 绑定到您的固件：

```cmake
add_subdirectory(WebCee)
target_add_webcee_ui(firmware ui/main.wce)
```

详细集成指南请参考 [docs/integration.md](docs/integration.md)。

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源。
