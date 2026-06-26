# HiTikZ

## 终架构与功能规范总结

---

### 📝 TikZ 代码合集管理器 (Linux/C++) 设计规范

#### 一、 技术栈与核心架构
1.  **开发框架**：C++ + Qt6 (原生 C++)。
2.  **数据存储**：基于文件系统的分散存储。
    *   根目录：`~/.local/share/TikzManager/`
    *   系统预设：`~/.local/share/TikzManager/presets/` (首次启动从程序内拷贝，只读)。
    *   用户图库：`~/.local/share/TikzManager/snippets/` (可读写)。
3.  **片段结构**：每个片段一个独立目录，包含：
    *   `meta.json`：名称、简介、主分类、标签、使用的 LaTeX 模板 ID、参数变量声明。
    *   `snippet.tex`：纯 `\begin{tikzpicture}...\end{tikzpicture}` 核心代码。
    *   `preview.png`：最后一次成功编译的持久化预览图。
    *   `assets/` (可选)：该片段依赖的外部图片或数据文件。

#### 二、 UI 交互与布局
1.  **主界面**：经典三栏 + 顶部多标签页 (Tab)。
    *   **左栏 (导航)**：顶部全局搜索框；中部树形分类导航；底部网格缩略图视图（点击分类或搜索时展示）。
    *   **中栏 (编辑)**：基于 `QPlainTextEdit` 的代码编辑器（等宽字体、带行号、Tab缩进4空格）；底部可收起的编译日志面板。
    *   **右栏 (属性与预览)**：顶部 PDF 矢量预览区（支持缩放平移）；下方元数据编辑区（名称、简介、分类、标签）；动态参数输入区；操作按钮栏。
2.  **编辑器**：轻量级实现，无语法高亮和自动补全，仅保证基础代码编辑体验。支持 `Ctrl+Z` 原生回退。
3.  **全局快速呼出**：支持系统全局快捷键一键置顶并聚焦搜索框，按 `Esc` 隐藏，适配课堂全屏演示场景。

#### 三、 编译与预览流水线
1.  **编译引擎**：调用系统 `xelatex`。
    *   命令：`xelatex -interaction=nonstopmode -halt-on-error -no-shell-escape`
    *   支持自定义环境变量（如 `TEXINPUTS` 包含 `assets/` 目录）。
2.  **编译机制**：防抖异步编译。停止输入 1.5 秒后触发，编译时显示 Loading，编译期间再次输入取消上次进程。切换片段或关闭程序时，若编译成功则持久化 `preview.png`。
3.  **预览渲染**：使用 `QtPdf` 或 `Poppler` 直接渲染 PDF 矢量数据，保证高清无损缩放。
4.  **模板系统**：支持多个可配置的 Standalone 模板（配置不同的 `\usepackage`），片段元数据绑定指定模板进行编译。
5.  **错误反馈**：正则解析 `.log` 文件提取错误行号，日志面板高亮显示，双击跳转编辑器对应行。

#### 四、 搜索系统
1.  **机制**：基于 UTF-8 字符的子序列匹配，不引入拼音转换。
2.  **打分规则**：基于紧密度评分（连续匹配高分，间隔扣分），名称匹配权重远大于简介。

#### 五、 图形参数化 (进阶特性)
1.  **占位符**：在 `snippet.tex` 中使用 `% @param: var=默认值` 声明变量，代码中使用 `@@var@@` 占位。
2.  **交互**：右栏动态生成参数输入框，修改后点击“应用参数”在内存中替换并触发编译。复制代码时可选复制模板或当前参数化后的最终代码。

#### 六、 导出与系统集成
1.  **格式转换**：统一由生成的 PDF 通过 `pdftocairo` 转换。支持自定义 DPI 的 PNG 导出/复制，SVG 导出/复制。
2.  **无弹窗快速复制**：
    *   复制代码：`Ctrl+Shift+C`
    *   复制 PNG：`Ctrl+Shift+P` (通过 `QClipboard`)
    *   复制 SVG：`Ctrl+Shift+S` (通过调用 `xclip` 或 `wl-copy`)
