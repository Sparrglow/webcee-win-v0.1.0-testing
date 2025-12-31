# 集成指南 (Integration Guide)

本指南只描述 v0.1 的对外集成方式，并以“终端命令可复制”为主。

WebCee 有两条常用路径：

1) **只用运行时（不使用 `.wce`）**：你手写前端资源（或只用内置兜底页面），C 侧只用 `wce_*` 运行时 API。
2) **使用 `.wce`（推荐）**：构建阶段解析 `.wce`，生成 `web_root/` 与可选的 `webcee_generated.c/.h`。

解析器选择：优先 Python 解析器；没有 Python 会回退到 C 解析器（对用户透明）。

## 方案 A：手动编译（最直接）

### Windows（MinGW-w64 gcc）

```powershell
gcc -g -std=c11 main.c src\webcee.c -Iinclude -o app.exe -lws2_32
./app.exe
```

### Linux

```bash
gcc -g -std=c11 main.c src/webcee.c -Iinclude -o app -lpthread
./app
```

## 方案 B：用脚手架创建工程（推荐给新项目）

在 WebCee 仓库根目录运行：

```powershell
create_project.bat ..\my_app
cd ..\my_app
\build.bat
```

脚手架会把运行时源码、头文件、解析器和构建脚本复制到你的工程目录里。

## 方案 C：CMake 集成（适合现有项目）

假设你的项目结构：

```
MyProject/
├── CMakeLists.txt
├── main.c
├── ui/
│   └── main.wce
└── WebCee/   (作为子目录或 submodule)
```

### 1) 修改 `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyWebDevice)

add_subdirectory(WebCee)
add_executable(firmware main.c)

# 绑定 UI：构建时生成 web_root/ + webcee_generated.c/.h，并自动部署资源
target_add_webcee_ui(firmware ui/main.wce)
```

### 2) 终端构建与运行

```bash
cmake -S . -B build
cmake --build build
```

运行时从可执行文件目录启动：

```bash
./build/firmware
```

说明：`target_add_webcee_ui()` 会把 `web_root/` 复制到可执行文件旁，运行时即可通过 `GET /` 访问页面。
