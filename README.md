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
  - [代码编辑器](#代码编辑器)
  - [代码补全](#代码补全)
  - [LaTeX 编译与 PDF 预览](#latex-编译与-pdf-预览)
  - [参数化系统](#参数化系统)
  - [自动保存与草稿恢复](#自动保存与草稿恢复)
  - [模板系统](#模板系统)
  - [宏包与 TikZ 库](#宏包与-tikz-库)
  - [自定义命令处理](#自定义命令处理)
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
- [已知问题](#已知问题)

---

## 功能概览

HiTikZ 围绕「片段（snippet）」这一核心概念组织：每个片段是一段独立的 TikZ 代码及其元数据。以下按功能领域概述主要能力，详细说明见[核心功能说明](#核心功能说明)。

### 片段与内容管理

- **片段管理**：创建、编辑、保存、删除 TikZ 代码片段，附带名称 / 简介 / 分类 / 标签 / 宏包 / TikZ 库 / 编译命令 / 模板等元数据
- **树形分类**：支持层级分类（如 `数学/几何`），含「全部」与「未分类」节点，拖拽片段到分类节点即可重新归类
- **模糊搜索**：Unicode 子序列匹配 + 双字倒排索引加速，按名称与简介搜索，连续匹配高分、间隔扣分
- **标签过滤**：搜索框下方自动展示所有标签徽章，点击筛选，多标签 AND 组合；标签过多时折叠为两行并支持弹窗选择；增删片段标签后标签栏即时更新，某标签不再被任何片段使用时自动从标签栏移除并取消其选中状态（避免"选中已删除标签导致列表空白且无法取消"）；标签集合未变时不重建标签栏，保存时不再闪烁
- **多选与批量操作**：Ctrl+点击多选缩略图，右键批量导出 / 改分类 / 删除 / 全选 / 导出全部
- **属性编辑**：右键单个缩略图弹出属性对话框，编辑全部元数据字段（含自定义编译引擎）

### 代码编辑器

- **多标签编辑器**：同时打开多个片段并快速切换，重复打开同一片段自动切换到已有标签页；每个标签页的元数据相互隔离
- **语法高亮**：TikZ/LaTeX 彩色高亮，含 **14 条正则规则 + 跨行选项括号处理 + 6 类用户定义名称**；key=value 分色显示且花括号深度感知，支持跨行选项列表与 `\begin{comment}` 多行注释块
- **选中词高亮**：光标置于某单词或双击选中时，文档中所有相同单词自动高亮，便于定位变量 / 命令
- **智能代码补全**：上下文感知的结构化补全，覆盖 TikZ/PGF 及多个专业绘图宏包，12 种上下文 + 三级过滤，并自动纳入用户定义的样式、坐标、命令等（详见[代码补全](#代码补全)）；`\begin{env}` 补全后自动插入 `\end{env}`
- **自动缩进**：换行继承上一行缩进，`{` 或 `\begin` 结尾的行自动增加一级缩进；当光标位于独占一行的 `{|}` 中间时按回车，`{` 与 `}` 自动拆分为上下两行、`}` 与 `{` 缩进对齐，光标落在中间缩进一级的新行
- **块缩进 / 反缩进**：选中多行后按 `Tab` 键整体增加一级缩进（4 空格），按 `Shift+Tab` 整体减少一级缩进；无选中时 `Shift+Tab` 反缩进当前行，便于手动对齐
- **括号自动配对**：输入 `{`/`[`/`(`/`$` 自动补全闭合符并置光标于中间；有文本选中时括号（含行内公式 `$…$`）包裹选中内容；输入闭合括号或再次输入 `$` 时若后一字符相同则跳过避免重复；光标在空括号对（含 `$$`）中按退格键同时删除两侧字符
- **过长行自动换行**：可选（默认开启），过长代码行软换行显示无需横向滚动，续行在行号栏以 `↳` 标记
- **撤销/重做**：Ctrl+Z / Ctrl+Shift+Z，工具栏以 ↩ ↪ 表示

### 编译与预览

- **实时编译预览**：默认调用 `xelatex -shell-escape` 编译，`QPdfView` 渲染高清 PDF 矢量预览；每个片段可独立指定编译引擎与参数（XeLaTeX / LuaLaTeX / PdfLaTeX）
- **强制结束**：遇到死循环代码可随时通过工具栏「强制结束」按钮中断编译
- **精简编译日志**：仅显示编译命令、错误（带上下文）与警告，过滤文件加载等噪音行，错误行号自动映射到编辑器行号并支持双击跳转
- **PDF 交互**：适应整页 / 宽度 / 高度三种缩放模式，滚轮以鼠标位置为中心缩放、左键拖拽平移；重新编译后保持视口位置便于对比
- **预览持久化**：编译成功后保存 PDF 与缩略图 PNG，切换片段即时展示
- **批量预览**：设置中「生成所有预览」多线程并行编译（线程数可调，默认 6），完成后弹出成功/失败统计报告

### 参数化与文档组织

- **参数化系统**：通过 `% @param: var=默认值` 声明变量，`@@var@@` 占位，右栏动态生成参数控件，编辑器内 `@@` 可触发参数补全
- **模板系统（极简）**：三个内置 LaTeX 模板仅含必要宏包（数学 / 物理 / 电路），额外宏包与 TikZ 库由每个片段自行声明
- **宏包与 TikZ 库**：每个片段可声明额外 `\usepackage` 宏包与 `\usetikzlibrary` 库，编译时自动注入导言区；右下角「额外宏包」「TikZ库」两栏均带逗号感知的名称自动补全，且库栏改动即时同步到编辑器补全（无需在代码里写 `\usetikzlibrary`）
- **自定义命令智能处理**：自动识别并抽取约 40 类定义命令（`\newcommand`、`\NewDocumentCommand`、`\tikzset`、`\definecolor`、`\pgfmathsetmacro` 等），编译与复制文档时注入导言区

### 导入 / 导出与剪贴板

- **存档导入/导出**：片段以 tar.gz 打包，支持单选 / 多选 / 全部导出
- **导入 .tex 文件**：自动提取 TikZ 代码（三段式回退），解析导言区宏包、TikZ 库与自定义命令，并按内容自动选择模板
- **从剪贴板导入**：直接粘贴完整 `.tex` 源码即可导入，解析流程同上
- **格式导出**：可将预览导出为 `.tex` / `.pdf` / `.png` / `.svg` 文件，或将 PNG / SVG 复制到剪贴板

### 数据安全与恢复

- **自动保存**：每 3 分钟自动保存所有打开标签页的完整状态为 JSON 草稿
- **草稿恢复**：异常退出后重启弹出恢复对话框，列出未保存草稿供勾选恢复或丢弃
- **保存后自动编译**：可选，保存后自动编译并刷新预览
- **关闭提示**：关闭标签页或退出程序时检查未保存更改

### 系统集成与个性化

- **系统托盘**：最小化到托盘，全局快捷键一键呼出/隐藏
- **可配置快捷键**：全部快捷键可在设置面板中自定义键序列，支持清空禁用
- **全局快捷键（KDE）**：KDE 桌面通过 KGlobalAccel 注册系统级快捷键
- **字体大小调节**：可分别调整代码编辑区与全局界面字体大小（均 8–48 pt）
- **依赖检测**：启动时检测 `xelatex` 与当前 SVG 转换工具，缺失时弹出安装指引

### 内容与实现

- **120 个高质量预置片段**：涵盖数学、物理、电路、化学等领域，均从 tikz.net 和 TeXample.net 精选并经 XeLaTeX 编译验证
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
| **pdftocairo** | poppler 工具集（默认 SVG 转换工具） |
| **Inkscape** | （可选）备选 SVG 转换工具，在设置面板中切换 |
| **tar** | GNU tar（用于导入/导出存档） |
| **unzip** | （可选）导入 .zip 格式存档时的回退工具 |

## 依赖安装指南

### Arch / Manjaro

```bash
sudo pacman -S --needed base-devel cmake git \
    qt6-base qt6-pdf qt6-tools \
    extra-cmake-modules kglobalaccel
```

> **注意**：如果 `kglobalaccel` 包未提供 KF6 版本，可尝试安装 `kglobalaccel6`。当前 Arch 滚动更新，`kglobalaccel` 已默认提供 KF6。

---

### Debian / Ubuntu

> **前置条件**：需要 **Debian Trixie (13)+** 或 **Ubuntu 24.04+**，较低版本可能缺少 `qt6-pdf-dev` 或 `libkf6globalaccel-dev`。

```bash
sudo apt update
sudo apt install -y build-essential cmake git pkg-config \
    qt6-base-dev qt6-pdf-dev qt6-tools-dev \
    extra-cmake-modules libkf6globalaccel-dev
```

> **如果 `qt6-pdf-dev` 不存在**（较旧系统），可改用 PPA：
> ```bash
> sudo add-apt-repository ppa:okirby/qt6-backports
> sudo apt update
> sudo apt install qt6-pdf-dev
> ```

---

### Fedora

> **前置条件**：建议 **Fedora 40+**（KF6 和 Qt6 PDF 模块完整可用）。

```bash
sudo dnf install -y cmake git gcc-c++ make pkgconf-pkg-config \
    qt6-qtbase-devel qt6-qtpdf-devel qt6-qttools-devel \
    extra-cmake-modules kf6-kglobalaccel-devel
```

---

### 构建步骤（通用）

```bash
# 克隆项目
git clone https://gitee.com/ylxdxx/HiTikZ.git #国内
git clone https://github.com/YLXDXX/HiTikZ.git #国外
cd HiTikZ

# 构建
cmake -B build
cmake --build build -j$(nproc)

# 直接从构建目录运行（无需安装）
./build/hitikz

# 运行测试
cd build && ctest --output-on-failure
```

> 可执行程序名为全小写的 **`hitikz`**。直接从构建目录运行时，程序会自动使用源码树内的 `resources/`；安装后则使用安装目录中的资源。

如需禁用 KGlobalAccel（仅用 QHotkey 后备方案）：
```bash
cmake -B build -DWITH_KGLOBALACCEL=OFF
```

如需同时禁用全局快捷键：
```bash
cmake -B build -DWITH_KGLOBALACCEL=OFF -DWITH_QHOTKEY=OFF
```

---

### 安装与卸载

遵循 Linux 目录惯例：用户自行编译的程序默认安装到 **`/usr/local`**（可通过 `CMAKE_INSTALL_PREFIX` 或 `--prefix` 自定义）。

```bash
# 安装（默认前缀 /usr/local，需要写入系统目录故用 sudo）
sudo cmake --install build

# 指定安装前缀（例如系统级 /usr）
cmake --install build --prefix /usr

# 卸载（依据安装时生成的 install_manifest.txt 移除已安装文件）
sudo cmake --build build --target uninstall
```

安装内容与目标位置（以默认前缀 `/usr/local` 为例）：

| 文件 | 安装位置 |
|------|---------|
| 可执行程序 `hitikz` | `/usr/local/bin/` |
| 预置片段与模板 `resources/` | `/usr/local/share/hitikz/resources/` |
| 桌面入口 `hitikz.desktop` | `/usr/local/share/applications/` |
| 应用图标 `hitikz.svg` | `/usr/local/share/icons/hicolor/scalable/apps/` |

> **打包用途**：安装步骤支持 `DESTDIR`（如 `DESTDIR=/tmp/stage cmake --install build`），便于制作发行版软件包。

> **版本号**：版本号在 `CMakeLists.txt` 的 `project(... VERSION x.y)` 处**统一设定**，经编译宏 `APP_VERSION` 传入程序（`main.cpp` 不再硬编码版本号），后续升级只需修改这一处。

---

### 运行时依赖（非构建依赖）

运行时需要 LaTeX 发行版：

| 发行版          | 安装命令                                                     |
| --------------- | ------------------------------------------------------------ |
| Arch / Manjaro  | `sudo pacman -S texlive`                                     |
| Debian / Ubuntu | `sudo apt install texlive texlive-latex-extra texlive-pictures` |
| Fedora          | `sudo dnf install texlive-scheme-medium texlive-pictures`    |

PDF 预览依赖 `Qt6::PdfWidgets`，已在构建依赖中包含，无需额外运行时安装。

## 用户界面

程序采用经典三栏布局，全栏宽度可拖动调整，右侧 PDF 预览与元数据区可上下拖动分割条。

### 工具栏

按从左到右顺序：

| 按钮 | 功能 |
|------|------|
| 新建片段 | 弹出对话框输入名称、分类和模板（默认 `default_math`） |
| 删除片段 | 删除当前片段或分类 |
| 导入/导出 ▼ | 下拉菜单：导入存档 / 导入 .tex 文件 / 从剪贴板导入 / 导出当前 / 导出全部 / 导出为 Tex 文档 / 导出为 PDF 文档 / 导出为 PNG 图片 / 导出为 SVG 图片 |
| ↩ / ↪ | 撤销（Ctrl+Z）/ 重做（Ctrl+Shift+Z） |
| 编译预览 | 保存并编译 TikZ 代码渲染 PDF（自动应用参数替换） |
| 应用参数 | 不保存，仅用参数值替换后编译（用于临时预览参数效果） |
| 保存 | 保存当前片段（若在设置中开启"保存后自动编译"，则保存后自动触发编译） |
| ⛔ 强制结束 | 强制中断正在进行的编译或批量生成（死循环 TikZ 代码 / 编译卡死时使用） |
| 复制代码 | 复制参数替换后的 TikZ 核心代码 |
| 复制文档 | 复制含模板头部的完整 LaTeX 文档 |
| 复制PNG | 复制 300 DPI PNG 到剪贴板 |
| 复制SVG | 复制 SVG 到剪贴板 |
| 适应整页/宽度/高度 | PDF 显示模式（可选中态） |
| − / + | PDF 缩小/放大 |
| 设置 | 打开设置面板 |

### 左栏

- **搜索框**：输入关键词实时搜索（150ms 防抖延迟），按名称 + 简介模糊匹配
- **标签过滤器**：搜索框下方的流式布局标签徽章，点击标签筛选片段（AND 逻辑），选中标签蓝色高亮。标签过多时自动折叠为最多两行（150ms 防抖，调整窗口宽度时稳定不闪烁），点击"更多标签..."弹出对话框展示全部标签（同样使用流式布局方便浏览）。每次增删片段标签（编辑保存 / 编译 / 属性对话框 / 删除 / 批量操作）都会即时刷新标签栏：新标签自动出现，不再被任何片段拥有的标签自动消失；若被移除的标签正处于选中状态，会自动取消选中并重新过滤，避免筛选停留在已不存在的标签上导致缩略图栏空白。刷新时先比对标签集合，**仅在集合真正变化时才重建标签栏**——保存时若标签未变则保留现有按钮及其折叠状态，杜绝"每次保存标签栏先展开再收拢"的闪烁
- **分类树**：层级分类导航，节点显示片段数量，点击分类过滤缩略图列表
  - 含"全部"和"未分类"两个特殊节点
  - 右键分类节点：重命名 / 删除 / 新建子分类
  - 右键"全部"：新建顶级分类
- **缩略图列表**：网格视图展示搜索结果 / 分类筛选结果（通过可拖动的分隔条与分类树调节高度比例）
  - 左键点击：选中片段并加载到编辑器（新标签页）
  - **Ctrl+点击**：多选（不触发编辑器加载）
  - 右键点击（单选）：弹出属性编辑对话框
  - 右键点击（多选）：弹出批量操作菜单（批量导出 / 修改分类 / 删除所选 / 全选 / 导出全部）
  - 拖拽到分类节点：移动片段到目标分类（支持多选拖拽）

### 中栏

- **多标签代码编辑器**：基于 `QTabWidget`，支持同时打开多个 TikZ 片段，标签页标题显示片段名称，未保存时显示 `*` 后缀
  - 点击缩略图打开片段时自动创建新标签页，已打开的片段直接切换到对应标签页
  - 标签页可关闭（× 按钮或 Ctrl+W），关闭前检查未保存更改（保存/放弃/取消）
  - 切换标签页时自动更新右侧的 PDF 预览、元数据表单和参数面板；每个标签页的元数据编辑（名称/简介/标签/宏包/库/模板）在标签页之间相互隔离，切走再切回不会串味，标签标题也不会互相污染
  - 退出程序时检查所有标签页的未保存更改，支持全部保存/全部放弃
- **代码编辑器**：基于 `QPlainTextEdit`，等宽字体（字号可调，默认 10pt），行号显示，当前行高亮，**TikZ/LaTeX 语法彩色高亮**，**选中词全文档高亮**，**智能代码补全**，**括号自动配对**（`{}`/`[]`/`()` 自动补全、选中文本包裹、闭合括号跳过覆盖、空括号对一键退格删除）
- **编译日志**：精简显示 xelatex 输出，仅展示编译命令、警告、错误信息，红色 = 错误、橙色 = 警告
  - 错误行号 `l.X` 自动换算为编辑器对应行号
  - 双击日志行中的 `l.<行号>` 跳转编辑器对应行

### 右栏

上部分为 **PDF 预览**，下部分为 **元数据编辑**（可拖拽分割条调整比例）：

- **PDF 预览**：Qt6 `QPdfView` 矢量渲染，支持工具栏缩放控制、鼠标滚轮缩放（以光标位置为中心）、左键拖拽平移。重新编译后保持当前缩放倍率和滚动位置，方便对比修改后的细节变化。
- **元数据表单**：名称、简介、标签、额外宏包、TikZ 库、模板选择
- **参数控件**：自动识别 `% @param:` 注释，动态生成输入框
- 右栏元数据区可在分割条拖到底时自动滚动

---

## 核心功能说明

### 片段管理

每个 TikZ 片段是一个独立目录，包含三个核心文件：

```
~/.local/share/HiTikZ/TikzManager/
├── snippets/               # 用户创建的片段
│   └── <uuid>/
│       ├── meta.json       # 名称、简介、分类、标签、模板、宏包、TikZ 库
│       ├── snippet.tex     # \begin{tikzpicture}...\end{tikzpicture} 核心代码
│       └── preview.png     # 最后一次编译成功的缩略图
├── presets/                # 系统预置片段（首次启动从 resources/presets/ 拷贝）
│   └── <uuid>/ ...
├── templates/              # LaTeX 模板（首次启动从 resources/templates/ 拷贝）
│   └── *.tex
└── drafts/                 # 自动保存的草稿（JSON 格式）
    └── *.json
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
  "tikzLibraries": "",
  "compileCommand": ""
}
```

### 分类系统

- 支持层级分类，使用 `/` 分隔（如 `数学/几何`）
- 创建新片段时可同时指定名称和分类
- 拖拽缩略图到分类树节点即可重新归类（支持多选拖拽）
- 支持批量修改多个片段的分类
- 分类树节点显示片段数量（含子分类汇总数）
- "全部"节点显示所有片段总数，"未分类"节点显示无分类的片段数

### 模糊搜索

- 基于 Unicode 子序列匹配，使用 NFC 归一化和 case-folding 进行大小写不敏感匹配
- 双字（bigram）倒排索引：查询长度 ≥2 时通过索引快速筛选候选，再对候选做全匹配打分
- 打分规则：连续匹配字符得高分（`10 + consecutive * 5`），间隔匹配低分
- 名称匹配权重是简介的 2 倍
- 空搜索显示所有片段

### 代码编辑器

- **语法高亮**（`TikzHighlighter`）：基于 `QSyntaxHighlighter`，**14 条优先正则规则 + 跨行选项括号处理 + 用户定义动态规则**
  - 基础规则（按优先级）：
    - 注释 `%` → 灰色斜体
    - 字符串 `"..."` → 橙黄色
    - 参数 `@@var@@` → 绿色斜体
    - 环境 `\begin{...}` / `\end{...}` → 紫色加粗
    - 命令 `\draw`、`\node` 等 → 蓝色加粗
    - 数学模式 `$...$`、`\(...\)`、`\[...\]` → 绿色
    - 坐标 `(x,y)` → 橙色
    - 选项 `[...]` → 青色（通过块状态机跟踪，支持跨行选项列表）
    - 数字（含单位）→ 紫色
    - 括号 `{` `}` → 红色
    - PGF 路径 `/tikz/...`、`/pgf/...` 等 → 靛蓝色
    - 键处理器 `/.style`、`/.code`、`/.default` 等 → 粉红色
    - `\usetikzlibrary{...}` 库列表 → 棕色
  - 用户定义规则（从 `TikzDocumentState` 实时读取）：
    - 用户样式名 → 橙红加粗（如 `mystyle`、`test lines`）
    - 节点名 `(name)` → 深紫加粗
    - 坐标名 → 金色
    - foreach 变量 `\x` → 橄榄绿斜体
    - 用户自定义命令 `\mycmd` → 青蓝加粗
    - key=value 中的 key 名 → 深青加粗，与值分色（感知花括号深度，`{a=b}` 内部的 `=` 不会误标为键值分隔符；跨行选项通过 `InBracket` 块状态保持键值识别）
  - `TikzDocumentState` 通过 `CodeEditor` 注入，300ms 防抖重新解析，用户定义名变化后自动重高亮

- **选中词高亮**：光标置于单词上或双击选中时，文档中所有相同单词以浅橙色背景高亮，辅助查看变量/命令的使用位置

- **自动缩进**：按 Enter 换行时自动继承上一行的前导空白，若行尾为 `{` 或以 `\begin` 开头则额外增加 4 空格缩进。当光标位于独占一行的 `{|}` 中间（前面仅有缩进）时按回车，`{` 之后新增两行：光标停在缩进一级的中间行，`}` 移至新增的第二行并与 `{` 缩进对齐，既保持美观又方便多行录入

- **块缩进 / 反缩进**：与主流编辑器一致，选中多行代码后按 `Tab` 键为每一行增加一级缩进（4 空格），按 `Shift+Tab` 为每一行减少一级缩进（最多移除一级前导空白）；无选中时 `Shift+Tab` 反缩进当前行，用于手动对齐缩进

- **过长行自动换行**：由设置面板"行为设置 → 过长代码自动换行"控制（默认开启）。开启后过长的代码行在编辑器内软换行，无需横向滚动即可看到全部内容；行号栏按视觉行绘制——逻辑行的首行显示行号，软换行产生的续行显示 `↳` 标记，便于区分新行与续行

- **撤销/重做**：标准 Ctrl+Z / Ctrl+Shift+Z，工具栏有独立按钮

### 代码补全

智能补全（`TikzCompleter`）基于**结构化词库**（`TikzKeywordDB`，2000+ 条带环境/命令/库元数据的结构化条目）和**文档状态追踪**（`TikzDocumentState`，范围栈/用户定义名/活动库/`\usepackage` 检测），实现三层精细过滤。补全上下文检测采用**跨行回溯**——从光标向前回溯到最近未闭合的 `[` 或 `{`（带回溯上限），因此换行书写的选项 / 参数也能正确识别上下文并补全：

> **空括号不打扰**：当 `[]` 或 `()` 内尚无任何输入时（如刚由自动配对插入的空括号对），不会自动弹出一大堆补全项；此时可按 `Ctrl+Space` 手动激活补全弹窗。命令（`\`）、环境（`\begin{`/`\end{`）、库（`\usetikzlibrary{`）、锚点（`.`）等带明确前导符的上下文仍即时弹出。

**基础上下文**（12 种）：
| 上下文 | 触发条件 | 补全内容 |
|--------|---------|---------|
| 命令 | 输入 `\` 后接字母（`\` 前为 `{`/`(`/`[` 等非字母也可触发） | 按当前环境和激活库动态过滤的 TikZ/LaTeX/PGF/tkz-euclide 命令（仅提示合法的 `\` 命令，路径操作、选项、环境名等不会作为命令误提示） |
| 环境 | `\begin{` 未闭合 | ~65 个 LaTeX/TikZ 环境，接受补全后自动插入 `\end{env}` 并关闭括号，光标留在 `\begin` 行尾 |
| 结束环境 | `\end{` 未闭合 | ~65 个环境名，当前最内层未闭合环境自动排在首位 |
| 选项 | `[...]` 内 | 环境/命令/库三级过滤的选项，含 358 个 CircuiTikZ 路径元件（`R`/`C`/`pR`/`vR`/`rmeterwa` 等，含前缀快捷方式） |
| 锚点/键处理 | 字母或 `/` 后跟 `.` | 仅显示当前激活库可用的锚点（通用 16 个 + 库门控 40+ 个）+ 16 个 PGF 键处理器；不使用或未激活的库锚点（如 circuitikz 的 `wiper`）不出现 |
| 值 | `=` 后 | **智能过滤 + 花括号感知**：`=` 查找忽略 `{}` 嵌套（`{a=b}` 不误触发），在 `{...}` 花括号内（如 `.style={>=|}`、`.style={font=\|}`）也能正确触发等值补全，按 key 名匹配——颜色键提示完整调色板及用户颜色，`pattern=` 仅提示图案，`decoration=` 提示全部 27 个装饰名，定位键（above/below/left/right）提示坐标/节点名，`label=`/`pin=` 提示方位（above / above left / …），`font=`/`node font=` 提示字体命令（`\itshape`/`\bfseries`/`\tiny`…\Huge，支持样式体内以 `\` 开头的值），`of=`（name intersections）提示用户命名路径。多库共享同名键（如 `column sep` 同属 matrix 与 tikz-cd）时取值提示并集 |
| 路径操作词 | 路径体中 `\draw (0,0)` 之后的裸词（花括号深度 ≤ 0） | 精选的 ~24 个路径操作（`rectangle`/`circle`/`grid`/`arc`/`parabola`/`sin`/`cos`/`node` 等），不再混杂数学函数（`reciprocal`）或形状（`rectangle split`）等无关词 |
| 参数 | `@@` 未闭合 | 代码中声明的参数变量 |
| TikZ 库 | `\usetikzlibrary{` 内 | ~88 个 TikZ 库名（从词库动态读取） |
| 通用词 | 花括号内 `{...}` 中任意已知词 | 全部词表去重并集（~2200 词），适用于节点文本、数学表达式等场景 |
| 坐标/节点名 | `(` 后 | 用户定义的坐标名、节点名（含 `name=<name>` 语法提取含空格的名称如 `critical 1`，以及 `name intersections={…,by={…}}` 中 `by=` 声明的交点名） |
| 用户命令 | `\` 后 | 用户通过 `\newcommand`/`\def` 定义的命令 |

**环境过滤**：在 `\begin{circuitikz}...\end{circuitikz}` 内只显示电路相关选项，在 `\begin{axis}...\end{axis}` 内只显示 pgfplots 选项。

**命令过滤**：`\node[...]` 内额外显示形状/锚点/文本选项，`\draw[...]` 内额外显示线型/箭头/装饰选项。path 命令（draw/fill/to）也能提示 node 可用的选项（如形状）。`name path`/`name path global`/`name path local` 在 `\node` 与 `\draw`/`\path` 选项中均可用。

**含空格的补全值**：带空格的补全项（如定位键的 `of digit`、方位 `below left`）在选中上屏时按整段替换，不会在开头多出重复字符。

**to 路径**：`to path` 作为通用选项在 `\tikzset{}`/样式体（无命令上下文）中同样提示；其路径表达式内可补全坐标宏 `\tikztostart`/`\tikztotarget`/`\tikztonodes`。

**库过滤**：未加载 `decorations.pathmorphing` 库时不提示 `snake`/`coil`/`zigzag`；未加载 `angles` 库时不提示 `angle radius`/`angle eccentricity`。库门控覆盖全部主要 TikZ 库——chains（`start chain`/`on chain`/`join`）、spy（`spy using outlines`/`lens`/`magnification`）、through（`circle through`）、shadows（`shadow scale`/`shadow xshift`/`copy shadow`/`circular glow`）、shapes 系列（`rectangle split part fill`/`shape border uses incircle`/`callout absolute pointer`/`arrow box arrows`）、patterns.meta（`patterns/tile size`/`patterns/bounding box`）、trees（`sibling angle`）、mindmap（`root concept`/`concept connection`）、fit（`rotate fit`）、petri（`place`/`transition`/`token`/`tokens`/`colored tokens`/`children are tokens`/`token distance`）、automata（`state`/`accepting`/`accepting by arrow`/`initial`/`initial by diamond`/`state with output`）、backgrounds（`framed`/`gridded`/`show background grid`/`tight background`/`inner frame sep`/`outer frame xsep`）、positioning（`base left`/`base right`/`mid left`/`mid right`）、topaths（`loop`/`in control`/`out control`/`relative`/`min distance`/`max distance`，随 TikZ 常驻）、trees（`sibling angle`/`edge from parent fork down`/`grow via three points`/`clockwise from`）、er（`entity`/`relationship`/`attribute`/`key attribute`）、matrix（`above delimiter`/`below delimiter`/`nodes in empty cells`；`column sep`/`row sep` 提供距离与 `between origins`/`small`/`large` 等取值提示）、graphs（生长键 `grow right`/`grow down sep`/`branch left` 等、布局策略 `Cartesian placement`/`circular placement`/`no placement`/`chain shift`/`clockwise`/`phase`、节点存在策略 `use existing nodes`/`fresh nodes`、图模式 `simple`/`multi`/`quick`/`no edges`、节点与边键 `edges`/`math nodes`/`empty nodes`/`number nodes`/`typeset`/`as`/`name separator`/`trie`、边种类 `default edge kind`（值 `--`/`->`/`<-`/`<->`/`-!-`）/`new ->`/`new --` 等、锚点 `left anchor`/`right anchor`、源/目标边样式 `source edge style`/`target edge node`/`clear >`/`clear <`、`put node text on incoming/outgoing edges`、节点集合键 `V`/`W`/`n`/`m`/`name shore V`/`name shore W`/`declare`、命名图结构 `complete bipartite`/`matching`/`butterfly'` 及 `every graph`；`graphs.standard` 库门控子图 `subgraph I_n`/`I_nm`/`K_n`/`K_nm`/`C_n`/`P_n`/`Grid_n`/`G_np` 与其边概率键 `p`）、shadings（`upper left`/`upper right`/`lower left`/`lower right`）、mindmap（`root concept`/`concept connection`/`small mindmap`/`level 2 concept`/`circle connection bar switch color`）、3d（`canvas is xz plane at y`/`plane origin`/`plane x`）、quotes（`node quotes mean`/`edge quotes mean`/`quotes mean pin`）、decorations.text（`text effects`/`group letters`/`reverse text`/`fit text to path`）、calendar（`dates`/`day list right`/`week list`/`month label above centered`）、turtle（`fd`/`bk`/`lt`/`rt`）、lindenmayersystems（`axiom`/`rule set`/`l-system`）、fit（`rotate fit`）、perspective（`3d view`/`isometric view`）、spy（`spy scope`）、views（`meet`/`slice`）、intersections（`name path`/`name path global`/`name path local`）、rdf（`has type`/`is a bag`）、shapes.gates.logic（`and gate`/`use US style logic gates`/`logic gate symbol color`）、animations（`animate`）、decorations 全系列子选项键（`amplitude`/`segment length`/`pre length`/`post length`/`raise`/`mirror` 等 35+ 键）等。未加载 `circuitikz` 时不提示 CircuiTikZ 组件与锚点（`wiper`/`cathode`/`B`/`C`/`E` 等）；未加载 `tkz-euclide` 时不提示 tkz-euclide 命令；未加载 `arrows.meta` 时不提示 `Stealth`/`Latex`/`Kite` 等 meta 箭头。`\usepackage{...}` 自动激活对应补全库：`circuitikz` / `tkz-euclide` / `tikz-cd`(→`cd`) / `chemfig` / `tikz-feynman` / `physics` / `siunitx` / `pgfplots` / `tikz-3dplot`(→`3d`)。片段「额外宏包」元数据字段与编辑器内 `\usepackage` 均可触发。

**数学物理宏包补全**：`\usepackage{physics}` 激活 physics 包约 136 个命令（`\vb`/`\va`/`\vu`/`\dv`/`\pdv`/`\grad`/`\div`/`\curl`/`\laplacian`/`\qty`/`\bra`/`\ket`/`\braket`/`\expval`/`\comm`/`\dd`/`\cross`/`\norm`/`\tr` 等）；`\usepackage{siunitx}` 激活 siunitx 命令（`\SI`/`\si`/`\num`/`\ang`/`\qty`/`\unit`/`\numlist`/`\qtyrange`/`\sisetup` 等）；`\usepackage{pgfplots}` 激活 pgfplots 键。这些命令仅在对应宏包被使用时才提示（编辑器 `\usepackage` 或片段「额外宏包」字段均可触发），不与纯 TikZ 补全混杂。

**专业绘图宏包补全**：
- **CircuiTikZ**（`circuitikz`）：358 个路径元件 + 128 个 node 形状 + 全局键（`voltage`/`current`/`voltage dir`/`logic ports`/各类 `scale`/`mirror`/`invert`/`l`/`v`/`i`/`a` 标注键等）。
- **tkz-euclide**（`tkz-euclide`）：261 个 v5.10c 命令，涵盖定义点/线/圆/多边形/三角形、绘制、交点、角标注、变换等。v5 采用「先 `\tkzDef...` 定义、再 `\tkzDrawPolygon` 绘制」的写法，补全内容与 v5 API 保持一致。
- **tikz-cd**（`tikzcd` 环境）：箭头方向快捷命令（`\ar`/`\rar`/`\lar`/`\uar`/`\dar`/`\urar`/`\ular`/`\drar`/`\dlar`）+ 箭头样式键（`rightarrow`/`hook`/`harpoon'`/`two heads`/`maps to`/`dashed`/`squiggly`/`equal` 等）+ 图表选项（`row sep`/`column sep`/`crossing over`/`phantom` 等）。
- **chemfig**（`chemfig`）：约 40 个命令（`\chemfig`/`\definesubmol`/`\chemname`/`\chemabove`/`\schemestart`/`\charge`/`\polymerdelim` 等）+ `\setchemfig` 键（`atom sep`/`bond offset`/`double bond sep`/`angle increment`/`cram width`/`arrow offset` 等）。
- **tikz-feynman**（`tikz-feynman`）：命令（`\feynmandiagram`/`\vertex`/`\diagram`/`\feynman`）+ 粒子/边样式（`fermion`/`anti fermion`/`photon`/`boson`/`scalar`/`ghost`/`gluon`/`majorana`/`momentum`/`half left` 等）。

> **词库来源**：补全词库中的命令、选项、形状、装饰、数学函数等条目，均以本地 TeX Live 源码为准逐条核对，力求与实际宏包 API 一致，避免收录不存在的命令或选项。参照的源码版本为 PGF/TikZ 3.1.10、pgfplots 1.18.1、CircuiTikZ 1.7.1、tkz-euclide 5.10c、tikz-cd、chemfig 1.66、tikz-feynman 1.1.0、physics、siunitx。

**用户定义补全**：实时解析代码中的自定义定义，自动纳入补全：
- 样式：`\tikzset{name/.style={...}}`、`\tikzstyle{name}` → 选项上下文，含空格名（如 `test lines`）
- 坐标：`\coordinate (name) at ...`，以及路径操作写法 `\draw ... coordinate (name) ...`（如 `\draw (2,1) coordinate (test) circle [radius=2mm];`，可带 `[options]`）→ 坐标上下文与高亮
- 节点：`\node (name) ...`、`\pic (name) ...`、`\node[... name=<name> ...]` → 坐标上下文（含 `name=` 选项语法）
- foreach 变量：`\foreach \x in ...`、`\foreach \xyz / \xtext in ...`（支持空格分隔多变量及可选方括号参数如 `[count=\i]`）→ 命令上下文
- 命令：`\newcommand{\foo}`、`\def\foo` → 命令上下文
- 颜色：`\definecolor{name}`、`\colorlet{name}` → 值上下文（`color=`、`draw=`、`fill=` 等）
- 命名路径：`name path=<name>`、`name path global/local=<name>`（含 `\node` 上的命名路径）→ `name intersections={of=…}` 的 `of=` 值上下文
- 交点名：`name intersections={…,by={[opt]A,B}}` 中 `by=` 声明的坐标名（自动剥离 `[options]` 前缀）→ 坐标上下文
- 定义删除后补全列表即时清空，不留残留项

**tkz-euclide 5.10c 支持**（261 个命令）：初始化/裁剪（`\tkzInit`/`\tkzClip`），定义点/线/圆/多边形/三角形及其特殊心（`\tkzDefPoint`/`\tkzDefLine`/`\tkzDefCircle`/`\tkzDefTriangle`/`\tkzDefTriangleCenter` 等），绘制点/线/段/多边形/圆/弧/扇形（`\tkzDrawPolygon`/`\tkzDrawCircle`/`\tkzDrawArc` 等），交点计算（`\tkzInterLL`/`\tkzInterLC`/`\tkzInterCC`），角标注（`\tkzMarkAngle`/`\tkzMarkRightAngle`/`\tkzLabelAngle`），标签、变换与全局样式设置。所有命令标记 `requiredLibs={"tkz-euclide"}`，仅在 `\usepackage{tkz-euclide}` 检测到后出现。

> **v5 vs v4**：tkz-euclide v5 采用「先 `\tkzDef...` 定义、再 `\tkzDrawPolygon` 绘制」的 API 风格，与 v4 的 `\tkzDrawTriangle`/`\tkzDrawSquare` 等直接绘图命令不同。补全词库以 v5.10c 为准。

### LaTeX 编译与 PDF 预览

**编译流程**：

1. 加载选中模板，将额外宏包（`\usepackage`）和 TikZ 库（`\usetikzlibrary`）注入模板导言区 `\begin{document}` 前
2. 将 TikZ 核心代码注入模板的 `%%% TIKZ_CODE_HERE %%%` 位置
3. 写入临时 `.tex` 文件（位于 `/tmp/TikzManager/<snippetId>/output.tex`）
4. 异步调用 `xelatex -interaction=nonstopmode -halt-on-error -shell-escape`
5. 编译成功 → PDF 加载到预览区 → 生成缩略图 PNG（150 DPI）→ 持久化到片段目录
6. 编译失败 → 日志面板精简显示错误和警告 → 双击跳转错误行（行号已自动映射到编辑器行号）

**可自定义编译引擎**：每个片段可通过属性对话框（右键缩略图 → 属性）设置自定义编译命令，如 `lualatex -interaction=nonstopmode -shell-escape`。留空则使用默认 `xelatex -interaction=nonstopmode -halt-on-error -shell-escape`。支持 XeLaTeX / LuaLaTeX / PdfLaTeX 及自定义参数。该字段不显示在主界面右栏，属高级功能。

**编译日志**：
- 顶部显示完整编译命令
- 成功：仅显示警告信息 + 编译成功标记
- 失败：显示错误块（含上下文）+ 警告信息 + 编译失败标记
- 自动过滤 `.sty`、`.cls`、`.aux` 等文件加载噪音行
- 错误行号 `l.X` 自动从完整文档行号换算为编辑器行号

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
- **参数补全**：编辑器内输入 `@@` 可触发参数名补全（显示当前代码中声明的所有参数名）

### 自动保存与草稿恢复

- **定时保存**：每 3 分钟自动保存所有打开标签页的完整状态（代码 + 名称 + 简介 + 标签 + 额外宏包 + TikZ 库 + 模板 ID）为 JSON 草稿文件。每个未保存的草稿标签页使用独立编号文件名（如 `scratch_0.json`、`scratch_1.json`），防止多个草稿标签页相互覆盖
- **草稿恢复**：程序异常退出后下次启动时，自动弹出恢复对话框，列出所有未保存草稿的具体名称和简介，可勾选需要的草稿、全选或全部丢弃
  - 有对应片段的草稿恢复后加载到编辑器（不覆盖已保存版本）
  - 无对应片段的草稿自动创建为新片段后加载
- **关闭提示**：关闭标签页或退出程序时，如有未保存更改则弹出"保存 / 不保存 / 取消"对话框
  - 临时片段（无关联保存片段）选择保存时弹出新建片段对话框
  - 退出时选择"全部放弃"自动清理所有草稿文件
  - 托盘"退出"菜单触发完整的关闭流程（包含未保存检查）
  - 窗口 X 按钮仅隐藏到托盘（不触发保存检查）

### 模板系统

三个极简 LaTeX 模板（额外宏包和 TikZ 库由每个片段自己声明）：

| 模板 | 用途 | 内置宏包 |
|------|------|---------|
| `default_math` | 数学图形 | `tikz`, `amsmath`, `xcolor` |
| `default_physics` | 物理示意图 | `tikz`, `xcolor` |
| `default_circuit` | 电路图 | `tikz`, `xcolor`, `circuitikz`（europeanvoltages, betterproportions）, `preview`（active, tightpage） |

所有模板均使用 `standalone` 文档类，生成紧凑的独立 PDF。

模板文件位于 `~/.local/share/HiTikZ/TikzManager/templates/`，可通过设置面板的模板管理界面创建、编辑、删除。

### 宏包与 TikZ 库

每个片段可声明自己需要的额外 LaTeX 宏包和 TikZ 库。右下角「额外宏包」「TikZ库」两栏均提供逗号感知的名称自动补全——仅匹配当前正在输入的最后一段（前序条目原样保留）：额外宏包栏补全常用 LaTeX 宏包名，TikZ库栏补全 TikZ 库名。TikZ库栏的改动会即时同步到编辑器的补全上下文（例如加入 `through` 后，代码里的 `circle through` 立即可补全），无需在代码中另写 `\usetikzlibrary`。

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

解析器正确处理嵌套括号 `{}` 和 `[]`，如 `[option={val1,val2}]` 中的逗号不会错误分割。

**TikZ 库**（`tikzLibraries` 字段）：

用逗号分隔库名，如：
```
calc,er,angles,patterns,decorations.pathmorphing,shadows.blur,pgfplots.fillbetween
```
编译时自动展开为：
```latex
\usetikzlibrary{calc,er,angles,patterns,decorations.pathmorphing,shadows.blur,pgfplots.fillbetween}
```

两者均在编译时注入到模板 `\begin{document}` 之前。

### 自定义命令处理

在 TikZ 代码编辑器中，与 TikZ 绘图层无关的 LaTeX 定义命令（`\newcommand`、`\tikzset` 等）约定写在 `\begin{tikzpicture}` 或 `\begin{circuitikz}` 环境**之外**（上方）。系统自动检测并抽取这些命令，在编译和复制文档时将其放入导言区（`\documentclass` 之后、`\begin{document}` 之前），确保编译正确。

**支持的定义命令清单**（共 40+ 类）：

| 命令 | 语法示例 | 说明 |
|------|---------|------|
| `\newcommand` / `\renewcommand` / `\providecommand` | `\newcommand{\foo}[2]{#1+#2}` 或 `\newcommand\foo[2]{#1+#2}` | 旧格式，支持 `*` 变体 |
| `\NewDocumentCommand` / `\RenewDocumentCommand` / `\ProvideDocumentCommand` / `\DeclareDocumentCommand` | `\NewDocumentCommand{\foo}{ O{red} m }{\draw[#1] #2;}` | xparse 新格式 |
| `\NewExpandableDocumentCommand` | `\NewExpandableDocumentCommand{\foo}{ m }{#1}` | 可展开变体 |
| `\NewCommandCopy` | `\NewCommandCopy{\new}{\old}` | 命令复制 |
| `\DeclareMathOperator` | `\DeclareMathOperator{\argmax}{argmax}` | 数学算子声明 |
| `\DeclareRobustCommand` | `\DeclareRobustCommand{\foo}[1]{#1}` | 鲁棒命令 |
| `\tikzset` | `\tikzset{style/.style={draw=red}}` | TikZ 样式和 pic 定义 |
| `\tikzstyle` | `\tikzstyle{name}=[options]` 或 `\tikzstyle{name}+=[...]` | 旧版 TikZ 样式（兼容） |
| `\ctikzset` | `\ctikzset{bipoles/length=1cm}` | CircuitikZ 全局设置 |
| `\pgfkeys` | `\pgfkeys{/tikz/line width=1pt}` | PGF 键值设置 |
| `\pgfplotsset` | `\pgfplotsset{compat=1.18}` | pgfplots 兼容性设置 |
| `\definecolor` | `\definecolor{myblue}{RGB}{20,20,100}` | 颜色定义 |
| `\colorlet` | `\colorlet{myshadow}{blue!50!white}` | 颜色别名 |
| `\contourlength` | `\contourlength{1.4pt}` | 轮廓文本线宽（contour 包） |
| `\pgfmathsetmacro` / `\pgfmathsetlength` | `\pgfmathsetmacro{\n}{round(10/3)}` | PGF 数学宏/长度定义 |
| `\pgfmathdeclarerandomlist` | `\pgfmathdeclarerandomlist{lst}{{a}{b}}` | PGF 随机列表声明 |
| `\pgfmathdeclarefunction` | `\pgfmathdeclarefunction{f}{1}{#1*#1}` | PGF 自定义数学函数 |
| `\def` | `\def\mymacro#1{#1}` | TeX 原语定义 |
| `\newif` / `\newboolean` / `\setboolean` | `\newboolean{show}` `\setboolean{show}{true}` | 条件开关（ifthen） |
| `\newlength` / `\newcounter` / `\newsavebox` | `\newlength{\mylen}` | 寄存器分配 |
| `\setlength` | `\setlength{\parindent}{0pt}` | 长度设置 |
| `\setcounter` | `\setcounter{page}{1}` | 计数器设置 |
| `\sansmath` | `\sansmath` | 无衬线数学字体 |
| `\pgfdeclarelayer` / `\pgfsetlayers` | `\pgfdeclarelayer{bg}` `\pgfsetlayers{bg,main}` | PGF 图层管理 |
| `\pgfdeclareradialshading` / `\pgfdeclareverticalshading` | `\pgfdeclareradialshading[tikz@ball]{ring}{...}` | PGF 自定义渐变着色 |
| `\tikzoption` | `\tikzoption{myoption}{myvalue}` | 旧版 TikZ 选项定义（兼容） |
| `\pgfdeclaredecoration` | `\pgfdeclaredecoration{name}{initial}{...}` | PGF 自定义装饰 |
| `\usepgfplotslibrary` | `\usepgfplotslibrary{fillbetween}` | pgfplots 库 |
| `\tdplotsetmaincoords` | `\tdplotsetmaincoords{70}{110}` | tikz-3dplot 视角设置 |
| `\tikzmath` | `\tikzmath{ \x = 10; }` | TikZ 数学运算 |
| `\PreviewEnvironment` | `\PreviewEnvironment{tikzpicture}` | preview 包环境声明 |
| `\makeatletter` / `\makeatother` | `\makeatletter` … `\makeatother` | @ 字符类别切换 |

**抽取规则**：
- 仅提取 `\begin{tikzpicture}` / `\begin{circuitikz}` **之前**的定义命令
- 环境内部的命令（如图内 `\tikzset`）原样保留不提取
- 多行定义、嵌套花括号、混合格式（新旧混用、花括号/无花括号混用）均正确解析
- 参数与正文之间的空白/换行被容忍，如 `\newcommand{\foo}[2]`（换行）`{...}` 也能完整抽取
- 带分隔符参数文本的 `\def` 正确解析，如 `\def\foo[size=#1](#2,#3){...}`（参数文本含 `[]`/`()`/`#n`）
- 遇到不完整/无法解析的定义（如 `\newcommand{\foo}` 无 `{body}`）时安全跳过，不中断后续命令的提取
- 编译时：先注入宏包和 TikZ 库，再注入自定义命令，确保依赖顺序正确
- 清理时保留换行分隔符，防止 LaTeX 注释行与后续命令合并
- 复制文档/导出 .tex 时：抽取 → 注入完整文档导言区

### 完整文档复制

工具栏"复制文档"按钮将当前片段的模板头部 + 额外宏包 + TikZ 库 + 自定义命令 + 参数替换后的 TikZ 代码组合成**完整可编译的 LaTeX 文档**复制到剪贴板。

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
| 复制文档 | 复制含模板头部的完整 LaTeX 文档 |
| 复制 PNG | 从 PDF 转换 300 DPI PNG 后复制到剪贴板（与复制SVG互斥，防止并发冲突） |
| 复制 SVG | 从 PDF 转换 SVG 后复制（附带 `image/svg+xml` MIME 类型），转换工具可在设置中切换（与复制PNG互斥，防止并发冲突） |

### 导入与导出

**导出**：使用异步进程（非阻塞）调用 `tar` 打包，支持多种粒度和格式：
- **导出当前 / 导出全部**：打包为 `.tar.gz` 归档
- **批量导出所选**：多选后右键菜单批量打包
- **导出为 Tex 文档**：生成含模板头部的完整可编译 LaTeX 文档
- **导出为 PDF 文档**：复制预览 PDF 到指定路径
- **导出为 PNG 图片**：通过 pdftocairo 将预览 PDF 转换为 PNG（DPI 遵循设置面板配置）
- **导出为 SVG 图片**：通过 pdftocairo 或 Inkscape（可在设置面板中切换）将预览 PDF 转换为 SVG 矢量图

**导入**：
- **导入存档**：选择 `.tar.gz` 或 `.zip` 文件 → 异步解压并为每个片段分配新 UUID → 读取 `snippet.tex` 内容后再调用保存逻辑（防止保存操作覆盖导入的代码）→ 刷新列表。导入时自动清除 `isPreset` 标记，若缺失 `meta.json` 则自动创建片段
- **导入 .tex 文件**：选择单个 `.tex` 文件 → 自动提取 TikZ 代码（`\begin{document}...\end{document}` 之间 → `\begin{tikzpicture}...\end{tikzpicture}` → 全文回退三段式解析），同时从导言区解析：
  - `\usepackage{}`（含可选参数 `[...]`）→ 填入宏包字段
  - `\usetikzlibrary{}` → 填入 TikZ 库字段
  - 自定义命令（`\newcommand`、`\tikzset` 等）→ 提取后放置在 TikZ 代码上方
  - 文件名自动作为片段名称，根据宏包自动选择模板（含 circuitikz → `default_circuit`，否则 `default_math`）
- **从剪贴板导入**：直接读取剪贴板中的完整 `.tex` 源代码，执行与 `.tex` 导入相同的解析流程（宏包、库、自定义命令、模板自动检测），以文件第一行或注释作为片段名称，无需先创建文件

---

## 键盘快捷键

全部快捷键均可在**设置面板 → 快捷键设置**中自定义键序列，清空则禁用该快捷键。

| 功能 | 默认快捷键 |
|------|----------|
| 全局快捷键：显示/隐藏窗口 | 无（可在设置中自定义） |
| 撤销 | `Ctrl+Z` |
| 重做 | `Ctrl+Shift+Z` |
| 手动触发代码补全 | `Ctrl+Space` |
| 块缩进 / 反缩进（多行选中） | `Tab` / `Shift+Tab` |
| 复制 TikZ 代码 | 无（可在设置中自定义） |
| 复制 PNG | 无（可在设置中自定义） |
| 复制 SVG | 无（可在设置中自定义） |
| 编译预览 | `F6` |
| 应用参数 | 无（可在设置中自定义） |
| 保存 | `Ctrl+S` |
| 关闭标签页 | `Ctrl+W` |

---

## 预置片段清单

程序内置 120 个高质量 TikZ 教学示例，均从 [tikz.net](https://tikz.net) 和 [TeXample.net](https://texample.net) 精选并经过 XeLaTeX 编译验证，所有标题、描述、标签已本地化为中文。

### 分类统计

| 学科 | 数量 | 主要分类 |
|------|------|---------|
| 数学 | 23 | 几何、函数、微积分、统计、分析、拓扑、艺术 |
| 物理 | 89 | 力学、电磁学、光学、热力学、流体力学、相对论、量子、粒子物理 |
| 电路 | 5 | 交流电路、RC电路、变压器 |
| 化学 | 3 | 元素周期表、有机分子、物理化学 |

### 数学类（23 个）

**数学/几何**（7 个）：垂线作图、简单曲线、光滑曲线的控制点、自定义光滑曲线端点、球体体积、角度与标注、角平分线

**数学/函数**（3 个）：函数图像、极坐标、函数图像 - 坐标轴环境：反正弦/反余弦/反正切

**数学/微积分**（5 个）：柱坐标体积微分、直角坐标体积微分、球坐标表面积微分、函数平均值 - 线性函数、函数平均值 - 正弦函数

**数学/统计**（4 个）：正态分布、线性回归、高斯分布 - 68-95-99法则、高斯分布 - CLs方法 p值（大重叠）

**数学/分析**（2 个）：复数平面 - 复振子三维、复数平面 - 复数旋转

**数学/拓扑**（1 个）：平面到环面

**数学/艺术**（1 个）：彭罗斯三角 - 变体2

### 物理类（89 个）

**物理/力学**（33 个）：弹簧、滑轮系统、加速度、能量与功、抛体运动、弹簧 - 垂直弹簧静止伸长、弹簧 - 水平双弹簧、简谐振子 - 圆上相位、简谐振子 - 余弦阻尼、简谐振子 - 余弦过阻尼、滑轮系统 - 桌面滑轮弹簧、滑轮系统 - 桌面双滑轮、滑轮系统 - 天花板定滑轮、滑轮系统 - 天花板滑轮人力、摩擦力 - 水平地面举升、摩擦力 - 倾斜地面、物体稳定性 - 不稳定性（含中性平衡）、滑块稳定性 - 扭矩、扭矩 - 扭矩角度、转动惯量 - 圆环（二维）、转动惯量 - 圆环（三维）、转动惯量 - 圆环三维（平行轴定理）、转动惯量 - 空心圆柱、转动惯量 - 圆盘滑轮质量块、转动惯量（简化） - 细杆轴在一端、转动惯量（简化） - 圆盘、转动惯量（简化） - 空心圆盘、转动惯量（简化） - 圆环、转动惯量（简化） - 实心圆柱、转动惯量（简化） - 空心圆柱、转动惯量（简化） - 球体、振子近似 - 谐振子能量、摆与滑块 - 摆的解

**物理/电磁学**（32 个）：电磁波、霍尔效应、毕奥-萨伐尔定律、电流、磁力、球体电场、电场线1、电场线2、镜像电荷（平面）、镜像电荷（球体）、电磁波谱、变压器、变压器绕组、电磁波 - 彩色电磁波、霍尔效应 - 水平磁场俯视图、电流 - 传导模型、楞次定律 - 电流环磁场（NS退出）、磁场 - 水平磁场速度沿场方向、磁力 - 铁钉吸引磁铁SN、磁矩 - 磁矩翻转、电场 - 垂直电场、电场 - 路径积分、球体电场 - 带电实心球体三维、球体电场 - 带空腔导体、平面电场 - 圆形薄板积分、平面电场 - 含电场线、平面电场 - 含高斯面、电场线1 - 正点电荷等势面、电场线1 - 负点电荷等势面、体电场 - 体电荷、镜像电荷（平面） - 电场线、镜像电荷（球体） - 电场线

**物理/光学**（10 个）：光学折射、透镜、棱镜、棱镜2、光学棱镜、透镜像差、相位延迟片、光学折射 - 布儒斯特偏振角、光学偏振 - 偏振 90°与0°、光学偏振 - 偏振 90°/45°/0°

**物理/热力学**（6 个）：黑体辐射颜色、理想气体、热力学P-V图 - 等温线、热力学P-V图 - 卡诺循环、理想气体 - 盒中气体（有隔板）、理想气体 - 盒中气体（无隔板）

**物理/流体力学**（5 个）：伯努利原理、伯努利原理 - 伯努利方程、伯努利原理 - 文丘里效应、气压计（流体力学） - 开管压力计、气压计（流体力学） - 压力计

**物理/相对论**（1 个）：彭罗斯图

**物理/量子**（1 个）：布洛赫球

**物理/粒子物理**（1 个）：CMS三维坐标轴 - CMS坐标系（含LHC）

### 电路类（5 个）

变压器电路、电容电路 - 电容并联、RC电路 - 开关闭合、RC+EMF电路 - 开关断开、交流电路波形 - LCR串联

### 化学类（3 个）

元素周期表、有机分子结构、分子振动

---

## 数据存储

所有用户数据存储在 `~/.local/share/HiTikZ/TikzManager/`：

```
~/.local/share/HiTikZ/TikzManager/
├── snippets/               # 用户创建/导入的片段
│   └── <uuid>/
│       ├── meta.json
│       ├── snippet.tex
│       └── preview.png
├── presets/                # 系统预置（首次运行拷贝自 resources/presets/）
│   └── <uuid>/ ...
├── templates/              # 用户自定义模板（首次运行拷贝自 resources/templates/）
│   └── *.tex
└── drafts/                 # 自动保存的草稿
    └── *.json
```

程序配置通过 `QSettings` 存储，包括：
- `xelatex/path`, `pdftocairo/path`, `inkscape/path`, `svg/tool`, `paths/texinputs`, `png/dpi`
- `editor/fontSize` — 代码字体大小
- `ui/fontSize` — 界面字体大小
- `behavior/autoCompileOnSave` — 保存后自动编译（默认开启）
- `behavior/threadCount` — 批量预览并行线程数（默认 6）
- `behavior/wrapLongLines` — 过长代码自动换行（默认开启）
- `shortcuts/copyCode`, `shortcuts/copyPng`, `shortcuts/copySvg`
- `shortcuts/compile`, `shortcuts/applyParams`, `shortcuts/save`, `shortcuts/closeTab`
- `shortcuts/globalHotkey` — 全局快捷键

---

## 设置面板

工具栏"设置"按钮打开设置对话框，包含以下区域：

**路径设置**：
- xelatex / pdftocairo / inkscape 命令（默认从 `$PATH` 查找，可填绝对路径）
- SVG 转换工具选择：pdftocairo 或 inkscape
- **命令搜索路径**：额外的可执行文件目录（多个用冒号分隔）。这些目录会被加入所有子进程的 `PATH`（同时也加入 `TEXINPUTS`），用于查找 `xelatex`/`pdftocairo`/`inkscape` 等命令。**从桌面图标启动、系统 `PATH` 不含 TeX Live 时尤其有用**——例如 TeX Live 用户可填入 `/usr/local/texlive/2025/bin/x86_64-linux`
- PNG DPI（72–1200，默认 300）
- 代码字体大小（8–48，默认 10）
- 界面字体大小（8–48，默认 10）— 影响左栏分类树、缩略图名称及全局界面字体

> **提示**：桌面环境（尤其是 Wayland/systemd 会话）启动 GUI 程序时的 `PATH` 往往比终端里的精简，可能不含 `/usr/local/texlive/.../bin`。若启动时提示「未找到依赖工具 xelatex」，在此处填入 TeX Live 的 `bin` 目录即可（无需登出或修改系统环境）。

**快捷键设置**：
- 全部 8 项操作均可自定义键序列
- 清空键序列 = 禁用该快捷键
- 按 Delete 键或点击清除按钮可清空

**行为设置**：
- **保存后自动编译** — 开启后点击保存按钮（或按保存快捷键）时自动触发编译并刷新 PDF 预览，无需手动点击"编译预览"。默认开启。
- **编译线程数** — 设置批量预览时并行编译的线程数（1–32，默认 6）。每个线程独立创建 LaTeX 编译器实例，互不干扰。
- **过长代码自动换行** — 开启后过长的代码行在编辑器内自动软换行显示，无需横向滚动；折行产生的续行在左侧行号栏以 `↳` 标记，便于区分新行与续行。默认开启。
- **编译状态指示** — 状态栏左侧显示编译结果（绿色"编译成功" / 红色"编译失败，详见日志"），3 秒后自动消失

**模板管理**：
- 左侧列表展示所有 `.tex` 模板
- 右侧代码编辑区可编辑选中模板
- +/- 按钮创建 / 删除模板
- 模板中必须包含 `%%% TIKZ_CODE_HERE %%%` 占位符

**工具**：
- **生成所有预览** — 多线程并行遍历全部片段编译生成 PDF + 缩略图 PNG（线程数由设置控制，每片段 30 秒超时，路径计算在主线程预完成确保线程安全，状态栏实时进度）。编译完成后自动弹出报告窗口，统计成功/失败数量，列出失败片段详情及编译错误
- **重置所有内容** — 删除所有用户数据，需输入"确定重置"二次确认（高危操作）

---

## 命令行构建

```bash
# 基本构建（自动检测 KGlobalAccel）
cmake -B build
cmake --build build -j$(nproc)

# 构建并运行
./build/hitikz

# 运行测试
cd build && ctest --output-on-failure

# 安装 / 卸载（详见「安装与卸载」）
sudo cmake --install build
sudo cmake --build build --target uninstall
```

CMake 选项：

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `WITH_KGLOBALACCEL` | ON | 启用 KDE KGlobalAccel |
| `WITH_QHOTKEY` | ON | KGlobalAccel 不可用时回退 QHotkey |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | 安装前缀（用户自编译程序的惯用位置） |

---

## 项目架构

> 为便于维护，多个体量较大的源文件已按职责拆分为多个翻译单元（共用同一头文件，C++ 允许类成员函数实现分散在多个 `.cpp` 中）。

```
src/
├── main.cpp                         # 入口（QApplication 初始化，版本号由 APP_VERSION 宏注入）
│
│   # ── 主窗口（共用 mainwindow.h，按职责拆分）──
├── mainwindow.h                     # MainWindow 类声明
├── mainwindow.cpp                   # 核心：窗口框架、UI 搭建、信号连接、片段加载/保存
├── mainwindow_internal.h            # 各实现单元共用的内部常量
├── mainwindow_tabs.cpp              # 多标签页管理（含 TabUiState 标签页间元数据隔离）
├── mainwindow_compile.cpp           # 编译流程、日志格式化、预览持久化、批量生成
├── mainwindow_params.cpp            # 参数化系统
├── mainwindow_shortcuts.cpp         # 快捷键与全局热键
├── mainwindow_drafts.cpp            # 自动保存与草稿恢复（含标签页隔离的 UI 状态持久化）
│
│   # ── 左栏组件（共用 search_panel.h）──
├── search_panel.h / search_panel.cpp   # 核心：搜索框、分类树（前缀边界感知，`数学` 不误包含 `数学分析`）、缩略图
├── search_panel_tags.cpp            # 标签过滤器
├── search_panel_menus.cpp           # 右键菜单与拖拽
│
│   # ── 数据层（共用 snippet_manager.h）──
├── snippet_manager.h / snippet_manager.cpp # 核心 CRUD、JSON、分类、缓存、批量操作
├── snippet_manager_search.cpp       # 模糊搜索（双字索引）与分类统计
├── snippet_manager_io.cpp           # 存档导入/导出（tar.gz）
│
│   # ── 编译引擎（共用 latex_compiler.h）──
├── latex_compiler.h / latex_compiler.cpp # 核心：编译流程、取消、行号映射
├── latex_compiler_wrap.cpp          # 模板加载与代码包装
├── latex_compiler_extract.cpp       # 自定义命令抽取（40+ 类定义命令，遇不完整定义安全跳过不中断解析）
├── latex_compiler_convert.cpp       # PNG/SVG 转换与工具可用性检测
│
├── code_editor.h / .cpp             # 编辑器（行号+续行 ↳ 标记、当前行/选中词高亮、自动缩进、过长行换行、括号自动配对（含行内公式 `$…$`）、补全/高亮器集成）
├── tikz_highlighter.h / .cpp        # TikZ/LaTeX 语法高亮（14 条正则规则 + 花括号深度感知的跨行选项括号 + 跨行 key=value + 6 类用户定义动态高亮）
│
│   # ── 智能代码补全（共用 tikz_completer.h）──
├── tikz_completer.h / tikz_completer.cpp # 核心：补全触发、按键处理、跨行上下文回溯、花括号感知的 key 提取
├── tikz_completer_context.cpp       # 上下文检测（12 种上下文）
├── tikz_completer_models.cpp        # 补全模型构建与更新（环境/命令/库三级过滤 + 用户定义 + 智能值提示）
│
│   # ── 结构化关键词库（共用 tikz_keywords.h）──
├── tikz_keywords.h                  # TikzKeywordDB 类声明
├── tikz_keywords.cpp                # DB 方法（filter / find / names / valueHints ...）
├── tikz_keywords_data.h / .cpp      # 注册入口 registerAllBuiltins + 共用辅助函数
├── tikz_keywords_basic.cpp          # 颜色/线宽/线型/箭头/形状/图案/装饰/锚点/处理器/PGF路径/库/环境
├── tikz_keywords_commands.cpp       # 命令 + CircuiTikZ 路径元件（358 个）+ physics/siunitx 命令 + tkz-euclide + 数学函数（源自 PGF 数学引擎）
├── tikz_keywords_options.cpp        # 通用选项
├── tikz_keywords_pgfplots.cpp       # 3dplot / pgfplots / 杂项形状选项
├── tikz_keywords_extended.cpp       # CircuiTikZ 形状/选项、tikz-cd、chemfig、tikz-feynman、graphs、pgfplots 扩展等
│                                    #（合计约 2200+ 条目：CircuiTikZ 元件358+形状128 + tkz-euclide 261 + chemfig + tikz-feynman + tikz-cd + 形状~120 + 装饰 ~30 + 装饰子选项 35+ + 库门控键 80+ + PGF键路径30+）
├── tikz_document_state.h / .cpp     # 文档状态追踪（范围栈、库解析、`\usepackage` → 活动库映射（circuitikz/tkz-euclide/tikz-cd/chemfig/tikz-feynman/physics/siunitx/pgfplots/tikz-3dplot）、用户样式/坐标/节点/pic/foreach(含可选参数)/颜色/命令/命名路径(name path)/交点名(by=) 解析）
├── tikz_words.h                     # TikZ 词库兼容层（委托到 TikzKeywordDB，含 tikzPathOperations 精选路径操作集合、latexPackages 常用宏包名）
├── comma_list_completer.h           # 逗号分隔字段（额外宏包 / TikZ库）的分段自动补全器（仅补全最后一段，保留前序条目）
├── flow_layout.h / flow_layout.cpp  # 流式布局组件（支持自动换行，用于标签过滤器）
├── pdf_preview_widget.h / .cpp      # PDF 预览组件（缩放/平移/适应模式，从 MainWindow 抽出）
├── settings_dialog.h / .cpp         # 设置面板（路径/行为/快捷键/模板管理/工厂重置）
├── snippet_properties_dialog.h / .cpp # 片段属性编辑对话框
├── kde_global_shortcut.h / .cpp     # KDE KGlobalAccel 全局快捷键（或 QHotkey 回退）
resources/
├── templates/                       # 出厂模板（3 个极简模板）
│   ├── default_math.tex            # tikz, amsmath, xcolor
│   ├── default_physics.tex         # tikz, xcolor
│   └── default_circuit.tex         # tikz, xcolor, circuitikz, preview
└── presets/                         # 出厂预置片段（120 个）
    └── <uuid>/
        ├── meta.json
        └── snippet.tex
tests/
├── test_snippet_manager.cpp         # CRUD 操作 + ZIP 导入/导出测试
├── test_latex_compiler.cpp          # 编译 + PNG/SVG 转换测试
├── test_search.cpp                  # 模糊搜索算法 + 分类 + 标签过滤测试
├── test_packages_libraries.cpp      # 宏包/TikZ库解析与模板注入测试
├── test_highlighter_regex.cpp       # 语法高亮正则表达式正确性测试（数学模式、注释、命令等）
├── test_multitab.cpp                # 多标签页功能测试（创建/切换/关闭/去重）+ 编辑器过长行换行切换
├── test_draft_recovery.cpp          # 草稿格式完整性 + 目录扫描测试
├── test_tex_import.cpp              # .tex 文件导入代码提取测试
├── test_params.cpp                  # 参数声明解析与替换测试
├── test_fixes.cpp                   # 关键修复验证（行号正则、注释优先级、QProcess、数据流）
├── test_completer.cpp               # 补全词库完整性 + detectContext 12 种上下文 65 用例（含 \end{}、花括号内 = 值补全）+ 箭头大小写双变体 + 定位键坐标补全 + 装饰完整列表 + 库门控 + label/pin 方位 + font/node font 字体值（含样式体内）+ to path 与坐标宏 + graphs/matrix 选项 + of= 命名路径 + 含空格补全上屏无多余字符 + CircuiTikZ/tikz-cd/tkz-euclide/chemfig/tikz-feynman 源码一致校验
├── test_document_state.cpp          # 文档状态追踪（20 个用例：范围/库/样式/坐标/pic/foreach(含可选参数)/颜色/\usepackage 激活 physics·siunitx·pgfplots/节点 name= 选项语法提取/命名路径 name path/by= 交点名提取）
└── test_enhanced_highlighter.cpp    # 增强语法高亮（14 个用例：PGF路径/处理器/用户定义名/key=value花括号深度/跨行选项/综合）

# ── 打包与安装 ──
cmake/
└── cmake_uninstall.cmake.in         # uninstall 目标使用的模板（按 install_manifest.txt 卸载）
hitikz.desktop                       # 桌面入口（Exec=hitikz，Icon=hitikz）
hitikz.svg                           # 应用图标（scalable）
```

### 核心类关系

```
MainWindow
├── SearchPanel          ← SnippetManager
│   ├── QLineEdit        搜索框（150ms 防抖）
│   ├── FlowLayout       标签过滤器（流式按钮、2行折叠、弹出对话框）
│   ├── QTreeView        分类树（含"未分类"节点）
│   ├── QSplitter        可拖动分隔条（分类树 / 缩略图）
│   └── QListView        缩略图网格（ExtendedSelection 多选）
├── QTabWidget           多标签页容器
│   ├── CodeEditor       代码编辑器（每个标签页一个实例）
│   │   ├── TikzHighlighter  语法彩色高亮（14 条规则 + 用户定义）
│   │   ├── TikzCompleter    智能补全（12 种上下文 + 三级过滤 + 智能值提示 + 上下文感知命令 + CircuiTikZ/tkz-euclide 支持 + 环境自动闭合）
│   │   ├── TikzDocumentState 文档状态追踪（300ms 防抖）
│   │   └── LineNumberArea   行号显示
│   └── ...（更多标签页）
├── QPlainTextEdit       编译日志面板（精简过滤、行号映射、彩色格式化）
├── QSplitter（垂直）    PDF 预览与元数据区可调分割
│   ├── PdfPreviewWidget PDF 矢量预览（缩放/平移/适应，独立组件）
│   └── QScrollArea      元数据编辑表单 + 参数控件
├── LatexCompiler        编译引擎（xelatex + pdftocairo/inkscape SVG转换 + 嵌套括号解析 + 行号映射 + 自定义命令抽取/注入）
├── SnippetManager       数据层（JSON 读写、双字索引搜索、分类缓存、批量操作）
├── SettingsDialog       设置面板（路径/快捷键/模板管理/工厂重置）
└── KdeGlobalShortcut    KDE 全局快捷键（或 QHotkey 回退）
```

### 编辑器信号关键路径

```
光标移动    → highlightCurrentLine()
             → ExtraSelection（当前行 + 选中词全部出现位置）

用户输入    → keyPressEvent()
              → handleCompletionKey()（Enter/Tab 上屏，↑↓ 导航；`\begin{env}` 完成后自动插入 `\end{env}`）
              → 括号自动配对（`{}`/`[]`/`()`/`$$` 双符插入、选中包裹、闭合跳过、空对退格删除）
              → QPlainTextEdit::keyPressEvent()
              → tryComplete()（上下文检测 → 切换 Completer）
              → parseParams()（扫描 @param → 更新参数控件 + 参数补全词库）

 文本变更    → textChanged
               → (m_loadingDepth == 0) → onCurrentSnippetChanged() → parseParams()
               → autoSaveTimer（180s） → performAutoSave()

标签切换    → onTabChanged() → setEditorForTab() → loadPreview + loadMetadata + parseParams()
```

---

## 测试

项目包含十三套自动化测试（通过 CTest 运行）：

| 测试 | 内容 |
|------|------|
| `test_snippet_manager` | 片段创建/读取/更新/删除，loadCode，renameCategory，compileCommand JSON 序列化，ZIP 导入/导出往返 |
| `test_latex_compiler` | xelatex 可用性检测，基本编译，PDF 生成，PNG 转换，SVG 转换，错误编译日志验证，自定义命令抽取（覆盖 14 类定义命令）、带分隔符参数的 `\def` 与参数后换行的 `\newcommand` 抽取、定义跳过后继续解析、析构回收转换子进程、compileCommand 自定义引擎编译与 lastFullCommand 验证（53+ 项测试） |
| `test_search` | 精确匹配、子序列匹配、连续加分、中文搜索、标签过滤、分类统计 |
| `test_packages_libraries` | 宏包字符串解析（含嵌套括号选项），TikZ 库解析，模板注入正确性，往返序列化 |
| `test_highlighter_regex` | 数学模式 `$...$`、`\(...\)`、`\[...\]` 正则表达式匹配验正 |
| `test_multitab` | 多标签页功能：创建/切换/关闭标签页、重复打开去重、关闭前未保存检查、编辑器过长行自动换行切换、元数据脏标记检测、**切换标签页元数据隔离与标题正确性**（切换更新右栏元数据、后台标签标题不被污染、未保存编辑跨切换保留）、UI 库栏改动即时同步到编辑器补全（`circle through` 等）、`$` 行内公式自动配对（插入/包裹选中/跳过闭合/空对退格）、`{|}` 独占行回车三行拆分、`Tab`/`Shift+Tab` 多行块缩进与反缩进、标签过滤器移除末位标签后自动取消选中并重新过滤、标签集合未变时不重建标签栏（保存不闪烁）、额外宏包 / TikZ库字段的逗号感知分段补全器 |
| `test_draft_recovery` | 草稿文件格式完整性（全字段 JSON 读写往返、空代码过滤、目录扫描） |
| `test_tex_import` | .tex 文件导入代码提取（三段式解析）及宏包/TikZ 库声明解析 |
| `test_params` | 参数声明正则解析与 `@@var@@` 替换正确性（单参数、多参数、负数、零值） |
| `test_fixes` | 关键修复验证：行号正则锚定、注释优先级、wrapCode 无 document 前置注入、QProcess 启动检测、数据流状态检查、自动编译设置、短命令行解析、原子文件重命名、草稿清理、路径遍历字符检测、导入失败目录回滚（11 项测试） |
| `test_completer` | 补全词库完整性（去重、条目数量）、detectContext 12 种上下文检测（65 用例含 `\end{}` 上下文、花括号内 `=` 值补全）、箭头大小写双变体（`stealth`/`Stealth`）、定位键坐标补全、PGF 键处理器 / 值提示 / `=` 颜色值补全、key 提取的花括号 / 中括号感知、跨行上下文回溯、装饰完整 27 名 eq 补全、锚点源码准确性、库门控（circuitikz 锚点开/关）、label/pin 方位值、font/node font 字体命令值（含 `.style={font=\|}` 样式体内）、`to path` 通用选项与 `\tikztostart`/`\tikztotarget`/`\tikztonodes` 坐标宏、graphs 库选项与 `\matrix` 的 `column sep`/`row sep` 取值提示（多库同名键取值并集）、`of=` 命名路径补全、含空格补全值上屏不多字符等；CircuiTikZ / 标准 TikZ / 数学函数 / 装饰 / physics·siunitx / pgfplots / tikz-cd·tkz-euclide·chemfig·tikz-feynman 各词库组的源码一致性校验，确保所有条目为对应宏包的真实命令/选项/形状，并仅由正确的库或环境门控 |
| `test_document_state` | 文档状态追踪：范围栈检测、库解析、用户样式（含空格名）/坐标/节点/pic名（含 `name=` 选项语法）/路径操作坐标 `coordinate (name)`/foreach变量（含空格分隔多变量、>2个变量、可选方括号参数）/颜色/命令解析、命名路径（`name path`/`global`/`local`）与 `by=` 交点名提取、环境名查询、片段库注入、`\usepackage{physics,siunitx,pgfplots,chemfig,tikz-feynman}` 激活对应补全库（21 个测试用例） |
| `test_enhanced_highlighter` | 增强语法高亮：PGF路径/键处理器/库规则不崩溃，用户样式/节点名/foreach变量高亮检测，key=value分色（含花括号深度隔离），跨行选项括号高亮，多行注释，综合测试，foreach空格分隔变量，**注释保护**验证（14 个测试用例） |

运行测试：
```bash
cd build && ctest --output-on-failure
```

全部 13 套测试通过，方可提交。

---

## 技术栈

| 技术 | 用途 |
|------|------|
| **C++17** | 核心语言 |
| **Qt 6** | Widgets (UI), Gui (QSyntaxHighlighter), Pdf (PDF 渲染), PdfWidgets (QPdfView) |
| **CMake** | 构建系统 |
| **KF6GlobalAccel** | KDE 原生全局快捷键 |
| **QHotkey** (fallback) | 非 KDE 环境的全局快捷键 |
| **XeLaTeX** | LaTeX → PDF 编译 |
| **pdftocairo** | PDF → PNG / SVG 转换（默认） |
| **Inkscape** | PDF → SVG 转换（备选，可在设置中切换） |
| **JSON** | 片段元数据格式 |
| **tar / unzip** | 存档导入/导出 |

---

## 已知问题

### 1. SVG 剪贴板粘贴到 Inkscape 样式变化

通过工具栏"复制SVG"将 SVG 粘贴到 Inkscape 后图片样式发生变化，而导出为 SVG 文件再在 Inkscape 中打开则正常。此问题与使用的转换工具（pdftocairo 或 Inkscape）无关，剪贴板粘贴路径本身会导致样式丢失。

- **现象**：粘贴后的图形颜色、线条或布局与原始预览不一致
- **临时规避**：使用"导出为 SVG 图片"保存为文件后，在 Inkscape 中通过"文件 → 打开"或"文件 → 导入"打开该文件

### 2. 部分不规范的 TikZ 代码可能导致编译卡死

某些格式错误的 TikZ 代码（如递归定义、无限循环等）可能让 XeLaTeX 陷入死循环。此时可点击工具栏"⛔ 强制结束"按钮立即中断编译进程，避免计算机风扇狂转。

### 3. 多行选项中个别 token 的颜色会被后续规则覆盖

`applyOptionBrackets` 先将跨行 `[...]` 整体标记为选项色（青色），但后续正则规则（如命令规则匹配 `\line`、`\width`）会覆盖个别 token 的颜色。这属于优先级设计——特定语法元素（命令、数字等）的颜色优先于选项底色。每个连续行的闭合 `]` 和行间选项整体识别不受影响。