3.  **导入导出**：以 `.zip` 格式打包片段文件夹，保持透明结构。
4.  **部署**：暂不考虑打包分发，以 CMake 编译安装为主。不自带 LaTeX 环境，首次启动检测系统依赖。

---

这个设计文档已经具备极高的可执行性，平衡了功能丰富度与 C++ 开发成本。你可以直接以此作为蓝图开启项目初始化了。

---

# TikZ 代码合集管理器 - 详细开发文档

## 🛠️ 准备工作：开发环境搭建

在开始写代码之前，确保你的 Linux 系统已安装必要的工具和库。

**1. 安装基础编译工具和 Qt6 库 **

当前环境是arch 中的 Manjaro，桌面是 kde 6，桌面环境是 wayland，各开发环境已安装好

**2. 安装 LaTeX 环境及转换工具:**

TexLive 2025 已安装好，转换工具已安装好

**3. 创建项目目录结构:**

```text
TikzManager/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   ├── snippet_manager.h     # 数据与文件管理
│   ├── snippet_manager.cpp
│   ├── latex_compiler.h      # 编译与转换
│   ├── latex_compiler.cpp
│   └── code_editor.h         # 带行号的编辑器
└── resources/
    └── templates/            # 存放 LaTeX standalone 模板
```

---

## 🏗️ 第一阶段：项目骨架与基础 UI

**目标**：搭建 CMake 构建系统，创建主窗口的三栏布局。

### 1. 编写 `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.16)
project(TikzManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 寻找 Qt6 组件
find_package(Qt6 REQUIRED COMPONENTS Widgets Pdf Network)

add_executable(TikzManager
    src/main.cpp
    src/mainwindow.cpp
    src/snippet_manager.cpp
    src/latex_compiler.cpp
    src/code_editor.h
)

target_link_libraries(TikzManager PRIVATE Qt6::Widgets Qt6::Pdf)
```

### 2. 编写 `src/main.cpp`
```cpp
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(1200, 800);
    window.show();
    return app.exec();
}
```

### 3. 设计主窗口 `src/mainwindow.h` 与 `mainwindow.cpp`
使用 `QSplitter` 实现可拖动的三栏布局。

**mainwindow.h:**
```cpp
#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QLineEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    void setupUI();
    
    // 左栏
    QWidget *leftPanel;
    QLineEdit *searchBox;
    QTreeView *categoryTree;
    QListView *thumbnailList;
    
    // 中栏
    QPlainTextEdit *codeEditor; // 后期替换为自定义的 CodeEditor
    QPlainTextEdit *logPanel;
    
    // 右栏
    QWidget *rightPanel;
    QLabel *previewLabel; // 初期用 QLabel 显示图片
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
};
```

**mainwindow.cpp (关键部分):**
```cpp
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

void MainWindow::setupUI() {
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // --- 左栏 ---
    leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    searchBox = new QLineEdit;
    searchBox->setPlaceholderText("搜索名称或简介...");
    categoryTree = new QTreeView;
    thumbnailList = new QListView;
    leftLayout->addWidget(searchBox);
    leftLayout->addWidget(categoryTree, 1); // 树占 1 份
    leftLayout->addWidget(thumbnailList, 2); // 缩略图占 2 份
    
    // --- 中栏 ---
    QSplitter *centerSplitter = new QSplitter(Qt::Vertical);
    codeEditor = new QPlainTextEdit;
    codeEditor->setFont(QFont("Mono", 10)); // 等宽字体
    logPanel = new QPlainTextEdit;
    logPanel->setReadOnly(true);
    centerSplitter->addWidget(codeEditor);
    centerSplitter->addWidget(logPanel);
    centerSplitter->setSizes({600, 100});
    
    // --- 右栏 ---
    rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    previewLabel = new QLabel("预览区");
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(300, 300);
    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("名称");
    descEdit = new QTextEdit;
    descEdit->setPlaceholderText("简介");
    rightLayout->addWidget(previewLabel, 3);
    rightLayout->addWidget(nameEdit);
    rightLayout->addWidget(descEdit, 1);
    
    // 组装
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({250, 600, 350});
    
    setCentralWidget(mainSplitter);
}
```

---

## 🗄️ 第二阶段：数据模型与文件系统管理

**目标**：实现代码片段的增删改查、JSON 读写。

### 1. 设计数据结构 (`src/snippet_manager.h`)
```cpp
#pragma once
#include <QString>
#include <QStringList>
#include <QJsonDocument>

