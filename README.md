# Wordle - C++ 控制台图形版

一个使用 **C++** 和 **EasyX** 图形库开发的 Wordle 猜词游戏，支持经典玩法、困难模式、提示系统和数据统计。

## 🎮 游戏截图

> （运行后截图可放在此处）

## ✨ 功能特性

- **经典 Wordle 玩法** — 6 次机会猜出 5 字母单词
- **困难模式** — 已猜对的字母必须在后续猜测中使用
- **键盘状态提示** — 虚拟键盘实时显示字母状态（正确 / 存在 / 不存在）
- **提示系统** — 卡住时可以使用提示
- **数据统计** — 记录游戏次数、胜率、连胜、猜测分布
- **帮助页面** — 内置游戏规则说明

## 🛠️ 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++20 |
| 构建工具 | CMake 4.0+ |
| 图形库 | [EasyX](https://easyx.cn/) |
| IDE | CLion / Visual Studio |

## 📦 依赖

- **EasyX** — 需要在项目中包含 `graphics.h` 头文件和 `libeasyx.a` 静态库，已放在 `include/` 和 `lib/` 目录下。也可从 [easyx.cn](https://easyx.cn/) 下载最新版。

## 🔧 编译与运行

### 使用 CLion

1. 用 CLion 打开项目根目录
2. 确保已配置 MinGW/GCC 工具链
3. 点击 **Build** → **Build Project**
4. 运行 `untitled` 目标

### 使用命令行

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./untitled
```

> **注意**：`words.txt` 需要与可执行文件在同一目录下，CMake 已配置自动复制。

## 🎯 游戏规则

1. 程序随机选择一个 5 字母单词作为目标
2. 玩家有 **6 次**猜测机会
3. 每次猜测后，字母方块会变色：
   - 🟩 **绿色** — 字母正确且位置正确
   - 🟨 **黄色** — 字母存在于单词中但位置不对
   - ⬜ **灰色** — 字母不在单词中
4. 困难模式下，已揭示的字母必须在后续猜测中使用

## 📁 项目结构

```
untitled/
├── main.cpp          # 主程序源码
├── CMakeLists.txt    # CMake 构建配置
├── words.txt         # 单词列表
├── include/          # EasyX 头文件
│   ├── graphics.h
│   └── easyx.h
├── lib/              # EasyX 静态库
│   ├── libeasyx.a
│   └── libeasyxw.a
└── .gitignore
```

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源。

## 🙏 致谢

- [EasyX](https://easyx.cn/) — 简单易用的 Windows 图形库
- 灵感来源于 [Wordle](https://www.nytimes.com/games/wordle/index.html) 原版游戏
