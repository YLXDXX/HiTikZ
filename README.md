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

- **TikZ 片段管理**：创建、编辑、保存、删除 TikZ 代码片段，附带名称 / 简介 / 分类 / 标签 / 宏包 / TikZ 库 / 编译命令 / 模板元数据
- **语法高亮**：TikZ/LaTeX 代码彩色高亮，覆盖 **21 类语法元素**（命令/环境/注释/坐标/数学/选项/字符串/数字/参数/PGF路径/键处理器/TikZ库）及 **6 类用户定义名称**（样式/节点/坐标/foreach变量/自定义命令/自定义颜色），支持 `\begin{comment}...\end{comment}` 多行注释块。key=value 中 key 和 value 分色显示。**用户定义高亮保护**——用户样式名、坐标名等出现在注释中时不会覆盖注释的灰色斜体格式
- **选中词高亮**：光标置于某单词或双击选中时，文档中所有相同单词自动高亮，快速定位变量/命令的使用位置
- **智能代码补全**：**上下文感知补全**——根据当前所处环境（tikzpicture/scope/axis/circuitikz）、当前绘图命令（\draw/\node/\path/\fill）、已加载库自动过滤候选项（如 circuitikz 环境下显示电路选项，axis 环境下显示 pgfplots 选项，载入 shadows.blur 库时提示模糊阴影选项，未加载 decorations 库时不提示 decoration 相关选项）；**用户定义补全**——自动识别用户自定义样式（含空格名如 `test lines`）、节点名、坐标名、foreach 变量（支持空格分隔多变量如 `\foreach \xyz / \xtext in`）、自定义命令、自定义颜色、pic 名并纳入补全；**智能值提示**——`=` 后的值根据 key 名动态匹配（如 `color=` 仅提示颜色，`pattern=` 仅提示图案）；覆盖 **~1400+ 个结构化关键词**（含 ~200 CircuiTikZ 组件选项、90 双极前缀组合、71 tkz-euclide v5 命令）；CircuiTikZ 组件通过 `requiredLibs` 库过滤，仅在激活时提示；片段元数据中的 TikZ 库自动传递给补全系统；`\usepackage{circuitikz}` 和 `\usepackage{tkz-euclide}` 自动检测激活对应库
- **自动缩进**：换行时继承上一行缩进，`{` 或 `\begin` 结尾的行自动增加一级缩进
- **多标签编辑器**：支持同时打开多个 TikZ 片段，通过标签页快速切换，重复打开同一片段自动切换到已有标签页
- **撤销/重做**：支持 Ctrl+Z / Ctrl+Shift+Z，工具栏以 ↩ ↪ 符号表示
- **实时编译预览**：调用系统 `xelatex -shell-escape`（默认）编译片段，每个片段可独立指定编译引擎和参数（XeLaTeX / LuaLaTeX / PdfLaTeX），通过 `QPdfView` 实时渲染高清 PDF 矢量预览。遇到死循环代码可随时通过工具栏"强制结束"按钮中断编译
- **简化的编译日志**：仅显示编译命令、错误（带上下文）、警告信息，过滤冗余的文件加载等噪音行，错误行号自动映射到编辑器行号
- **可拆分预览面板**：右侧 PDF 预览与元数据编辑区之间可拖动分割条，PDF 可放大至占满整个面板
- **适应式缩放**：支持适应整页 / 适应宽度 / 适应高度三种显示模式，滚轮缩放以鼠标位置为中心，左键拖拽平移
- **重编译保持视口**：重新编译后自动保持 PDF 的缩放倍率和滚动位置，继续查看修改前的同一位置，方便对比细节变化
- **预览持久化**：编译成功后自动保存 PDF 和缩略图 PNG，下次切换片段即时展示预览
- **批量预览生成报告**：设置中点击"生成所有预览"后，多线程并行编译（线程数可在设置中调整，默认 6，路径计算在主线程预完成以保证线程安全），编译完成自动弹出报告窗口，显示总计/成功/失败数量，失败列表含 ID、名称和格式化编译错误详情
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
- **导入 .tex 文件**：支持导入单个 `.tex` 文件，自动提取 TikZ 代码（三段式回退），从导言区解析 `\usepackage` 宏包、`\usetikzlibrary` 库声明及自定义命令，自动检测模板（含 circuitikz 时自动选用电路模板），文件名作为片段名称
- **从剪贴板导入**：无需手动保存文件，直接粘贴完整 `.tex` 源码即可导入，自动解析宏包、库、自定义命令和模板
- **自定义命令智能处理**：自动识别并提取40余类定义命令（含 `\newcommand`、`\NewDocumentCommand`、`\tikzset`、`\tikzstyle`、`\ctikzset`、`\definecolor`、`\colorlet`、`\contourlength`、`\pgfkeys`、`\pgfplotsset`、`\pgfmathsetmacro`/`\pgfmathsetlength`、`\def`、`\newif`、`\newboolean`/`\setboolean`、`\pgfdeclarelayer`/`\pgfsetlayers`、`\pgfdeclareradialshading`、`\tdplotsetmaincoords`、`\makeatletter`/`\makeatother` 等），编译时自动注入导言区，复制文档时正确编排
- **存档导入/导出**：片段以 tar.gz 格式打包，支持单选 / 多选批量 / 全部导出，同时支持导出为独立 .tex 文档、PDF、PNG、SVG 格式
- **标签过滤**：搜索框下方自动展示所有片段的标签徽章，点击筛选，多标签 AND 组合；大量标签时自动折叠为两行并通过弹出窗口选择
- **系统托盘**：最小化到托盘，全局快捷键一键呼出/隐藏，托盘菜单"退出"触发关闭前保存提示
- **可配置快捷键**：全部快捷键可在设置面板中自定义键序列，支持清空禁用
- **全局快捷键（KDE）**：KDE 桌面通过 KGlobalAccel 注册系统级快捷键
- **字体大小调节**：设置面板中可分别调整代码编辑区字体大小（8-48 pt）和全局界面字体大小（8-48 pt），左栏分类树和缩略图名称跟随界面字体设置
- **依赖检测**：启动时检测 `xelatex` 和当前配置的 SVG 转换工具（`pdftocairo` 或 `inkscape`），缺失时弹出安装指引
- **120 个高质量预置片段**：涵盖数学（统计/几何/微积分）、物理（电磁学/力学/光学/热力学）、电路、化学等学科领域，均从 tikz.net 和 TeXample.net 精选并经过 XeLaTeX 编译验证
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
mkdir build && cd build
cmake ..
cmake --build .