struct Snippet {
    QString id;        // 使用 UUID 作为目录名
    QString name;
    QString description;
    QString category;  // 如 "数学/几何"
    QString templateId;// 使用的模板 ID
    QString code;      // TikZ 核心代码
    // 注意：文件在磁盘上以 id 命名的文件夹存储
};

class SnippetManager : public QObject {
    Q_OBJECT
public:
    SnippetManager(QObject *parent = nullptr);
    
    QString getBasePath() const; // ~/.local/share/TikzManager/snippets/
    Snippet loadSnippet(const QString &id);
    bool saveSnippet(const Snippet &s);
    QList<Snippet> getAllSnippets();
    
private:
    QString basePath;
    QString getSnippetPath(const QString &id);
    QJsonObject snippetToJson(const Snippet &s);
    Snippet jsonToSnippet(const QJsonObject &obj);
};
```

### 2. 实现文件读写 (`src/snippet_manager.cpp`)
```cpp
#include "snippet_manager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>

SnippetManager::SnippetManager(QObject *parent) : QObject(parent) {
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    basePath = dataLocation + "/snippets/";
    QDir().mkpath(basePath); // 确保目录存在
}

bool SnippetManager::saveSnippet(const Snippet &s) {
    QString path = getSnippetPath(s.id);
    QDir().mkpath(path);
    
    // 保存 meta.json
    QJsonDocument doc(snippetToJson(s));
    QFile metaFile(path + "meta.json");
    if (metaFile.open(QIODevice::WriteOnly)) {
        metaFile.write(doc.toJson());
        metaFile.close();
    }
    
    // 保存 snippet.tex
    QFile texFile(path + "snippet.tex");
    if (texFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        texFile.write(s.code.toUtf8());
        texFile.close();
        return true;
    }
    return false;
}

// ... 实现 loadSnippet, getAllSnippets 逻辑：遍历 basePath 下的目录，读取 json 和 tex
```

---

## ⚙️ 第三阶段：LaTeX 编译流水线

**目标**：调用 XeLaTeX 编译，生成 PDF，转为 PNG 预览。

### 1. 设计编译器 (`src/latex_compiler.h`)
```cpp
#pragma once
#include <QProcess>
#include <QObject>

class LatexCompiler : public QObject {
    Q_OBJECT
public:
    explicit LatexCompiler(QObject *parent = nullptr);
    void compile(const QString &texCode, const QString &templateId, const QString &snippetId);
    
signals:
    void compilationFinished(bool success, const QString &pdfPath, const QString &logOutput);
    
private:
    QProcess *process;
    QString tempDir;
    QString wrapCode(const QString &texCode, const QString &templateId);
    void convertToPng(const QString &pdfPath); // 调用 pdftocairo
};
```

### 2. 实现编译逻辑 (`src/latex_compiler.cpp`)
```cpp
#include "latex_compiler.h"
#include <QDir>
#include <QTemporaryFile>
#include <QStandardPaths>

LatexCompiler::LatexCompiler(QObject *parent) : QObject(parent) {
    process = new QProcess(this);
    tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/TikzManager/";
    QDir().mkpath(tempDir);
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus) {
            QString log = process->readAllStandardError() + process->readAllStandardOutput();
            QString pdfPath = tempDir + "output.pdf";
            bool success = (exitCode == 0 && QFile::exists(pdfPath));
            emit compilationFinished(success, pdfPath, log);
        });
}

