# WebCee

A lightweight, C-based web UI framework for embedded systems.

## Features

- **Pure C Compiler**: No Python or Node.js dependencies.
- **Declarative UI**: Define UI in `.wce` files with a C-like syntax.
- **Nested Layouts**: Support for complex nested rows, columns, cards, and panels.
- **Data Binding**: Simple two-way data binding between C and UI.
- **Embedded Server**: Built-in HTTP server for serving the UI.

## Quick Start

### 1. Create a New Project

Use the included script to generate a ready-to-run project template:

```bat
.\create_project.bat MyNewApp
```

This will create a folder `MyNewApp` with:
- `ui.wce`: The UI layout definition.
- `main.c`: The application logic.
- `build.bat`: A script to compile and run your app.

### 2. Run Your App

```bat
cd MyNewApp
.\build.bat
.\app.exe
```

Open your browser at `http://localhost:8080`.
To stop the server, press **Enter** in the console window.

## Examples

### Showcase Demo

The `examples/showcase` project demonstrates all core features of WebCee, including:
- **Layouts**: Rows, columns, cards, and panels.
- **Interactivity**: Buttons with C callback functions.
- **Data Binding**: Real-time text updates and input field binding.
- **Styling**: Custom CSS injection.

**To build and run the showcase:**

1. Build the project (compiles the framework and the showcase):
   ```bat
   tools\build_project.bat
   ```

2. Run the showcase executable:
   ```bat
   examples\showcase\showcase.exe
   ```

3. Open `http://localhost:8080` to interact with the demo.

## Project Structure

- `compiler/`: Source code for the `wce` compiler (converts `.wce` to C).
- `include/`: Header files (`webcee.h`, `webcee_build.h`).
- `src/`: Runtime library source (`webcee.c`).
- `tools/`: Build scripts and the compiled `wce.exe`.
- `examples/`: Example projects.

## License

MIT