# 运行
./TikzManager

# 运行测试
ctest --output-on-failure
```

如需禁用 KGlobalAccel（仅用 QHotkey 后备方案）：
```bash
cmake .. -DWITH_KGLOBALACCEL=OFF
```

如需同时禁用全局快捷键：
```bash
cmake .. -DWITH_KGLOBALACCEL=OFF -DWITH_QHOTKEY=OFF
```

---

### 运行时依赖（非构建依赖）

行时需要 LaTeX 发行版：

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
- **标签过滤器**：搜索框下方的流式布局标签徽章，点击标签筛选片段（AND 逻辑），选中标签蓝色高亮。标签过多时自动折叠为最多两行（150ms 防抖，调整窗口宽度时稳定不闪烁），点击"更多标签..."弹出对话框展示全部标签（同样使用流式布局方便浏览）
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

- **语法高亮**（`TikzHighlighter`）：基于 `QSyntaxHighlighter`，**15 条优先正则规则 + 用户定义动态规则**
  - 基础规则（按优先级）：
    - 注释 `%` → 灰色斜体
    - 字符串 `"..."` → 橙黄色
    - 参数 `@@var@@` → 绿色斜体
    - 环境 `\begin{...}` / `\end{...}` → 紫色加粗
    - 命令 `\draw`、`\node` 等 → 蓝色加粗
    - 数学模式 `$...$`、`\(...\)`、`\[...\]` → 绿色
    - 坐标 `(x,y)` → 橙色
    - 选项 `[...]` → 青色
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
    - key=value 中的 key 名 → 深青加粗，与值分色
  - `TikzDocumentState` 通过 `CodeEditor` 注入，300ms 防抖重新解析，用户定义名变化后自动重高亮

- **选中词高亮**：光标置于单词上或双击选中时，文档中所有相同单词以浅橙色背景高亮，辅助查看变量/命令的使用位置

- **自动缩进**：按 Enter 换行时自动继承上一行的前导空白，若行尾为 `{` 或以 `\begin` 开头则额外增加 4 空格缩进

- **撤销/重做**：标准 Ctrl+Z / Ctrl+Shift+Z，工具栏有独立按钮

### 代码补全

智能补全（`TikzCompleter`）基于**结构化词库**（`TikzKeywordDB`，~1400+ 条带环境/命令/库元数据的结构化条目）和**文档状态追踪**（`TikzDocumentState`，范围栈/用户定义名/活动库/`\usepackage` 检测），实现三层精细过滤：

**基础上下文**（10 种）：
| 上下文 | 触发条件 | 补全内容 |
|--------|---------|---------|
| 命令 | 输入 `\` 后接字母 | 按当前环境和激活库动态过滤的 TikZ/LaTeX/PGF/tkz-euclide 命令 |
| 环境 | `\begin{` 未闭合 | ~65 个 LaTeX/TikZ 环境 |
| 选项 | `[...]` 内 | 环境/命令/库三级过滤的选项，含 ~200+ CircuiTikZ 组件（`R`/`C`/`resistor`/`npn` 等）及 90 个双极前缀组合（`pR`/`vR`/`sR` 等） |
| 锚点/键处理 | 字母或 `/` 后跟 `.` | ~55 个锚点 + 16 个 PGF 键处理器（不含小数点数字触发） |
| 值 | `=` 后 | **智能过滤**：按 key 名匹配——颜色键仅提示颜色、宽度键仅提示尺寸、`pattern=` 仅提示图案、`decoration=` 仅提示装饰 |
| 参数 | `@@` 未闭合 | 代码中声明的参数变量 |
| TikZ 库 | `\usetikzlibrary{` 内 | ~88 个 TikZ 库名（从词库动态读取） |
| 通用词 | 任意已知词 | ~1400+ 个所有词表去重并集 |
| 坐标/节点名 | `(` 后 | 用户定义的坐标名、节点名 |
| 用户命令 | `\` 后 | 用户通过 `\newcommand`/`\def` 定义的命令 |

**环境过滤**：在 `\begin{circuitikz}...\end{circuitikz}` 内只显示电路相关选项，在 `\begin{axis}...\end{axis}` 内只显示 pgfplots 选项。

**命令过滤**：`\node[...]` 内额外显示形状/锚点/文本选项，`\draw[...]` 内额外显示线型/箭头/装饰选项。path 命令（draw/fill/to）也能提示 node 可用的选项（如形状）。

**库过滤**：未加载 `decorations.pathmorphing` 库时不提示 `snake`/`coil`/`zigzag`；未加载 `angles` 库时不提示 `angle radius`/`angle eccentricity`；未加载 `shadows.blur` 库时不提示 `blur shadow` 等模糊阴影选项；未加载 `circuitikz` 时不提示 CircuiTikZ 组件选项；未加载 `tkz-euclide` 时不提示 tkz-euclide 命令。`\usepackage{circuitikz}` 和 `\usepackage{tkz-euclide}` 自动激活对应库，`\usepackage{tikz-3dplot}` 自动激活 `3d` 库。

**用户定义补全**：实时解析代码中的自定义定义，自动纳入补全：
- 样式：`\tikzset{name/.style={...}}`、`\tikzstyle{name}` → 选项上下文，含空格名（如 `test lines`）
- 坐标：`\coordinate (name) at ...` → 坐标上下文
- 节点：`\node (name) ...`、`\pic (name) ...` → 坐标上下文
- foreach 变量：`\foreach \x in ...`、`\foreach \xyz / \xtext in ...`（支持空格分隔多变量，如 `/` 两侧有空格）→ 命令上下文
- 命令：`\newcommand{\foo}`、`\def\foo` → 命令上下文
- 颜色：`\definecolor{name}`、`\colorlet{name}` → 值上下文（`color=`、`draw=`、`fill=` 等）
- 定义删除后补全列表即时清空，不留残留项

**tkz-euclide v5 支持**（71 个命令）：定义点/线/圆/多边形、绘制点/线/多边形/圆/弧、角标注、交点计算、三角形特殊线（bisector/median/altitude/Euler）、裁剪、全局样式设置。所有命令标记 `requiredLibs={"tkz-euclide"}`，仅在 `\usepackage{tkz-euclide}` 检测到后出现。

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
| 物理 | 90 | 力学、电磁学、光学、热力学、流体力学、相对论、量子、粒子物理 |
| 电路 | 5 | 交流电路、RC电路、变压器 |
| 化学 | 2 | 有机分子、元素周期表 |

### 数学类（23 个）

**数学/几何**（7 个）：垂线作图、简单曲线、光滑曲线的控制点、自定义光滑曲线端点、球体体积、角度与标注、角平分线

**数学/函数**（3 个）：函数图像、极坐标、函数图像 - 坐标轴环境：反正弦/反余弦/反正切

**数学/微积分**（5 个）：柱坐标体积微分、直角坐标体积微分、球坐标表面积微分、函数平均值 - 线性函数、函数平均值 - 正弦函数

**数学/统计**（4 个）：正态分布、线性回归、高斯分布 - 68-95-99法则、高斯分布 - CLs方法 p值（大重叠）

**数学/分析**（2 个）：复数平面 - 复振子三维、复数平面 - 复数旋转

**数学/拓扑**（1 个）：平面到环面

**数学/艺术**（2 个）：角平分线、彭罗斯三角 - 变体2

### 物理类（90 个）

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

### 化学类（2 个）

元素周期表、有机分子结构（含分子振动）

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
- 界面字体大小（8–48，默认 10）— 影响左栏分类树、缩略图名称及全局界面字体

**快捷键设置**：
- 全部 8 项操作均可自定义键序列
- 清空键序列 = 禁用该快捷键
- 按 Delete 键或点击清除按钮可清空

**行为设置**：
- **保存后自动编译** — 开启后点击保存按钮（或按保存快捷键）时自动触发编译并刷新 PDF 预览，无需手动点击"编译预览"。默认开启。
- **编译线程数** — 设置批量预览时并行编译的线程数（1–32，默认 6）。每个线程独立创建 LaTeX 编译器实例，互不干扰。
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
├── tikz_highlighter.h / .cpp        # TikZ/LaTeX 语法高亮（21 种格式：15 条规则 + 6 类用户定义动态高亮）
├── tikz_completer.h / .cpp          # 智能代码补全（10 种上下文 + 环境/命令/库三级过滤 + 用户定义补全）
├── tikz_keywords.h / .cpp           # 结构化关键词库（TikzKeywordDB 单例，~1400+ 条目含环境/命令/库元数据，覆盖 CircuiTikZ 完整组件(~200)+双极前缀(90)+节点形状(~120)+PGF键路径(30+)，tkz-euclide v5(71 命令)。智能过滤：路径命令也匹配节点可用选项）
├── tikz_document_state.h / .cpp     # 文档状态追踪（范围栈、\usetikzlibrary 解析、\usepackage 自动检测（circuitikz/tkz-euclide/tikz-3dplot/tikz-cd 映射到活动库）、用户样式/坐标/节点/foreach变量(支持空格分隔多变量)/颜色/命令/pic名解析）
├── tikz_words.h                     # TikZ 词库兼容层（委托到 TikzKeywordDB）
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
├── test_multitab.cpp                # 多标签页功能测试（创建/切换/关闭/去重）
├── test_draft_recovery.cpp          # 草稿格式完整性 + 目录扫描测试
├── test_tex_import.cpp              # .tex 文件导入代码提取测试
├── test_params.cpp                  # 参数声明解析与替换测试
├── test_fixes.cpp                   # 关键修复验证（行号正则、注释优先级、QProcess、数据流）
├── test_completer.cpp               # 补全词库完整性 + detectContext 上下文检测（34 个用例）
├── test_document_state.cpp          # 文档状态追踪（12 个用例：范围/库/样式/坐标/pic/foreach/颜色）
└── test_enhanced_highlighter.cpp    # 增强语法高亮（10 个用例：PGF路径/处理器/用户定义名/综合）
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
│   │   ├── TikzHighlighter  语法彩色高亮（15 条规则 + 用户定义）
│   │   ├── TikzCompleter    智能补全（10 种上下文 + 三级过滤 + 智能值提示 + 上下文感知命令 + CircuiTikZ/tkz-euclide 支持）
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

项目包含十三套自动化测试（通过 CTest 运行）：

| 测试 | 内容 |
|------|------|
| `test_snippet_manager` | 片段创建/读取/更新/删除，loadCode，renameCategory，compileCommand JSON 序列化，ZIP 导入/导出往返 |
| `test_latex_compiler` | xelatex 可用性检测，基本编译，PDF 生成，PNG 转换，SVG 转换，错误编译日志验证，自定义命令抽取（41 项测试覆盖 14 类定义命令），compileCommand 自定义引擎编译与 lastFullCommand 验证 |
| `test_search` | 精确匹配、子序列匹配、连续加分、中文搜索、标签过滤、分类统计 |
| `test_packages_libraries` | 宏包字符串解析（含嵌套括号选项），TikZ 库解析，模板注入正确性，往返序列化 |
| `test_highlighter_regex` | 数学模式 `$...$`、`\(...\)`、`\[...\]` 正则表达式匹配验正 |
| `test_multitab` | 多标签页功能：创建/切换/关闭标签页、重复打开去重、关闭前未保存检查 |
| `test_draft_recovery` | 草稿文件格式完整性（全字段 JSON 读写往返、空代码过滤、目录扫描） |
| `test_tex_import` | .tex 文件导入代码提取（三段式解析）及宏包/TikZ 库声明解析 |
| `test_params` | 参数声明正则解析与 `@@var@@` 替换正确性（单参数、多参数、负数、零值） |
| `test_fixes` | 关键修复验证：行号正则锚定、注释优先级、wrapCode 无 document 前置注入、QProcess 启动检测、数据流状态检查、自动编译设置、原子文件重命名、草稿清理、路径遍历字符检测、导入失败目录回滚（10 项测试） |
| `test_completer` | TikZ 补全词库完整性验证（选项/锚点/颜色/线型/交点/角度库关键字含重复检测）、detectContext 上下文检测（34 个测试用例）、PGF 键处理器、值提示（箭头/图案/角度/曲线）、空列表模型清空（15 项测试） |
| `test_document_state` | 文档状态追踪：范围栈检测、库解析、用户样式（含空格名）/坐标/节点/pic名/foreach变量（含空格分隔多变量、>2个变量）/颜色/命令解析、环境名查询、片段库注入（14 个测试用例） |
| `test_enhanced_highlighter` | 增强语法高亮：PGF路径/键处理器/库规则不崩溃，用户样式/节点名/foreach变量高亮检测，key=value分色，多行注释，综合测试，foreach空格分隔变量，**注释保护**验证（13 个测试用例） |

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