void LatexCompiler::compile(const QString &texCode, const QString &templateId, const QString &snippetId) {
    // 1. 组装完整代码
    QString fullCode = wrapCode(texCode, templateId);
    
    // 2. 写入临时 tex 文件
    QFile texFile(tempDir + "output.tex");
    if (texFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        texFile.write(fullCode.toUtf8());
        texFile.close();
    }
    
    // 3. 调用 xelatex (使用 -output-directory 避免污染程序目录)
    QString cmd = "xelatex";
    QStringList args;
    args << "-interaction=nonstopmode" << "-halt-on-error" 
         << "-output-directory" << tempDir << tempDir + "output.tex";
         
    process->start(cmd, args);
}
```

---

## 🔍 第四阶段：搜索与列表联动

**目标**：实现 UTF-8 子序列模糊搜索，将结果展示在左下角列表。

### 1. 实现模糊搜索算法 (放在 `SnippetManager` 或单独的工具类中)
```cpp
// 简单的子序列匹配打分算法
int fuzzyMatchScore(const QString &query, const QString &target) {
    if (query.isEmpty()) return 100; // 空搜索匹配所有
    
    int score = 0;
    int qi = 0; // query 的索引
    int consecutive = 0; // 连续匹配加分
    
    for (int ti = 0; ti < target.length() && qi < query.length(); ++ti) {
        if (query[qi].toLower() == target[ti].toLower()) {
            score += 10 + consecutive * 5; // 连续匹配得分高
            consecutive++;
            qi++;
        } else {
            consecutive = 0;
        }
    }
    
    // 如果 query 没有完全匹配完，返回 0 分
    return (qi == query.length()) ? score : 0;
}
```

### 2. 联动 UI
在 `MainWindow` 中连接 `searchBox` 的 `textChanged` 信号，遍历 `getAllSnippets()`，对其 `name` 和 `description` 计算得分，过滤出得分 > 0 的项，更新 `thumbnailList` 的数据模型。

---

## ✍️ 第五阶段：带行号的代码编辑器

**目标**：替换原来的 `QPlainTextEdit`，实现带行号的编辑器，方便查错。

### 1. 创建 `src/code_editor.h`
（这部分 Qt 官方有经典示例，直接套用即可）
```cpp
#pragma once
#include <QPlainTextEdit>
#include <QWidget>

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr);
    void lineNumberPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
private:
    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberPaintEvent(event); }
private:
    CodeEditor *codeEditor;
};
```
*具体实现代码参考 Qt 官方文档的 [Code Editor Example](https://doc.qt.io/qt-6/qtwidgets-widgets-codeeditor-example.html)，将其集成到你的项目中，并在 `MainWindow` 中用 `CodeEditor` 替换 `QPlainTextEdit`。*

---

## 🖼️ 第六阶段：PDF 矢量预览与错误跳转

**目标**：显示高清矢量图，解析日志双击跳转。

### 1. 预览 PDF
使用 Qt6 的 `QPdfDocument` 和 `QPdfView`。
```cpp
// MainWindow 构造函数中
QPdfDocument *doc = new QPdfDocument(this);
QPdfView *pdfView = new QPdfView(this);
pdfView->setDocument(doc);

// 替换右栏的 previewLabel
rightLayout->replaceWidget(previewLabel, pdfView);

// 在编译完成信号槽中
connect(compiler, &LatexCompiler::compilationFinished, this, [this, doc](bool success, const QString &pdfPath, const QString &log){
    if (success) {
        doc->load(pdfPath);
    }
    logPanel->setPlainText(log);
});
```

### 2. 日志跳转
捕获 `logPanel` 的 `doubleClicked` 信号，使用正则表达式 `QRegularExpression("l\.(\\d+)")` 提取行号，然后：
```cpp
int line = captured.toInt();
QTextCursor cursor = codeEditor->textCursor();
cursor.movePosition(QTextCursor::Start);
cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
codeEditor->setTextCursor(cursor);
codeEditor->highlightCurrentLine(); // 高亮当前行
```

---

## 📋 第七阶段：剪贴板与格式转换

**目标**：实现一键复制 PNG/SVG/Code。

在 `LatexCompiler` 中添加转换函数：
```cpp
void LatexCompiler::convertTo(const QString &format, int dpi) {
    QStringList args;
    if (format == "png") {
        args << "-png" << "-r" << QString::number(dpi) << tempDir + "output.pdf" << tempDir + "output";
    } else if (format == "svg") {
        args << "-svg" << tempDir + "output.pdf" << tempDir + "output.svg";
    }
    QProcess::execute("pdftocairo", args);
}
```

在 `MainWindow` 中设置快捷键和剪贴板逻辑：
```cpp
// 复制代码
auto copyCodeAction = new QAction("复制代码", this);
copyCodeAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
connect(copyCodeAction, &QAction::triggered, [this](){
    QApplication::clipboard()->setText(codeEditor->toPlainText());
    statusBar()->showMessage("代码已复制", 2000);
});

