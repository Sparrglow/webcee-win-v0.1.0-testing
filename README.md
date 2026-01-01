# WebCee

A lightweight, C-based web UI framework for embedded systems.

## Features

- **Pure C Compiler**: No Python or Node.js dependencies.
- **Declarative UI**: Define UI in `.wce` files with a C-like syntax.
- **Nested Layouts**: Support for complex nested rows, columns, cards, and panels.
- **Data Binding**: Simple two-way data binding between C and UI.
- **Embedded Server**: Built-in HTTP server for serving the UI.

## Getting Started

### Prerequisites

- GCC Compiler (MinGW on Windows, or standard GCC on Linux)

### Building the Showcase

1. Run the build script:
   ```bat
   tools\build_project.bat
   ```

2. Run the executable:
   ```bat
   examples\showcase\showcase.exe
   ```

3. Open your browser at `http://localhost:8080`.

## Project Structure

- `compiler/`: Source code for the `wce` compiler.
- `include/`: Header files (`webcee.h`, `webcee_build.h`).
- `src/`: Runtime library source (`webcee.c`).
- `tools/`: Build scripts and the compiled `wce.exe`.
- `examples/`: Example projects (see `showcase`).

## Usage

1. Create a `.wce` file for your UI layout.
2. Compile it using `tools\wce.exe input.wce output.c`.
3. Include the generated C file in your project.
4. Link against `src/webcee.c` and `ws2_32` (on Windows).

## License

MIT
