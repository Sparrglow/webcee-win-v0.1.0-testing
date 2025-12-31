# 函数作用域语法指南 (Function-scope Syntax)

WebCee v0.1 对外只推荐“函数作用域”写法：用 `wce_*({ ... });` 形成清晰的层级结构。

## 1) 基本结构

容器组件使用“作用域块”包裹子节点：

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

## 2) 规则与常见坑

- `wce_container / wce_row / wce_col / wce_card / wce_panel` 是容器：参数应当是一个 `{ ... }` 块（内部写子节点）。
- `wce_text / wce_button / wce_slider / wce_progress / wce_input` 是叶子节点：作为语句直接调用。
- v0.1 的 `wce_card(...)` 只做“卡片容器”，不要给它传标题等额外参数（以 `include/webcee_build.h` 的宏实现为准）。

## 3) 迁移（不展示旧语法）

如果你有历史代码需要迁移，可以使用升级脚本把旧写法批量转换成函数作用域写法：

```bash
python tools/upgrade_assistant.py <你的文件路径>
```

转换后建议直接编译运行验证（Windows 脚手架工程）：

```powershell
\build.bat
```