// 复制 PNG
auto copyPngAction = new QAction("复制PNG", this);
copyPngAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
connect(copyPngAction, &QAction::triggered, [this](){
    compiler->convertTo("png", 300); // 300 DPI
    QImage img(tempDir + "output-1.png"); // pdftocairo 默认加页码
    QApplication::clipboard()->setImage(img);
    statusBar()->showMessage("PNG已复制", 2000);
});

// 添加到右键菜单或工具栏
```

---

## ⚙️ 第八阶段：设置面板与模板系统

**目标**：让用户配置环境变量和查看/修改模板。

1. 在 `resources/templates/` 下创建 `default_math.tex` 和 `default_physics.tex`。
2. 写一个 `SettingsDialog`，使用 `QFormLayout` 让用户配置：
   - `xelatex` 路径 (默认从 $PATH 找)
   - `pdftocairo` 路径
   - 默认 PNG DPI
   - 自定义 `TEXINPUTS` 环境变量 (追加到 LatexCompiler 的 `QProcess::setProcessEnvironment` 中)
3. 实现模板管理：软件启动时将 `resources/templates/` 拷贝到用户配置目录，允许用户在设置面板编辑这些 `.tex` 模板。`LatexCompiler::wrapCode` 时读取对应模板，将 `%%% TIKZ_CODE_HERE %%%` 替换为片段代码。

---

## 🧩 第九阶段：参数化功能

**目标**：解析 `% @param`，动态生成 UI。

1. 在 `CodeEditor` 的 `textChanged` 信号中，扫描代码首行或前几行：
   ```cpp
   QRegularExpression re("% @param:\\s*(\\w+)=(\\w+)");
   // 提取出变量名和默认值
   ```
2. 清空右栏的参数布局 `paramsLayout`。
3. 为每个变量生成 `QLabel` 和 `QLineEdit`（或 `QSpinBox`），填入默认值。
4. 当用户修改输入框或点击“应用参数”时：
   ```cpp
   QString finalCode = originalCode;
   for (auto &param : params) {
       finalCode.replace("@@" + param.name + "@@", param.lineEdit->text());
   }
   // 触发 compiler->compile(finalCode, ...)
   ```

---

## 🚀 第十阶段：系统托盘与全局快捷键

**目标**：后台常驻，一键呼出。

### 1. 系统托盘
```cpp
// MainWindow 构造函数
QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
trayIcon->setIcon(QIcon(":/icons/icon.png"));
trayIcon->show();

// 重写 closeEvent
void MainWindow::closeEvent(QCloseEvent *event) {
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
```

### 2. 全局快捷键
Qt 原生不支持全局快捷键，需要借助第三方小库 `QHotkey` (可通过 CMake FetchContent 引入)。
```cpp
#include <QHotkey>
// ...
QHotkey *hotkey = new QHotkey(QKeySequence("Ctrl+Alt+T"), true, this);
connect(hotkey, &QHotkey::activated, [this](){
    if (isVisible() && !isMinimized()) {
        hide();
    } else {
        show();
        activateWindow();
        raise();
        searchBox->setFocus();
    }
});
```

---

## 🎉 开发完成！

按照这十个阶段，逐步实现功能并进行局部测试，最终你将得到一个高度定制化、高效且强大的 Linux 环境下的 TikZ 代码管理与编辑工具。建议先打通第一阶段到第三阶段的主线流程（能看到自己写的代码编译成图片显示出来），再去完善搜索、参数化和全局快捷键等高级特性。祝你编码愉快！
