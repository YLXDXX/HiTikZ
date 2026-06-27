# HiTikZ — TikZ 代码合集管理器

> 面向 Linux (KDE 6 / Wayland) 的 TikZ/PGF 矢量图形管理工具。
> 创建、编辑、预览、搜索、导出 TikZ 图像的专用 IDE。

---

## 目录

- [功能概览](#功能概览)
- [系统要求](#系统要求)
- [快速开始](#快速开始)
- [用户界面](#用户界面)
- [核心功能说明](#核心功能说明)
  - [片段管理](#片段管理)
  - [分类系统](#分类系统)
  - [模糊搜索](#模糊搜索)
  - [LaTeX 编译与 PDF 预览](#latex-编译与-pdf-预览)
  - [参数化系统](#参数化系统)
  - [模板系统](#模板系统)
  - [剪贴板操作](#剪贴板操作)
  - [导入与导出](#导入与导出)
- [键盘快捷键](#键盘快捷键)
- [预置片段清单](#预置片段清单)
- [数据存储](#数据存储)
- [设置面板](#设置面板)
- [命令行构建](#命令行构建)
- [项目架构](#项目架构)
- [测试](#测试)
- [技术栈](#技术栈)

---

## 功能概览

- **Ti*k*Z 片段管理**：创建、编辑、保存、删除 TikZ 代码片段，附带名称 / 简介 / 分类 / 标签元数据
- **实时编译预览**：调用系统 `xelatex` 编译片段，通过 `QPdfView` 实时渲染高清 PDF 矢量预览
- **适应式缩放**：支持适应整页 / 适应宽度 / 适应高度三种显示模式，滚轮缩放以鼠标位置为中心，左键拖拽平移
- **预览持久化**：编译成功后自动保存 PDF 和缩略图 PNG，下次切换片段即时展示预览
- **批量预览生成**：一键生成所有片段的预览图
- **彩色日志面板**：错误行红色高亮、警告橙色，双击错误行跳转到编辑器对应行
- **模糊搜索**：UTF-8 子序列匹配，支持名称和简介搜索，连续匹配高分、间隔扣分
- **树形分类**：支持层级分类（如 `数学/几何`），显示每个分类的片段数量，拖拽片段到分类节点即可重新分类
- **右键属性编辑对话框**：右键缩略图弹出对话框，可编辑名称 / 简介 / 分类 / 标签 / 模板，支持删除 / 导出 / 复制代码
- **参数化功能**：通过 `% @param: var=默认值` 声明变量，代码中使用 `@@var@@` 占位，右栏动态生成参数控件
- **模板系统**：三个内置 LaTeX 模板（数学 / 物理 / 电路），用户可自定义，通过占位符 `%%% TIKZ_CODE_HERE %%%` 注入核心代码
- **格式导出**：编译生成的 PDF 可通过 `pdftocairo` 转换并复制为 PNG/SVG 到剪贴板
- **存档导入/导出**：片段以 tar.gz 格式打包，方便分享和备份
- **系统托盘**：最小化到托盘，支持 `Ctrl+Alt+T` 全局快捷键一键呼出/隐藏
- **依赖检测**：启动时检测 `xelatex` 和 `pdftocairo`，缺失时弹出安装指引
- **14 个预置片段**：涵盖数学（几何 / 函数 / 向量 / 曲线）、物理（力学 / 电路）常用图形
- **C++17 + Qt6 原生实现**：高性能，启动快，原生 Wayland 支持

---

## 系统要求

| 依赖 | 版本 / 说明 |
|------|------------|
| **操作系统** | Linux（主力支持 KDE 6 / Wayland） |
| **C++ 编译器** | GCC 11+ 或 Clang 14+（C++17） |
| **CMake** | 3.16+ |
| **Qt6** | Widgets、Pdf、PdfWidgets 模块 |
| **QHotkey** | 通过 CMake FetchContent 自动拉取（可选，用于全局快捷键） |
| **XeLaTeX** | TeX Live 2023+（或等效发行版） |
| **pdf2cairo** | poppler 工具集（`pdftocairo` 命令） |
| **tar** | GNU tar（用于导入/导出存档） |
| **unzip** | （可选）导入 .zip 格式存档时的回退工具 |

### 安装依赖（Arch / Manjaro）

```bash
sudo pacman -S cmake gcc qt6-base qt6-webengine poppler texlive-core
```

### 安装依赖（Debian / Ubuntu）

```bash
sudo apt install cmake g++ qt6-base-dev libqt6pdf6 libqt6pdfwidgets6 \
    qt6-pdf-dev qt6-pdfwidgets-dev texlive-xetex poppler-utils
```

### 安装依赖（Fedora）

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtpdf-devel \
    texlive-xetex poppler-utils
```

---

## 快速开始

```bash
# 克隆项目
git clone <repo-url> HiTikZ
cd HiTikZ

# 构建（默认启用 QHotkey 全局快捷键）
mkdir build && cd build
cmake .. -DWITH_QHOTKEY=ON
cmake --build . -j$(nproc)

# 运行
./TikzManager

# 运行测试
ctest --output-on-failure
```

如需禁用全局快捷键支持（在某些桌面环境下可能不工作）：

```bash
cmake .. -DWITH_QHOTKEY=OFF
```

---

## 用户界面

程序主窗口采用经典三栏布局，三栏宽度可通过鼠标拖动调整：

```
┌──────────────────┬──────────────────────────┬────────────────────────┐
│   左栏 (导航)     │     中栏 (编辑)           │     右栏 (属性与预览)   │
│                  │                          │                        │
│ [搜索框]          │  ┌────────────────────┐  │ [适应整页][适应宽度]..  │
│                  │  │                    │  │ ┌────────────────────┐ │
│ ▼ 全部 (20)      │  │   代码编辑器         │  │ │                    │ │
│   ▼ 数学         │  │   (等宽、带行号)     │  │ │   PDF 预览         │ │
│     ● 几何 (3)   │  │                    │  │ │   滚轮缩放          │ │
│     ● 函数 (1)   │  │                    │  │ │   左键拖拽          │ │
│     ● 向量 (1)   │  │                    │  │ └────────────────────┘ │
│     ● 曲线 (1)   │  │                    │  │ 名称: [_____________]  │
│   ▼ 物理         │  └────────────────────┘  │ 简介: [_____________]  │
│     ● 力学 (4)   │  ┌────────────────────┐  │ 标签: [_____________]  │
│     ● 电路 (1)   │  │  编译日志 (可收起)   │  │ 模板: [下拉菜单____]   │
│                  │  └────────────────────┘  │ 参数: [动态控件区]     │
│ ┌──┐ ┌──┐ ┌──┐  │                          │                        │
│ │缩│ │缩│ │缩│  │                          │ [编译] [应用参数] [保存]│
│ │略│ │略│ │略│  │                          │                        │
│ └──┘ └──┘ └──┘  │                          │                        │
└──────────────────┴──────────────────────────┴────────────────────────┘
```

### 左栏

- **搜索框**：输入关键词实时搜索（150ms 防抖延迟），按名称 + 简介模糊匹配
- **分类树**：层级分类导航，节点显示片段数量，点击分类过滤缩略图列表
  - 右键分类节点：重命名 / 删除 / 新建子分类
  - 右键"全部"：新建顶级分类
- **缩略图列表**：网格视图展示搜索结果 / 分类筛选结果
  - 左键点击：选中片段并加载到编辑器
  - 右键点击：弹出属性编辑对话框
  - 拖拽到分类节点：移动片段到目标分类

### 中栏

- **代码编辑器**：基于 `QPlainTextEdit`，等宽字体、行号显示、当前行高亮、Tab=4 空格
- **编译日志**：显示 xelatex 的编译输出，红色 = 错误、橙色 = 警告、灰色 = 信息
  - 双击日志行中的 `l.<行号>` 跳转编辑器对应行

### 右栏

- **PDF 预览**：Qt6 `QPdfView` 矢量渲染，支持：
  - 适应整页 / 适应宽度 / 适应高度 三种自动模式
  - +/- 按钮缩放
  - 鼠标滚轮缩放（以光标位置为中心）
  - 按住左键拖拽平移
  - 选中模式按钮高亮反馈
- **元数据**：名称、简介、标签（逗号分隔）、模板选择
- **参数控件**：自动识别 `% @param:` 注释，动态生成输入框
- **操作按钮**：编译预览 / 应用参数 / 保存（水平排列）

---

## 核心功能说明

### 片段管理

每个 TikZ 片段是一个独立目录，包含三个核心文件：

```
~/.local/share/TikzManager/
├── snippets/               # 用户创建的片段
│   └── <uuid>/
│       ├── meta.json       # 名称、简介、分类、标签、模板ID
│       ├── snippet.tex     # \begin{tikzpicture}...\end{tikzpicture} 核心代码
│       └── preview.png     # 最后一次编译成功的缩略图
├── presets/                # 系统预置片段（首次启动从 resources/presets/ 拷贝）
│   └── <uuid>/ ...
└── templates/              # LaTeX 模板（首次启动从 resources/templates/ 拷贝）
    ├── default_math.tex
    ├── default_physics.tex
    └── default_circuit.tex
```

**meta.json 格式示例**：

```json
{
  "id": "213e8fc6-ada0-4038-9d04-2d660e1610fb",
  "name": "电路基础元件",
  "description": "电阻、电容、电感符号",
  "category": "物理/电路",
  "tags": ["电路", "元件"],
  "templateId": "default_circuit"
}
```

### 分类系统

- 支持层级分类，使用 `/` 分隔（如 `数学/几何`）
- 创建新片段时可指定分类，或通过新建子分类创建占位片段
- 拖拽缩略图到分类树节点即可重新归类
- 分类树节点显示片段数量（含子分类汇总数）

### 模糊搜索

- 基于 UTF-8 子序列匹配，不依赖拼音转换
- 打分规则：连续匹配字符得高分（`10 + consecutive * 5`），间隔匹配低分
- 名称匹配权重是简介的 2 倍
- 空搜索显示所有片段，按预设/用户分组

### LaTeX 编译与 PDF 预览

**编译流程**：

1. 将 TikZ 核心代码注入选中模板的 `%%% TIKZ_CODE_HERE %%%` 位置（如无模板则使用内置默认模板）
2. 写入临时 `.tex` 文件（位于 `/tmp/TikzManager/<snippetId>/output.tex`）
3. 异步调用 `xelatex -interaction=nonstopmode -halt-on-error -no-shell-escape`
4. 编译成功 → PDF 加载到预览区 → 生成缩略图 PNG（150 DPI）→ 持久化到片段目录
5. 编译失败 → 日志面板显示错误 → 双击跳转错误行

**预览交互**：

| 操作 | 行为 |
|------|------|
| 适应整页按钮 | 缩放至 PDF 完整可见 |
| 适应宽度按钮 | 缩放至宽度恰好填满视口 |
| 适应高度按钮 | 缩放至高度恰好填满视口（基于 `logicalDpiY` 计算） |
| +/- 按钮 / 鼠标滚轮 | 以光标位置为中心缩放 +/- 25% |
| 按住左键拖拽 | 平移视口 |
| 状态记忆 | 模式选择跨编译/切换保持；滚轮缩放不改变模式偏好 |

### 参数化系统

在 TikZ 代码中使用 `% @param:` 注释声明参数，`@@var@@` 作为占位符：

```latex
% @param: radius=2
% @param: color=red
\begin{tikzpicture}
  \draw[@@color@@] (0,0) circle (@@radius@@);
\end{tikzpicture}
```

- **自动解析**：代码编辑时实时扫描 `% @param:` 行，动态生成 `变量名: [默认值]` 输入框
- **应用参数**：点击"应用参数"按钮，将 `@@var@@` 替换为输入框中的当前值后触发编译
- **批量预览**：生成所有预览时自动使用默认值替换参数
- **复制代码**：`Ctrl+Shift+C` 复制的代码为参数替换后的最终代码（注释行已移除）

### 模板系统

三个内置 LaTeX 模板（可通过设置面板编辑或新增）：

| 模板 | 用途 | 特殊宏包 |
|------|------|---------|
| `default_math` | 数学图形 | `amsmath`, `ctex`, 常用 Ti*k*Z 库 |
| `default_physics` | 物理示意图 | `amssymb`, `arrows.meta`, `angles`, `quotes` |
| `default_circuit` | 电路图 | `circuitikz`, `ctex`，`preview` 裁剪环境 |

所有模板均使用 `standalone` 文档类 + `tikz` 选项，生成紧凑的独立 PDF。

模板文件位于 `~/.local/share/TikzManager/templates/`，可通过设置面板的模板管理界面创建、编辑、删除。

如无选中模板，程序使用内置默认模板：
```latex
\documentclass[tikz, border=5pt]{standalone}
\usepackage{tikz}\usepackage{xcolor}\usepackage{ctex}
\usetikzlibrary{calc,shapes,arrows,positioning,patterns}
\begin{document}
...
\end{document}
```

### 剪贴板操作

| 快捷键 | 操作 | 说明 |
|--------|------|------|
| `Ctrl+Shift+C` | 复制代码 | 复制参数替换后的 TikZ 核心代码 |
| `Ctrl+Shift+P` | 复制 PNG | 从 PDF 转换 300 DPI PNG 后复制到剪贴板 |
| `Ctrl+Shift+S` | 复制 SVG | 从 PDF 转换 SVG 后复制（附带 `image/svg+xml` MIME 类型） |

### 导入与导出

**导出**：选中片段 → 工具栏"导出存档"（或右键→导出）→ 保存为 `.tar.gz` 文件。
包含片段目录中的全部文件（`meta.json`、`snippet.tex`、`preview.png`）。

**导入**：工具栏"导入存档" → 选择 `.tar.gz` 或 `.zip` 文件 → 自动解压并为每个片段分配新 UUID（同时更新 `meta.json` 中的 id）→ 刷新列表。

---

## 键盘快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+Alt+T` | 全局快捷键：显示/隐藏窗口（依赖 QHotkey） |
| `Ctrl+Shift+C` | 复制 TikZ 代码到剪贴板 |
| `Ctrl+Shift+P` | 复制 PNG 图片到剪贴板 |
| `Ctrl+Shift+S` | 复制 SVG 矢量图到剪贴板 |
| `Esc` | 隐藏窗口到系统托盘 |
| `Ctrl+Z` | 代码编辑器原生撤销 |

---

## 预置片段清单

程序首次启动时自动安装 14 个教学/常用预置片段：

| 分类 | 片段名称 | 简介 |
|------|---------|------|
| 数学/基础 | 箭头样式 | 各种箭头样式展示 |
| 数学/几何 | 三角形 | 基本三角形图形，带顶点标注 |
| 数学/几何 | 矩形 | 带阴影的矩形，展示基本填充效果 |
| 数学/几何 | 椭圆 | 椭圆与焦点标注 |
| 数学/函数 | 基础函数曲线 | y=x² 和 y=sin(x) 函数图 |
| 数学/向量 | 向量加减法 | 向量加法平行四边形法则 |
| 数学/曲线 | 贝塞尔曲线 | 三次贝塞尔曲线与控制点 |
| 数学/坐标系 | 坐标轴 | 标准 x-y 坐标轴框架 |
| 数学/坐标系 | 坐标格纸 | 实验用坐标格纸 (mm 方格) |
| 物理/力学 | 单摆 | 单摆运动示意图 |
| 物理/力学 | 地面与斜面 | 带摩擦系数的斜面与物体 |
| 物理/力学 | 墙面 (斜线填充) | 带斜线填充的墙面示意图 |
| 物理/力学 | 弹簧 | 带参数的弹簧示意图，展示 `@@` 参数替换 |
| 物理/电路 | 电路基础元件 | 电阻、电容、电感符号 |

---

## 数据存储

所有用户数据存储在 `~/.local/share/TikzManager/`：

```
~/.local/share/TikzManager/
├── snippets/               # 用户创建/导入的片段
│   └── <uuid>/
│       ├── meta.json
│       ├── snippet.tex
│       └── preview.png
├── presets/                # 系统预置（首次运行拷贝自 resources/presets/）
│   └── <uuid>/ ...
└── templates/              # 用户自定义模板（首次运行拷贝自 resources/templates/）
    └── *.tex
```

程序配置通过 `QSettings` 存储（键值对），包括：
- `xelatex/path`
- `pdftocairo/path`
- `paths/texinputs`
- `png/dpi`

---

## 设置面板

菜单栏 → "设置" 打开设置对话框，包含：

**路径设置**：
- xelatex 路径（默认 `xelatex`，从 `$PATH` 查找）
- pdftocairo 路径（默认 `pdftocairo`）
- TEXINPUTS 环境变量（扩展 LaTeX 搜索路径）
- PNG DPI（72–1200，默认 300）

**模板管理**：
- 左侧列表展示所有 `.tex` 模板
- 右侧代码编辑区可编辑选中模板
- +/- 按钮创建 / 删除模板
- 模板中必须包含 `%%% TIKZ_CODE_HERE %%%` 占位符

**重置出厂**：工具栏"重置出厂"按钮将清空所有用户数据并恢复预置片段和模板。

---

## 命令行构建

```bash
# 基本构建
cmake -B build -DWITH_QHOTKEY=ON
cmake --build build -j$(nproc)

# 运行
./build/TikzManager

# 运行测试
cd build && ctest --output-on-failure

# 安装（可选）
cmake --install build --prefix /usr/local
```

CMake 选项：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `WITH_QHOTKEY` | ON | 启用全局快捷键（`Ctrl+Alt+T`） |

构建产物：
- `build/TikzManager` — 主程序
- `build/test_snippet_manager` — 片段管理测试
- `build/test_latex_compiler` — LaTeX 编译测试
- `build/test_search` — 搜索算法测试

---

## 项目架构

```
src/
├── main.cpp                         # 入口（QApplication 初始化）
├── mainwindow.h / mainwindow.cpp    # 主窗口（UI 编排、信号槽、业务逻辑）
├── search_panel.h / search_panel.cpp # 左栏组件（搜索框、分类树、缩略图）
├── snippet_manager.h / .cpp         # 数据模型与文件系统 CRUD
├── latex_compiler.h / .cpp          # xelatex 编译 + pdftocairo 转换
├── code_editor.h / .cpp             # 带行号的代码编辑器
├── settings_dialog.h / .cpp         # 设置面板与模板管理
├── snippet_properties_dialog.h / .cpp # 右键属性编辑对话框
resources/
├── templates/                       # 出厂模板（math / physics / circuit）
│   ├── default_math.tex
│   ├── default_physics.tex
│   └── default_circuit.tex
└── presets/                         # 出厂预置片段（14 个）
    └── <uuid>/
        ├── meta.json
        └── snippet.tex
tests/
├── test_snippet_manager.cpp         # CRUD 操作 + ZIP 导入/导出测试
├── test_latex_compiler.cpp          # 编译 + PNG/SVG 转换测试
└── test_search.cpp                  # 模糊搜索算法 + 分类测试
```

### 核心类关系

```
MainWindow
├── SearchPanel          ← SnippetManager
│   ├── QLineEdit        搜索框
│   ├── QTreeView        分类树
│   └── QListView        缩略图网格
├── CodeEditor           代码编辑器（带行号）
├── QPlainTextEdit       编译日志面板
├── QPdfView             PDF 矢量预览（右栏顶部）
├── LatexCompiler        编译引擎（xelatex + pdftocairo）
├── SnippetManager       数据层（JSON 读写、搜索、分类）
└── SettingsDialog       设置面板
```

### 信号关键路径

```
搜索/分类变化 → SearchPanel::snippetSelected(id)
              → MainWindow::loadSnippetIntoEditor(id)

编译按钮 → saveCurrentSnippet() → compiler->compile()
         → compilationFinished(success, pdf, log)
         → QPdfView 加载 PDF + savePreviewData() 异步生成缩略图

右键缩略图 → SnippetPropertiesDialog(exec)
           → 保存/删除 → SearchPanel::refreshSearch/refreshCategoryTree
```

---

## 测试

项目包含三套自动化测试（通过 CTest 运行）：

| 测试 | 内容 |
|------|------|
| `test_snippet_manager` | 片段创建/读取/更新/删除，loadCode 参数，renameCategory 代码保持，ZIP 导入/导出往返 |
| `test_latex_compiler` | xelatex 可用性检测，基本编译，PDF 生成，PNG 转换，SVG 转换，错误编译日志验证 |
| `test_search` | 精确匹配、子序列匹配、连续加分、无匹配、大小写不敏感、中文搜索、分类统计 |

运行测试：
```bash
cd build && ctest --output-on-failure -V
```

---

## 技术栈

| 技术 | 用途 |
|------|------|
| **C++17** | 核心语言 |
| **Qt 6** | Widgets (UI), Pdf (PDF 渲染), PdfWidgets (QPdfView) |
| **CMake** | 构建系统 |
| **QHotkey** (optional) | 全局快捷键 |
| **XeLaTeX** | LaTeX → PDF 编译 |
| **pdftocairo** | PDF → PNG / SVG 转换 |
| **JSON** | 片段元数据格式 |
| **tar / unzip** | 存档导入/导出 |

---

## 局限性

- 暂不内置 LaTeX 语法高亮和自动补全（TODO）
- 防抖自动编译未启用，需手动点击"编译预览"（TODO）
- 仅 Linux 平台支持（核心依赖 xelatex / Qt6 / Wayland 均为跨平台，移植可行）
- 预设片段可被用户编辑/删除（按需设计，非只读保护）
- 导入支持 `.tar.gz` 和 `.zip`，导出仅支持 `.tar.gz`

---

## 贡献

欢迎提交 Issue 和 Pull Request。开发环境建议：

```bash
# 安装构建依赖
sudo pacman -S cmake gcc qt6-base qt6-webengine poppler texlive-core

# 编辑代码后
cd build
cmake --build . -j$(nproc)
ctest --output-on-failure
```
