# HiTikZ — TikZ 代码合集管理器

> 面向 Linux (KDE 6 / Wayland) 的 TikZ/PGF 矢量图形管理工具。
> 创建、编辑、预览、搜索、导出 TikZ 图像并支持批量操作。

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
  - [宏包与 TikZ 库](#宏包与-tikz-库)
  - [完整文档复制](#完整文档复制)
  - [多选与批量操作](#多选与批量操作)
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

- **TikZ 片段管理**：创建、编辑、保存、删除 TikZ 代码片段，附带名称 / 简介 / 分类 / 标签 / 宏包 / TikZ 库 / 模板元数据
- **实时编译预览**：调用系统 `xelatex` 编译片段，通过 `QPdfView` 实时渲染高清 PDF 矢量预览
- **可拆分预览面板**：右侧 PDF 预览与元数据编辑区之间可拖动分割条，PDF 可放大至占满整个面板
- **适应式缩放**：支持适应整页 / 适应宽度 / 适应高度三种显示模式，滚轮缩放以鼠标位置为中心，左键拖拽平移
- **预览持久化**：编译成功后自动保存 PDF 和缩略图 PNG，下次切换片段即时展示预览
- **批量预览生成**：设置面板中一键生成所有片段预览（同步 pdftocairo 确保不丢图）
- **彩色日志面板**：错误行红色高亮、警告橙色，双击错误行跳转到编辑器对应行
- **模糊搜索**：UTF-8 子序列匹配，支持名称和简介搜索，连续匹配高分、间隔扣分
- **树形分类**：支持层级分类（如 `数学/几何`），显示每个分类的片段数量，拖拽片段到分类节点即可重新分类
- **多选批量操作**：Ctrl+点击多选缩略图，右键弹出批量导出 / 改分类 / 删除菜单，支持全选、导出全部
- **属性编辑对话框**：右键缩略图（单选时）弹出属性对话框，可编辑全部元数据字段
- **参数化功能**：通过 `% @param: var=默认值` 声明变量，代码中使用 `@@var@@` 占位，右栏动态生成参数控件
- **模板系统（极简）**：三个内置 LaTeX 模板仅含必要宏包（数学 / 物理 / 电路），额外宏包和 TikZ 库由每个片段自行声明
- **完整文档复制**：一键复制含模板头部 + 片段的完整可编译 LaTeX 文档
- **格式导出**：编译生成的 PDF 可通过 `pdftocairo` 转换并复制为 PNG/SVG 到剪贴板
- **存档导入/导出**：片段以 tar.gz 格式打包，支持单选 / 多选批量 / 全部导出
- **系统托盘**：最小化到托盘，全局快捷键一键呼出/隐藏
- **可配置快捷键**：全部快捷键可在设置面板中自定义键序列，支持清空禁用
- **全局快捷键（KDE）**：KDE 桌面通过 KGlobalAccel 注册系统级快捷键
- **代码字体调节**：设置面板中可调代码编辑区字体大小（8-48 pt）
- **依赖检测**：启动时检测 `xelatex` 和 `pdftocairo`，缺失时弹出安装指引
- **9 个预置片段**：数学（几何 / 函数）、物理（力学）、电路（RLC / 分压 / 运放）各 3 个典型示例
- **C++17 + Qt6 原生实现**：高性能，启动快，原生 Wayland 支持

---

## 系统要求

| 依赖 | 版本 / 说明 |
|------|------------|
| **操作系统** | Linux（主力支持 KDE 6 / Wayland） |
| **C++ 编译器** | GCC 11+ 或 Clang 14+（C++17） |
| **CMake** | 3.16+ |
| **Qt6** | Widgets、Pdf、PdfWidgets 模块 |
| **KF6GlobalAccel** | （可选）KDE 原生全局快捷键 |
| **QHotkey** | 通过 CMake FetchContent 自动拉取（KF6GlobalAccel 不可用时的回退） |
| **XeLaTeX** | TeX Live 2023+（或等效发行版） |
| **pdftocairo** | poppler 工具集 |
| **tar** | GNU tar（用于导入/导出存档） |
| **unzip** | （可选）导入 .zip 格式存档时的回退工具 |

### 安装依赖（Arch / Manjaro）

```bash
sudo pacman -S cmake gcc qt6-base qt6-webengine poppler texlive-core kf6-kglobalaccel
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

# 构建（自动检测 KF6GlobalAccel，不可用时回退 QHotkey）
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# 运行
./TikzManager

# 运行测试
ctest --output-on-failure
```

CMake 选项：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `WITH_KGLOBALACCEL` | ON | 启用 KDE KGlobalAccel 全局快捷键 |
| `WITH_QHOTKEY` | ON | KGlobalAccel 不可用时回退 QHotkey |

---

## 用户界面

程序采用经典三栏布局，全栏宽度可拖动调整，右侧 PDF 预览与元数据区可上下拖动分割条。

### 工具栏

按从左到右顺序：

| 按钮 | 功能 |
|------|------|
| 新建片段 | 弹出对话框输入名称和分类 |
| 删除片段 | 删除当前片段或分类 |
| 导入/导出 ▼ | 下拉菜单：导入存档 / 导出当前 / 导出全部 |
| 编译预览 | 编译 TikZ 代码并渲染 PDF |
| 应用参数 | 应用参数值后编译 |
| 保存 | 保存当前片段 |
| 复制代码 | 复制参数替换后的 TikZ 核心代码 |
| 复制完整文档 | 复制含模板头部的完整 LaTeX 文档 |
| 复制PNG | 复制 300 DPI PNG 到剪贴板 |
| 复制SVG | 复制 SVG 到剪贴板 |
| 适应整页/宽度/高度 | PDF 显示模式（可选中态） |
| 放大/缩小 | PDF 缩放 |
| 设置 | 打开设置面板 |

### 左栏

- **搜索框**：输入关键词实时搜索（150ms 防抖延迟），按名称 + 简介模糊匹配
- **分类树**：层级分类导航，节点显示片段数量，点击分类过滤缩略图列表
  - 右键分类节点：重命名 / 删除 / 新建子分类
  - 右键"全部"：新建顶级分类
- **缩略图列表**：网格视图展示搜索结果 / 分类筛选结果
  - 左键点击：选中片段并加载到编辑器
  - **Ctrl+点击**：多选（不触发编辑器加载）
  - 右键点击（单选）：弹出属性编辑对话框
  - 右键点击（多选）：弹出批量操作菜单（批量导出 / 修改分类 / 删除所选 / 全选 / 导出全部）
  - 拖拽到分类节点：移动片段到目标分类

### 中栏

- **代码编辑器**：基于 `QPlainTextEdit`，等宽字体（字号可调，默认 10pt）、行号显示、当前行高亮
- **编译日志**：显示 xelatex 的编译输出，红色 = 错误、橙色 = 警告、灰色 = 信息
  - 双击日志行中的 `l.<行号>` 跳转编辑器对应行

### 右栏

上部分为 **PDF 预览**，下部分为 **元数据编辑**（可拖拽分割条调整比例）：

- **PDF 预览**：Qt6 `QPdfView` 矢量渲染，支持工具栏缩放控制、鼠标滚轮缩放（以光标位置为中心）、左键拖拽平移
- **元数据表单**：名称、简介、标签、额外宏包、TikZ 库、模板选择
- **参数控件**：自动识别 `% @param:` 注释，动态生成输入框
- 右栏元数据区可在分割条拖到底时自动滚动

---

## 核心功能说明

### 片段管理

每个 TikZ 片段是一个独立目录，包含三个核心文件：

```
~/.local/share/TikzManager/
├── snippets/               # 用户创建的片段
│   └── <uuid>/
│       ├── meta.json       # 名称、简介、分类、标签、模板、宏包、TikZ 库
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
  "id": "10000000-0000-0000-0000-000000000001",
  "name": "勾股定理",
  "description": "勾股定理几何证明：直角三角形三边正方形面积关系",
  "category": "数学/几何",
  "tags": ["勾股定理", "几何", "三角形"],
  "templateId": "default_math",
  "packages": "",
  "tikzLibraries": ""
}
```

### 分类系统

- 支持层级分类，使用 `/` 分隔（如 `数学/几何`）
- 创建新片段时可同时指定名称和分类
- 拖拽缩略图到分类树节点即可重新归类
- 支持批量修改多个片段的分类
- 分类树节点显示片段数量（含子分类汇总数）

### 模糊搜索

- 基于 UTF-8 子序列匹配，不依赖拼音转换
- 打分规则：连续匹配字符得高分（`10 + consecutive * 5`），间隔匹配低分
- 名称匹配权重是简介的 2 倍
- 空搜索显示所有片段

### LaTeX 编译与 PDF 预览

**编译流程**：

1. 加载选中模板，将额外宏包（`\usepackage`）和 TikZ 库（`\usetikzlibrary`）注入模板导言区 `\begin{document}` 前
2. 将 TikZ 核心代码注入模板的 `%%% TIKZ_CODE_HERE %%%` 位置
3. 写入临时 `.tex` 文件（位于 `/tmp/TikzManager/<snippetId>/output.tex`）
4. 异步调用 `xelatex -interaction=nonstopmode -halt-on-error -no-shell-escape`
5. 编译成功 → PDF 加载到预览区 → 生成缩略图 PNG（150 DPI）→ 持久化到片段目录
6. 编译失败 → 日志面板显示错误 → 双击跳转错误行

### 参数化系统

在 TikZ 代码中使用 `% @param:` 注释声明参数，`@@var@@` 作为占位符：

```latex
% @param: angle=30
% @param: radius=2
\begin{tikzpicture}
  \draw (0,0) -- ({@@radius@@*cos(@@angle@@)}, {@@radius@@*sin(@@angle@@)});
\end{tikzpicture}
```

- **自动解析**：代码编辑时实时扫描 `% @param:` 行，动态生成 `变量名: [默认值]` 输入框
- **应用参数**：将 `@@var@@` 替换为输入框中的当前值后触发编译
- **批量预览**：生成所有预览时自动使用默认值替换参数
- **复制代码**：复制的代码为参数替换后的最终代码（注释行已移除）

### 模板系统

三个极简 LaTeX 模板（额外宏包和 TikZ 库由每个片段自己声明）：

| 模板 | 用途 | 内置宏包 |
|------|------|---------|
| `default_math` | 数学图形 | `tikz`, `amsmath`, `xcolor` |
| `default_physics` | 物理示意图 | `tikz`, `xcolor` |
| `default_circuit` | 电路图 | `tikz`, `xcolor`, `circuitikz`（europeanvoltages, betterproportions）, `preview`（active, tightpage） |

所有模板均使用 `standalone` 文档类，生成紧凑的独立 PDF。

模板文件位于 `~/.local/share/TikzManager/templates/`，可通过设置面板的模板管理界面创建、编辑、删除。

### 宏包与 TikZ 库

每个片段可声明自己需要的额外 LaTeX 宏包和 TikZ 库：

**额外宏包**（`packages` 字段）：

用逗号分隔宏包名，可选参数用 `[options]` 前置，如：
```
tikz-3dplot,[european,nosiunitx]circuitikz,tikz-cd
```
编译时自动展开为：
```latex
\usepackage{tikz-3dplot}
\usepackage[european,nosiunitx]{circuitikz}
\usepackage{tikz-cd}
```

**TikZ 库**（`tikzLibraries` 字段）：

用逗号分隔库名，如：
```
calc,er,angles,patterns,decorations.pathmorphing
```
编译时自动展开为：
```latex
\usetikzlibrary{calc,er,angles,patterns,decorations.pathmorphing}
```

两者均在编译时注入到模板 `\begin{document}` 之前。

### 完整文档复制

工具栏"复制完整文档"按钮将当前片段的模板头部 + 额外宏包 + TikZ 库 + 参数替换后的 TikZ 代码组合成**完整可编译的 LaTeX 文档**复制到剪贴板。

### 多选与批量操作

- **Ctrl+点击**缩略图可多选（不触发编辑器加载）
- 选中多个片段后右键弹出批量操作菜单：
  - **批量导出所选**：打包为单个 `.tar.gz`
  - **修改分类**：统一修改所有选中片段的分类
  - **删除所选**：确认后批量删除
  - **全选**：选中全部
  - **导出全部**：将所有片段打包导出

### 剪贴板操作

| 操作 | 说明 |
|------|------|
| 复制代码 | 复制参数替换后的 TikZ 核心代码 |
| 复制完整文档 | 复制含模板头部的完整 LaTeX 文档 |
| 复制 PNG | 从 PDF 转换 300 DPI PNG 后复制到剪贴板 |
| 复制 SVG | 从 PDF 转换 SVG 后复制（附带 `image/svg+xml` MIME 类型） |

### 导入与导出

**导出**：支持三种粒度 — 导出当前片段 / 批量导出所选 / 导出全部片段，保存为 `.tar.gz` 文件。

**导入**：选择 `.tar.gz` 或 `.zip` 文件 → 自动解压并为每个片段分配新 UUID → 刷新列表。

---

## 键盘快捷键

全部快捷键均可在**设置面板 → 快捷键设置**中自定义键序列，清空则禁用该快捷键。

| 功能 | 默认快捷键 |
|------|----------|
| 全局快捷键：显示/隐藏窗口 | `Ctrl+Alt+T`（KDE 通过 KGlobalAccel 注册） |
| 复制 TikZ 代码 | `Ctrl+Shift+C` |
| 复制 PNG | `Ctrl+Shift+P` |
| 复制 SVG | `Ctrl+Shift+S` |
| 编译预览 | 无（可在设置中自定义） |
| 应用参数 | 无（可在设置中自定义） |
| 保存 | 无（可在设置中自定义） |

---

## 预置片段清单

程序首次启动时自动安装 9 个高质量教学示例（数学 3 / 物理 3 / 电路 3）：

| 分类 | 片段名称 | 简介 | 参数 |
|------|---------|------|------|
| 数学/几何 | 勾股定理 | 直角三角形三边正方形面积关系 | — |
| 数学/函数 | 三角函数图像 | 正弦和余弦函数曲线 | `xmax=7.5` |
| 数学/几何 | 立方体投影 | 三维立方体斜二测投影 | — |
| 物理/力学 | 斜面受力分析 | 重力分解：N、mg sinθ | — |
| 物理/力学 | 单摆运动 | 摆球、重力与张力 | `angle=30` |
| 物理/力学 | 弹簧振子 | 墙壁、弹簧、质量块 | `x=2.5` |
| 电路/基础 | RLC串联电路 | RLC 串联谐振电路 | — |
| 电路/基础 | 分压电路 | 两电阻分压电路 | — |
| 电路/放大 | 运算放大器 | 反相运算放大器 | — |

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

程序配置通过 `QSettings` 存储，包括：
- `xelatex/path`, `pdftocairo/path`, `paths/texinputs`, `png/dpi`
- `editor/fontSize` — 代码字体大小
- `shortcuts/copyCode`, `shortcuts/copyPng`, `shortcuts/copySvg`
- `shortcuts/compile`, `shortcuts/applyParams`, `shortcuts/save`
- `shortcuts/globalHotkey` — 全局快捷键

---

## 设置面板

工具栏"设置"按钮打开设置对话框，包含以下区域：

**路径设置**：
- xelatex / pdftocairo 路径（默认从 `$PATH` 查找）
- TEXINPUTS 环境变量
- PNG DPI（72–1200，默认 300）
- 代码字体大小（8–48，默认 10）

**快捷键设置**：
- 全部 7 项操作均可自定义键序列
- 清空键序列 = 禁用该快捷键
- 按 Delete 键或点击清除按钮可清空

**模板管理**：
- 左侧列表展示所有 `.tex` 模板
- 右侧代码编辑区可编辑选中模板
- +/- 按钮创建 / 删除模板
- 模板中必须包含 `%%% TIKZ_CODE_HERE %%%` 占位符

**工具**：
- **生成所有预览** — 遍历全部片段编译生成 PDF + 缩略图 PNG
- **恢复出厂设置** — 删除所有用户数据，需输入"确定重置"二次确认（高危操作）

---

## 命令行构建

```bash
# 基本构建（自动检测 KGlobalAccel）
cmake -B build
cmake --build build -j$(nproc)

# 构建并运行
./build/TikzManager

# 运行测试
cd build && ctest --output-on-failure
```

CMake 选项：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `WITH_KGLOBALACCEL` | ON | 启用 KDE KGlobalAccel |
| `WITH_QHOTKEY` | ON | KGlobalAccel 不可用时回退 QHotkey |

---

## 项目架构

```
src/
├── main.cpp                         # 入口（QApplication 初始化）
├── mainwindow.h / mainwindow.cpp    # 主窗口（UI 编排、信号槽、业务逻辑）
├── search_panel.h / search_panel.cpp # 左栏组件（搜索框、分类树、缩略图列表、多选与右键菜单）
├── snippet_manager.h / .cpp         # 数据模型与文件系统 CRUD（含批量操作）
├── latex_compiler.h / .cpp          # xelatex 编译 + pdftocairo 转换 + 宏包/库注入
├── code_editor.h / .cpp             # 带行号的代码编辑器
├── settings_dialog.h / .cpp         # 设置面板与模板管理（含快捷键配置、重置）
├── snippet_properties_dialog.h / .cpp # 属性编辑对话框
├── kde_global_shortcut.h / .cpp     # KDE KGlobalAccel 全局快捷键
resources/
├── templates/                       # 出厂模板（3 个极简模板）
│   ├── default_math.tex            # tikz, amsmath, xcolor
│   ├── default_physics.tex         # tikz, xcolor
│   └── default_circuit.tex         # tikz, xcolor, circuitikz, preview
└── presets/                         # 出厂预置片段（9 个）
    └── <uuid>/
        ├── meta.json
        └── snippet.tex
tests/
├── test_snippet_manager.cpp         # CRUD 操作 + ZIP 导入/导出测试
├── test_latex_compiler.cpp          # 编译 + PNG/SVG 转换测试
├── test_search.cpp                  # 模糊搜索算法 + 分类测试
└── test_packages_libraries.cpp      # 宏包/TikZ库解析与模板注入测试
```

### 核心类关系

```
MainWindow
├── SearchPanel          ← SnippetManager
│   ├── QLineEdit        搜索框
│   ├── QTreeView        分类树
│   └── QListView        缩略图网格（ExtendedSelection 多选）
├── CodeEditor           代码编辑器（带行号，字号可调）
├── QPlainTextEdit       编译日志面板
├── QSplitter（垂直）    PDF 预览与元数据区可调分割
│   ├── QPdfView         PDF 矢量预览
│   └── QScrollArea      元数据编辑表单
├── LatexCompiler        编译引擎（xelatex + pdftocairo + 宏包/库解析注入）
├── SnippetManager       数据层（JSON 读写、搜索、分类、批量操作）
├── SettingsDialog       设置面板（含快捷键配置、模板管理）
└── KdeGlobalShortcut    KDE 全局快捷键（或 QHotkey 回退）
```

### 信号关键路径

```
搜索/分类变化 → SearchPanel::snippetSelected(id)
              → MainWindow::loadSnippetIntoEditor(id)

编译按钮 → saveCurrentSnippet() → compiler->compile()
         → compilationFinished(success, pdf, log)
         → QPdfView 加载 PDF + savePreviewData() 生成缩略图

批量操作 → SearchPanel::batchExportRequested / batchDeleteRequested / ...
          → MainWindow 处理对话框 → SnippetManager 批量方法
```

---

## 测试

项目包含四套自动化测试（通过 CTest 运行）：

| 测试 | 内容 |
|------|------|
| `test_snippet_manager` | 片段创建/读取/更新/删除，loadCode，renameCategory，ZIP 导入/导出往返 |
| `test_latex_compiler` | xelatex 可用性检测，基本编译，PDF 生成，PNG 转换，SVG 转换，错误编译日志验证 |
| `test_search` | 精确匹配、子序列匹配、连续加分、中文搜索、分类统计 |
| `test_packages_libraries` | 宏包字符串解析（含中括号选项），TikZ 库解析，模板注入正确性，往返序列化 |

运行测试：
```bash
cd build && ctest --output-on-failure
```

---

## 技术栈

| 技术 | 用途 |
|------|------|
| **C++17** | 核心语言 |
| **Qt 6** | Widgets (UI), Pdf (PDF 渲染), PdfWidgets (QPdfView) |
| **CMake** | 构建系统 |
| **KF6GlobalAccel** | KDE 原生全局快捷键 |
| **QHotkey** (fallback) | 非 KDE 环境的全局快捷键 |
| **XeLaTeX** | LaTeX → PDF 编译 |
| **pdftocairo** | PDF → PNG / SVG 转换 |
| **JSON** | 片段元数据格式 |
| **tar / unzip** | 存档导入/导出 |
