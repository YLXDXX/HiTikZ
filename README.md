# HiTikZ — TikZ 代码合集管理器

> 面向 Linux (KDE 6 / Wayland) 的 TikZ/PGF 矢量图形管理工具。
> 创建、编辑、预览、搜索、导出 TikZ 图像并支持批量操作。
> 当前版本：1.0（2026-07-01 更新）

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

- **TikZ 片段管理**：创建、编辑、保存、删除 TikZ 代码片段，附带名称 / 简介 / 分类 / 标签 / 宏包 / TikZ 库 / 模板元数据
- **语法高亮**：TikZ/LaTeX 代码彩色高亮（命令蓝色、环境紫色、注释灰色、坐标橙色、数学模式 `$...$` / `\(...\)` / `\[...\]` 绿色等 12 类语法元素），支持 `\begin{comment}...\end{comment}` 多行注释块
- **选中词高亮**：光标置于某单词或双击选中时，文档中所有相同单词自动高亮，快速定位变量/命令的使用位置
- **智能代码补全**：9 种上下文感知补全（命令 `\`、环境 `\begin{`、选项 `[...]`、锚点 `.`、值 `=`、参数 `@@`、库 `\usetikzlibrary{` 等），含值提示（颜色、线宽、箭头等），覆盖 CircuitikZ 电路元件、tikz-3dplot、标准 LaTeX 数学符号等
- **自动缩进**：换行时继承上一行缩进，`{` 或 `\begin` 结尾的行自动增加一级缩进
- **多标签编辑器**：支持同时打开多个 TikZ 片段，通过标签页快速切换，重复打开同一片段自动切换到已有标签页
- **撤销/重做**：支持 Ctrl+Z / Ctrl+Shift+Z，工具栏以 ↩ ↪ 符号表示
- **实时编译预览**：调用系统 `xelatex -shell-escape` 编译片段，通过 `QPdfView` 实时渲染高清 PDF 矢量预览
- **简化的编译日志**：仅显示编译命令、错误（带上下文）、警告信息，过滤冗余的文件加载等噪音行，错误行号自动映射到编辑器行号
- **可拆分预览面板**：右侧 PDF 预览与元数据编辑区之间可拖动分割条，PDF 可放大至占满整个面板
- **适应式缩放**：支持适应整页 / 适应宽度 / 适应高度三种显示模式，滚轮缩放以鼠标位置为中心，左键拖拽平移
- **重编译保持视口**：重新编译后自动保持 PDF 的缩放倍率和滚动位置，继续查看修改前的同一位置，方便对比细节变化
- **预览持久化**：编译成功后自动保存 PDF 和缩略图 PNG，下次切换片段即时展示预览
- **批量预览生成**：设置面板中一键异步生成所有片段预览（队列式编译，状态栏实时进度，单片段 30 秒超时自动跳过错片）
- **彩色日志面板**：错误行红色高亮、警告橙色，双击错误行跳转到编辑器对应行
- **模糊搜索**：Unicode 子序列匹配，双字索引加速，支持名称和简介搜索，连续匹配高分、间隔扣分
- **树形分类**：支持层级分类（如 `数学/几何`），含"未分类"节点，拖拽片段到分类节点即可重新分类
- **多选批量操作**：Ctrl+点击多选缩略图，右键弹出批量导出 / 改分类 / 删除菜单，支持全选、导出全部
- **属性编辑对话框**：右键缩略图（单选时）弹出属性对话框，可编辑全部元数据字段
- **参数化功能**：通过 `% @param: var=默认值` 声明变量，代码中使用 `@@var@@` 占位，右栏动态生成参数控件，编辑器内 `@@` 可触发参数补全。scratch 模式（无已保存片段）下同样支持参数替换编译
- **保存后自动编译**：可在设置中开启，点击保存按钮或按保存快捷键后自动编译并刷新 PDF 预览
- **自动保存**：每 3 分钟自动保存所有打开标签页的完整状态（代码 + 名称 + 简介 + 标签 + 宏包 + TikZ 库 + 模板）为 JSON 草稿文件
- **草稿恢复**：程序异常退出后重启时，自动弹出恢复对话框，列出所有未保存草稿的具体名称，可勾选需恢复的草稿、全选/取消全选、或一键全部丢弃，恢复的草稿自动创建为片段或关联到已有片段
- **模板系统（极简）**：三个内置 LaTeX 模板仅含必要宏包（数学 / 物理 / 电路），额外宏包和 TikZ 库由每个片段自行声明
- **完整文档复制**：一键复制含模板头部 + 片段的完整可编译 LaTeX 文档（现更名为"复制文档"）
- **格式导出**：编译生成的 PDF 可转换并复制为 PNG/SVG 到剪贴板，也可直接导出为 `.tex` / `.pdf` / `.png` / `.svg` 文件
- **导入 .tex 文件**：支持导入单个 `.tex` 文件，自动提取 TikZ 代码（三段式回退）并从导言区解析 `\usepackage` 宏包和 `\usetikzlibrary` 库声明，文件名作为片段名称
- **存档导入/导出**：片段以 tar.gz 格式打包，支持单选 / 多选批量 / 全部导出，同时支持导出为独立 .tex 文档、PDF、PNG、SVG 格式
- **标签过滤**：搜索框下方自动展示所有片段的标签徽章，点击筛选，多标签 AND 组合；大量标签时自动折叠为两行并通过弹出窗口选择
- **系统托盘**：最小化到托盘，全局快捷键一键呼出/隐藏，托盘菜单"退出"触发关闭前保存提示
- **可配置快捷键**：全部快捷键可在设置面板中自定义键序列，支持清空禁用
- **全局快捷键（KDE）**：KDE 桌面通过 KGlobalAccel 注册系统级快捷键
- **代码字体调节**：设置面板中可调代码编辑区字体大小（8-48 pt）
- **依赖检测**：启动时检测 `xelatex` 和当前配置的 SVG 转换工具（`pdftocairo` 或 `inkscape`），缺失时弹出安装指引
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
| **pdftocairo** | poppler 工具集（默认 SVG 转换工具） |
| **Inkscape** | （可选）备选 SVG 转换工具，在设置面板中切换 |
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
| 导入/导出 ▼ | 下拉菜单：导入存档 / 导入 .tex 文件 / 导出当前 / 导出全部 / 导出为 Tex 文档 / 导出为 PDF 文档 / 导出为 PNG 图片 / 导出为 SVG 图片 |
| ↩ / ↪ | 撤销（Ctrl+Z）/ 重做（Ctrl+Shift+Z） |
| 编译预览 | 保存并编译 TikZ 代码渲染 PDF（自动应用参数替换） |
| 应用参数 | 不保存，仅用参数值替换后编译（用于临时预览参数效果） |
| 保存 | 保存当前片段（若在设置中开启"保存后自动编译"，则保存后自动触发编译） |
| 复制代码 | 复制参数替换后的 TikZ 核心代码 |
| 复制文档 | 复制含模板头部的完整 LaTeX 文档 |
| 复制PNG | 复制 300 DPI PNG 到剪贴板 |
| 复制SVG | 复制 SVG 到剪贴板 |
| 适应整页/宽度/高度 | PDF 显示模式（可选中态） |
| − / + | PDF 缩小/放大 |
| 设置 | 打开设置面板 |

### 左栏

- **搜索框**：输入关键词实时搜索（150ms 防抖延迟），按名称 + 简介模糊匹配
- **标签过滤器**：搜索框下方的流式布局标签徽章，点击标签筛选片段（AND 逻辑），选中标签蓝色高亮。标签过多时自动折叠为最多两行，点击"更多标签..."弹出对话框展示全部标签（同样使用流式布局方便浏览）
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
  - 切换标签页时自动更新右侧的 PDF 预览、元数据表单和参数面板
  - 退出程序时检查所有标签页的未保存更改，支持全部保存/全部放弃
- **代码编辑器**：基于 `QPlainTextEdit`，等宽字体（字号可调，默认 10pt），行号显示，当前行高亮，**TikZ/LaTeX 语法彩色高亮**，**选中词全文档高亮**，**智能代码补全**
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
~/.local/share/TikzManager/
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
  "tikzLibraries": ""
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

- **语法高亮**（`TikzHighlighter`）：基于 `QSyntaxHighlighter`，12 条优先正则规则
  - 注释 `%` → 灰色斜体
  - 字符串 `"..."` → 橙黄色
  - 参数 `@@var@@` → 绿色斜体
  - 环境 `\begin{...}` / `\end{...}` → 紫色加粗
  - 命令 `\draw`、`\node` 等 → 蓝色加粗
  - 数学模式 `$...$` → 绿色
  - 坐标 `(x,y)` → 橙色
  - 选项 `[...]` → 青色
  - 数字（含单位）→ 紫色
  - 括号 `{` `}` → 红色

- **选中词高亮**：光标置于单词上或双击选中时，文档中所有相同单词以浅橙色背景高亮，辅助查看变量/命令的使用位置

- **自动缩进**：按 Enter 换行时自动继承上一行的前导空白，若行尾为 `{` 或以 `\begin` 开头则额外增加 4 空格缩进

- **撤销/重做**：标准 Ctrl+Z / Ctrl+Shift+Z，工具栏有独立按钮

### 代码补全

智能补全（`TikzCompleter`）根据光标上下文自动给出建议，覆盖 9 种场景：

| 上下文 | 触发条件 | 补全内容 | 示例 |
|--------|---------|---------|------|
| 命令 | 输入 `\` 后接字母 | ~300 个 TikZ/LaTeX 命令 | `\draw`、`\node`、`\tdplotsetmaincoords`、`\pgfmathsetmacro` |
| 环境 | `\begin{` 未闭合 | ~70 个 LaTeX/TikZ 环境 | `tikzpicture`、`scope`、`axis`、`groupplot`、`matrix` |
| 选项 | `[...]` 内 | ~250 个 TikZ 选项 | `thick`、`left color`、`wiper pos`、`tdplot_main_coords` |
| 锚点 | 单词后跟 `.` | ~80 个节点锚点 | `north`、`south east`、`wiper`、`cathode`、`anode` |
| 值 | `=` 后 | 颜色 / 线型 / 箭头的值提示 | 输入 `color=` 后列出全部颜色名 |
| 参数 | `@@` 未闭合 | 代码中声明的参数变量 | 输入 `@@` 后显示 `angle`、`width` 等 |
| TikZ 库 | `\usetikzlibrary{` 内 | ~120 个 TikZ 库名 | `calc`、`arrows.meta`、`circuitikz`、`3d`、`shapes.gates.logic` |
| 通用词 | 输入任意已知词 | 命令、选项、锚点、颜色的并集 | 连续输入 2 个以上字母时触发 |

**补全交互**：
- 自动弹出：检测到对应上下文时自动显示补全列表
- 默认选中第一个候选项
- ↑↓ 键切换选中项，Enter / Tab 上屏
- Esc 关闭列表
- 上下文检测使用轻量前缀扫描，将来可替换为完整 AST 扫描器

**补全覆盖**：
- **CircuitikZ**：常用电路元件名称（`resistor`、`capacitor`、`potentiometer`、`battery` 等）作为选项/命令补全；专用锚点（`wiper`、`cathode`、`anode`、`gate`、`tap` 等）作为锚点补全
- **tikz-3dplot**：完整 25 个官方命令（`\tdplotsetmaincoords`、`\tdplotsetrotatedcoords`、`\tdplotdrawarc` 等）和样式键（`tdplot_main_coords`、`canvas is xy plane at z` 等）
- **标准 LaTeX**：希腊字母、数学运算符与符号、定界符、重音、字体命令等

### LaTeX 编译与 PDF 预览

**编译流程**：

1. 加载选中模板，将额外宏包（`\usepackage`）和 TikZ 库（`\usetikzlibrary`）注入模板导言区 `\begin{document}` 前
2. 将 TikZ 核心代码注入模板的 `%%% TIKZ_CODE_HERE %%%` 位置
3. 写入临时 `.tex` 文件（位于 `/tmp/TikzManager/<snippetId>/output.tex`）
4. 异步调用 `xelatex -interaction=nonstopmode -halt-on-error -shell-escape`
5. 编译成功 → PDF 加载到预览区 → 生成缩略图 PNG（150 DPI）→ 持久化到片段目录
6. 编译失败 → 日志面板精简显示错误和警告 → 双击跳转错误行（行号已自动映射到编辑器行号）

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

- **定时保存**：每 3 分钟自动保存所有打开标签页的完整状态（代码 + 名称 + 简介 + 标签 + 额外宏包 + TikZ 库 + 模板 ID）为 JSON 草稿文件
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

解析器正确处理嵌套括号 `{}` 和 `[]`，如 `[option={val1,val2}]` 中的逗号不会错误分割。

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

工具栏"复制文档"按钮将当前片段的模板头部 + 额外宏包 + TikZ 库 + 参数替换后的 TikZ 代码组合成**完整可编译的 LaTeX 文档**复制到剪贴板。

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
| 复制 SVG | 从 PDF 转换 SVG 后复制（附带 `image/svg+xml` MIME 类型），转换工具可在设置中切换 |

### 导入与导出

**导出**：使用异步进程（非阻塞）调用 `tar` 打包，支持多种粒度和格式：
- **导出当前 / 导出全部**：打包为 `.tar.gz` 归档
- **批量导出所选**：多选后右键菜单批量打包
- **导出为 Tex 文档**：生成含模板头部的完整可编译 LaTeX 文档
- **导出为 PDF 文档**：复制预览 PDF 到指定路径
- **导出为 PNG 图片**：通过 pdftocairo 将预览 PDF 转换为 PNG（DPI 遵循设置面板配置）
- **导出为 SVG 图片**：通过 pdftocairo 或 Inkscape（可在设置面板中切换）将预览 PDF 转换为 SVG 矢量图

**导入**：
- **导入存档**：选择 `.tar.gz` 或 `.zip` 文件 → 异步解压并为每个片段分配新 UUID → 刷新列表。导入时自动清除 `isPreset` 标记，若缺失 `meta.json` 则自动创建片段
- **导入 .tex 文件**：选择单个 `.tex` 文件 → 自动提取 TikZ 代码（`\begin{document}...\end{document}` 之间 → `\begin{tikzpicture}...\end{tikzpicture}` → 全文回退三段式解析），同时从导言区解析 `\usepackage{}`（含可选参数 `[...]`）和 `\usetikzlibrary{}` 声明，自动填入片段的宏包和 TikZ 库字段，文件名自动作为片段名称，创建为新片段后加载到编辑器

---

## 键盘快捷键

全部快捷键均可在**设置面板 → 快捷键设置**中自定义键序列，清空则禁用该快捷键。

| 功能 | 默认快捷键 |
|------|----------|
| 全局快捷键：显示/隐藏窗口 | `Ctrl+Alt+T`（KDE 通过 KGlobalAccel 注册） |
| 撤销 | `Ctrl+Z` |
| 重做 | `Ctrl+Shift+Z` |
| 复制 TikZ 代码 | `Ctrl+Shift+C` |
| 复制 PNG | `Ctrl+Shift+P` |
| 复制 SVG | `Ctrl+Shift+S` |
| 编译预览 | 无（可在设置中自定义） |
| 应用参数 | 无（可在设置中自定义） |
| 保存 | 无（可在设置中自定义） |
| 关闭标签页 | `Ctrl+W` |

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
├── templates/              # 用户自定义模板（首次运行拷贝自 resources/templates/）
│   └── *.tex
└── drafts/                 # 自动保存的草稿
    └── *.json
```

程序配置通过 `QSettings` 存储，包括：
- `xelatex/path`, `pdftocairo/path`, `inkscape/path`, `svg/tool`, `paths/texinputs`, `png/dpi`
- `editor/fontSize` — 代码字体大小
- `ui/fontSize` — 界面字体大小
- `behavior/autoCompileOnSave` — 保存后自动编译（默认关闭）
- `shortcuts/copyCode`, `shortcuts/copyPng`, `shortcuts/copySvg`
- `shortcuts/compile`, `shortcuts/applyParams`, `shortcuts/save`, `shortcuts/closeTab`
- `shortcuts/globalHotkey` — 全局快捷键

---

## 设置面板

工具栏"设置"按钮打开设置对话框，包含以下区域：

**路径设置**：
- xelatex / pdftocairo / inkscape 命令（默认从 `$PATH` 查找）
- SVG 转换工具选择：pdftocairo 或 inkscape
- 额外环境变量
- PNG DPI（72–1200，默认 300）
- 代码字体大小（8–48，默认 10）
- 界面字体大小（8–48，默认 10）

**快捷键设置**：
- 全部 8 项操作均可自定义键序列
- 清空键序列 = 禁用该快捷键
- 按 Delete 键或点击清除按钮可清空

**行为设置**：
- **保存后自动编译** — 开启后点击保存按钮（或按保存快捷键）时自动触发编译并刷新 PDF 预览，无需手动点击"编译预览"

**模板管理**：
- 左侧列表展示所有 `.tex` 模板
- 右侧代码编辑区可编辑选中模板
- +/- 按钮创建 / 删除模板
- 模板中必须包含 `%%% TIKZ_CODE_HERE %%%` 占位符

**工具**：
- **生成所有预览** — 异步队列遍历全部片段编译生成 PDF + 缩略图 PNG（每片段 30 秒超时，状态栏实时进度）
- **重置所有内容** — 删除所有用户数据，需输入"确定重置"二次确认（高危操作）

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
├── search_panel.h / search_panel.cpp # 左栏组件（搜索框、分类树、缩略图、多选/右键菜单）
├── snippet_manager.h / .cpp         # 数据层（JSON CRUD、模糊搜索含双字索引、分类缓存、批量操作）
├── latex_compiler.h / .cpp          # 编译引擎（xelatex + pdftocairo/inkscape + 宏包/库注入）
├── code_editor.h / .cpp             # 编辑器（行号、当前行+选中词高亮、自动缩进、补全/高亮器集成）
├── tikz_highlighter.h / .cpp        # TikZ/LaTeX 语法高亮（QSyntaxHighlighter 子类，12 条优先规则）
├── tikz_completer.h / .cpp          # 智能代码补全（9 种上下文检测 + 多 QCompleter + 值提示）
├── tikz_words.h                     # TikZ 关键词库（命令/环境/选项/锚点/颜色/箭头/线型/值提示）
├── flow_layout.h / flow_layout.cpp  # 流式布局组件（支持自动换行，用于标签过滤器）
├── pdf_preview_widget.h / .cpp      # PDF 预览组件（缩放/平移/适应模式，从 MainWindow 抽出）
├── settings_dialog.h / .cpp         # 设置面板（路径/快捷键/模板管理/工厂重置）
├── snippet_properties_dialog.h / .cpp # 片段属性编辑对话框
├── kde_global_shortcut.h / .cpp     # KDE KGlobalAccel 全局快捷键（或 QHotkey 回退）
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
├── test_search.cpp                  # 模糊搜索算法 + 分类 + 标签过滤测试
├── test_packages_libraries.cpp      # 宏包/TikZ库解析与模板注入测试
├── test_highlighter_regex.cpp       # 语法高亮正则表达式正确性测试（数学模式、注释、命令等）
├── test_multitab.cpp                # 多标签页功能测试（创建/切换/关闭/去重）
├── test_draft_recovery.cpp          # 草稿格式完整性 + 目录扫描测试
├── test_tex_import.cpp              # .tex 文件导入代码提取测试
├── test_params.cpp                  # 参数声明解析与替换测试
└── test_fixes.cpp                   # 关键修复验证（行号正则、注释优先级、QProcess、数据流）
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
│   │   ├── TikzHighlighter  语法彩色高亮（12 条规则）
│   │   ├── TikzCompleter    智能补全（9 种上下文）
│   │   └── LineNumberArea   行号显示
│   └── ...（更多标签页）
├── QPlainTextEdit       编译日志面板（精简过滤、行号映射、彩色格式化）
├── QSplitter（垂直）    PDF 预览与元数据区可调分割
│   ├── PdfPreviewWidget PDF 矢量预览（缩放/平移/适应，独立组件）
│   └── QScrollArea      元数据编辑表单 + 参数控件
├── LatexCompiler        编译引擎（xelatex + pdftocairo/inkscape SVG转换 + 嵌套括号解析 + 行号映射）
├── SnippetManager       数据层（JSON 读写、双字索引搜索、分类缓存、批量操作）
├── SettingsDialog       设置面板（路径/快捷键/模板管理/工厂重置）
└── KdeGlobalShortcut    KDE 全局快捷键（或 QHotkey 回退）
```

### 编辑器信号关键路径

```
光标移动    → highlightCurrentLine()
             → ExtraSelection（当前行 + 选中词全部出现位置）

用户输入    → keyPressEvent()
             → handleCompletionKey()（Enter/Tab 上屏，↑↓ 导航）
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

项目包含十套自动化测试（通过 CTest 运行）：

| 测试 | 内容 |
|------|------|
| `test_snippet_manager` | 片段创建/读取/更新/删除，loadCode，renameCategory，ZIP 导入/导出往返 |
| `test_latex_compiler` | xelatex 可用性检测，基本编译，PDF 生成，PNG 转换，SVG 转换，错误编译日志验证 |
| `test_search` | 精确匹配、子序列匹配、连续加分、中文搜索、标签过滤、分类统计 |
| `test_packages_libraries` | 宏包字符串解析（含嵌套括号选项），TikZ 库解析，模板注入正确性，往返序列化 |
| `test_highlighter_regex` | 数学模式 `$...$`、`\(...\)`、`\[...\]` 正则表达式匹配验正 |
| `test_multitab` | 多标签页功能：创建/切换/关闭标签页、重复打开去重、关闭前未保存检查 |
| `test_draft_recovery` | 草稿文件格式完整性（全字段 JSON 读写往返、空代码过滤、目录扫描） |
| `test_tex_import` | .tex 文件导入代码提取（三段式解析）及宏包/TikZ 库声明解析 |
| `test_params` | 参数声明正则解析与 `@@var@@` 替换正确性（单参数、多参数、负数、零值） |
| `test_fixes` | 关键修复验证：行号正则锚定、注释优先级、wrapCode 无 document 前置注入、QProcess 启动检测、数据流状态检查、自动编译设置读写 |

运行测试：
```bash
cd build && ctest --output-on-failure
```

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
