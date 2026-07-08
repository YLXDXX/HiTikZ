#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "tikz_document_state.h"
#include "settings_dialog.h"
#ifdef HAS_KGLOBALACCEL
#include "kde_global_shortcut.h"
#endif
#include "pdf_preview_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QSplitter>
#include <QApplication>
#include <QStatusBar>
#include <QHeaderView>
#include <QIcon>
#include <QFileDialog>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFileInfo>
#include <QClipboard>
#include <QImage>
#include <QShortcut>
#include <QMenu>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QEventLoop>
#include <QTimer>
#include <QMimeData>
#include <QDataStream>
#include <QDropEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QTabBar>
#include <QCheckBox>
#include <memory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QtConcurrent>
#include <QThreadPool>
#include "mainwindow_internal.h"

void MainWindow::jumpToErrorLine(const QString &logText)
{
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatchIterator it = re.globalMatch(logText);
    if (!it.hasNext()) return;

    QRegularExpressionMatch match = it.next();
    int line = match.captured(1).toInt();
    if (line < 1) return;

    int editorLine = line - m_userCodeStartLine + 1;
    if (editorLine < 1) editorLine = 1;

    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, editorLine - 1);
    ed->setTextCursor(cursor);
    ed->highlightCurrentLine();
    ed->setFocus();
}

void MainWindow::clearPdfPreview()
{
    pdfPreview->clearDocument();
}

void MainWindow::savePreviewData(const QString &pdfPath, const QString &snippetId)
{
    if (snippetId.isEmpty()) return;

    QString basePath = snippetDataPath(snippetId);
    QString previewPdf = basePath + "/preview.pdf";

    if (QFile::exists(previewPdf))
        QFile::remove(previewPdf);
    if (!QFile::copy(pdfPath, previewPdf)) {
        qWarning() << "Failed to save preview PDF:" << previewPdf;
        return;
    }

    QProcess *pngProc = new QProcess(this);
    connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        pngProc, &QProcess::deleteLater);
    QTimer *timeout = new QTimer(pngProc);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, pngProc, [pngProc]() {
        if (pngProc->state() != QProcess::NotRunning) {
            pngProc->kill();
        }
    });
    connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        timeout, &QTimer::stop);
    timeout->start(15000);
    QStringList args;
    args << "-png" << "-r" << QString::number(kPreviewDpi) << "-singlefile" << pdfPath << (basePath + "/preview");
    pngProc->start(compiler->pdfToCairoCommand(), args);
}

void MainWindow::loadPreviewForSnippet(const QString &id)
{
    if (id.isEmpty()) {
        clearPdfPreview();
        return;
    }

    QString previewPdf = snippetDataPath(id) + "/preview.pdf";
    if (QFile::exists(previewPdf)) {
        pdfPreview->reloadDocument(QFileInfo(previewPdf).absoluteFilePath());
        QTimer::singleShot(kZoomApplyDelayMs, pdfPreview, &PdfPreviewWidget::applyZoomPreference);
    } else {
        clearPdfPreview();
    }
}

void MainWindow::generateAllPreviews()
{
    if (m_batchGenerating) {
        statusBar()->showMessage(QStringLiteral("批量生成正在进行中，请稍候..."), kStatusBarShortMs);
        return;
    }

    QList<Snippet> all = snippetMgr->getAllSnippets(true);
    all.append(snippetMgr->getAllPresets(true));

    if (all.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("没有可生成预览的条目"), kStatusBarShortMs);
        emit batchPreviewFinished();
        return;
    }

    m_previewTotal = all.size();
    m_batchCompleted.storeRelaxed(0);
    m_batchSubmitted.storeRelaxed(0);
    m_batchFailures.clear();
    m_batchGenerating = true;
    m_batchCancelFlag.storeRelaxed(0);
    m_compiling = true;
    compileAct->setEnabled(false);
    applyParamsAct->setEnabled(false);
    forceStopAct->setEnabled(true);
    statusBar()->showMessage(QStringLiteral("正在生成所有预览..."), 0);

    int threadCount = QSettings("HiTikZ", "TikzManager").value("behavior/threadCount", 6).toInt();
    if (threadCount < 1) threadCount = 1;
    if (threadCount > 32) threadCount = 32;
    QThreadPool::globalInstance()->setMaxThreadCount(threadCount);

    for (const Snippet &s : all) {
        m_batchSubmitted.fetchAndAddRelaxed(1);
        QString basePath = snippetDataPath(s.id);
        QThreadPool::globalInstance()->start([this, s, basePath] {
            if (m_batchCancelFlag.loadRelaxed()) {
                QMetaObject::invokeMethod(this, "onBatchTaskFinished", Qt::QueuedConnection,
                    Q_ARG(Snippet, s), Q_ARG(bool, false), Q_ARG(QString, QString()),
                    Q_ARG(QString, QStringLiteral("用户取消编译")));
                return;
            }

            QString code = resolveParamsFromCode(s.code);

            LatexCompiler localCompiler;
            SettingsDialog::applyToCompiler(&localCompiler);

            QString pdfPath, log;
            bool ok = localCompiler.compileBlocking(code, s.templateId, s.id,
                s.packages, s.tikzLibraries, kBatchCompileTimeoutMs, pdfPath, log, s.compileCommand);

            if (ok && !pdfPath.isEmpty()) {
                QString previewPdf = basePath + "/preview.pdf";
                QString tempPdf = basePath + "/preview.tmp.pdf";

                if (QFile::exists(tempPdf))
                    QFile::remove(tempPdf);
                QFile::copy(pdfPath, tempPdf);

                if (QFile::exists(previewPdf))
                    QFile::remove(previewPdf);
                QFile::rename(tempPdf, previewPdf);

                QProcess pngProc;
                QStringList args;
                args << "-png" << "-r" << QString::number(kPreviewDpi) << "-singlefile"
                     << pdfPath << (basePath + "/preview");
                pngProc.start(localCompiler.pdfToCairoCommand(), args);
                pngProc.waitForFinished(10000);
            }

            QMetaObject::invokeMethod(this, "onBatchTaskFinished", Qt::QueuedConnection,
                Q_ARG(Snippet, s), Q_ARG(bool, ok), Q_ARG(QString, pdfPath), Q_ARG(QString, log));
        });
    }
}

void MainWindow::onBatchTaskFinished(const Snippet &snippet, bool success, const QString &pdfPath, const QString &log)
{
    Q_UNUSED(pdfPath)

    if (!m_batchGenerating) return;

    int done = m_batchCompleted.fetchAndAddRelaxed(1) + 1;

    if (!success) {
        QMutexLocker locker(&m_batchMutex);
        m_batchFailures.append({snippet, log});
    }

    statusBar()->showMessage(
        QStringLiteral("生成预览: %1/%2").arg(done).arg(m_previewTotal), 0);

    if (done >= m_previewTotal) {
        m_batchGenerating = false;
        m_compiling = false;
        compileAct->setEnabled(true);
        applyParamsAct->setEnabled(true);
        forceStopAct->setEnabled(false);

        if (m_batchCancelFlag.loadRelaxed()) {
            statusBar()->showMessage(
                QStringLiteral("批量生成已取消: 完成 %1/%2").arg(done).arg(m_previewTotal),
                kStatusBarLongMs);
        } else {
            statusBar()->showMessage(
                QStringLiteral("预览生成完毕: %1 个条目").arg(m_previewTotal), kStatusBarLongMs);
        }
        refreshSearch();
        showBatchPreviewSummary();
        emit batchPreviewFinished();
    }
}

void MainWindow::showBatchPreviewSummary()
{
    int successCount = m_previewTotal - m_batchFailures.size();

    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(QStringLiteral("批量预览生成报告"));
    dlg->resize(700, 500);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(dlg);

    auto *logWidget = new QPlainTextEdit(dlg);
    logWidget->setReadOnly(true);
    logWidget->setFont(logPanel->font());

    QTextCursor cursor = logWidget->textCursor();
    cursor.movePosition(QTextCursor::Start);

    QTextCharFormat summaryFmt;
    summaryFmt.setForeground(QColor(60, 120, 200));
    summaryFmt.setFontWeight(QFont::Bold);
    cursor.insertText(QStringLiteral("批量生成预览完成：总计 %1 个，成功 %2 个，失败 %3 个\n\n")
                           .arg(m_previewTotal)
                           .arg(successCount)
                           .arg(m_batchFailures.size()),
                       summaryFmt);

    if (!m_batchFailures.isEmpty()) {
        QTextCharFormat headerFmt;
        headerFmt.setForeground(QColor(220, 30, 30));
        headerFmt.setFontWeight(QFont::Bold);
        cursor.insertText(QStringLiteral("--- 失败详情 ---\n\n"), headerFmt);

        for (int i = 0; i < m_batchFailures.size(); ++i) {
            const auto &pair = m_batchFailures[i];
            const Snippet &s = pair.first;
            const QString &log = pair.second;

            QTextCharFormat idFmt;
            idFmt.setForeground(QColor(200, 140, 0));
            idFmt.setFontWeight(QFont::Bold);
            cursor.insertText(QStringLiteral("[%1] %2\n").arg(i + 1).arg(s.name), idFmt);
            cursor.insertText(QStringLiteral("    ID: %1\n").arg(s.id));

            if (log.isEmpty()) {
                QTextCharFormat errFmt;
                errFmt.setForeground(QColor(220, 30, 30));
                cursor.insertText(QStringLiteral("    编译失败，无详细错误信息（可能超时）\n\n"), errFmt);
            } else {
                static const QRegularExpression errorRe("^!\\s");
                static const QRegularExpression warningRe("Warning",
                    QRegularExpression::CaseInsensitiveOption);
                static const QRegularExpression lineRe("^l\\.(\\d+)");
                static const QRegularExpression noiseBannerRe(
                    "^(This is XeTeX|entering extended mode|Transcript written on|No file .*\\.aux)"
                );
                static const QRegularExpression noiseFileRe("^[\\(/].*\\.(sty|cls|def|cfg|fd|aux|tex|map|enc|tfm)");

                const QStringList lines = log.split('\n');
                bool inErrorBlock = false;
                for (const QString &line : lines) {
                    bool isError = errorRe.match(line).hasMatch();
                    bool isLineNum = lineRe.match(line.trimmed()).hasMatch();
                    bool isWarn = warningRe.match(line).hasMatch();
                    bool isNoise = noiseBannerRe.match(line).hasMatch()
                        || noiseFileRe.match(line.trimmed()).hasMatch();

                    if (isError) {
                        inErrorBlock = true;
                        QTextCharFormat errFmt;
                        errFmt.setForeground(QColor(220, 30, 30));
                        errFmt.setFontWeight(QFont::Bold);
                        cursor.insertText(QStringLiteral("    %1\n").arg(line), errFmt);
                    } else if (isLineNum) {
                        QTextCharFormat lnFmt;
                        lnFmt.setForeground(QColor(180, 60, 60));
                        cursor.insertText(QStringLiteral("    %1\n").arg(line), lnFmt);
                    } else if (isWarn) {
                        QTextCharFormat warnFmt;
                        warnFmt.setForeground(QColor(200, 140, 0));
                        cursor.insertText(QStringLiteral("    %1\n").arg(line), warnFmt);
                    } else if (inErrorBlock && !line.trimmed().isEmpty() && !isNoise) {
                        cursor.insertText(QStringLiteral("    %1\n").arg(line));
                    } else if (line.trimmed().isEmpty() || isNoise || line.trimmed() == "?") {
                        inErrorBlock = false;
                    }
                }
                cursor.insertText("\n");
            }
        }
    }

    cursor.movePosition(QTextCursor::Start);
    logWidget->setTextCursor(cursor);

    layout->addWidget(logWidget);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::close);
    connect(btnBox, &QDialogButtonBox::accepted, dlg, &QDialog::close);
    layout->addWidget(btnBox);

    dlg->show();
}

void MainWindow::setFormattedLog(bool success, const QString &command, const QString &log, int userCodeStartLine)
{
    logPanel->clear();

    QTextCharFormat cmdFormat;
    cmdFormat.setForeground(QColor(60, 120, 200));
    cmdFormat.setFontWeight(QFont::Bold);

    QTextCharFormat successFormat;
    successFormat.setForeground(QColor(20, 150, 20));
    successFormat.setFontWeight(QFont::Bold);

    QTextCharFormat failureFormat;
    failureFormat.setForeground(QColor(220, 30, 30));
    failureFormat.setFontWeight(QFont::Bold);

    QTextCharFormat errorFormat;
    errorFormat.setForeground(QColor(220, 30, 30));
    errorFormat.setFontWeight(QFont::Bold);

    QTextCharFormat warningFormat;
    warningFormat.setForeground(QColor(200, 140, 0));

    QTextCharFormat lineNumFormat;
    lineNumFormat.setForeground(QColor(180, 60, 60));

    QTextCharFormat defaultFormat;

    logPanel->textCursor().insertText(QStringLiteral("Compile command:\n"), cmdFormat);
    logPanel->textCursor().insertText(command + "\n\n", defaultFormat);

    if (log.isEmpty()) {
        logPanel->textCursor().insertText(
            success ? QStringLiteral("Compilation successful ✓\n") : QStringLiteral("Compilation failed ✗\n"),
            success ? successFormat : failureFormat);
        return;
    }

    const QStringList lines = log.split('\n');
    static const QRegularExpression errorRe("^!\\s");
    static const QRegularExpression warningRe("Warning", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression lineRe("^l\\.(\\d+)");
    static const QRegularExpression outputRe("^Output written on");
    static const QRegularExpression overfullRe("Overfull|Underfull");
    static const QRegularExpression noiseFileRe("^[\\(/].*\\.(sty|cls|def|cfg|fd|aux|tex|map|enc|tfm)");
    static const QRegularExpression noiseBannerRe(
        "^(This is XeTeX|entering extended mode|Transcript written on|No file .*\\.aux)"
    );

    QStringList filtered;
    bool inErrorBlock = false;

    auto adjustLineNum = [&](const QString &line) -> QString {
        QRegularExpressionMatch m = lineRe.match(line.trimmed());
        if (m.hasMatch()) {
            int fullLine = m.captured(1).toInt();
            int editorLine = fullLine - userCodeStartLine + 1;
            if (editorLine < 1) editorLine = 1;
            QString result = line;
            static const QRegularExpression anchoredLineRe(QStringLiteral("^l\\.\\d+"));
            result.replace(anchoredLineRe, QStringLiteral("l.") + QString::number(editorLine));
            return result;
        }
        return line;
    };

    for (const QString &line : lines) {
        bool isError = errorRe.match(line).hasMatch();
        bool isLineNum = lineRe.match(line.trimmed()).hasMatch();
        bool isWarning = warningRe.match(line).hasMatch() || overfullRe.match(line).hasMatch();
        bool isOutput = outputRe.match(line).hasMatch();
        bool isNoise = noiseBannerRe.match(line).hasMatch()
            || noiseFileRe.match(line.trimmed()).hasMatch();

        if (isError) {
            inErrorBlock = true;
            filtered.append(adjustLineNum(line));
        } else if (isLineNum || isWarning || isOutput) {
            filtered.append(adjustLineNum(line));
            if (isOutput) inErrorBlock = false;
        } else if (success) {
            continue;
        } else if (inErrorBlock && !line.trimmed().isEmpty() && !isNoise) {
            filtered.append(adjustLineNum(line));
        } else if (line.trimmed().isEmpty() || isNoise || line.trimmed() == "?") {
            inErrorBlock = false;
        }
    }

    for (const QString &line : filtered) {
        QTextCharFormat fmt = defaultFormat;
        if (errorRe.match(line).hasMatch()) {
            fmt = errorFormat;
        } else if (lineRe.match(line.trimmed()).hasMatch()) {
            fmt = lineNumFormat;
        } else if (warningRe.match(line).hasMatch() || overfullRe.match(line).hasMatch()) {
            fmt = warningFormat;
        }
        logPanel->textCursor().insertText(line + '\n', fmt);
    }

    logPanel->textCursor().insertText(
        success ? QStringLiteral("\nCompilation successful ✓\n") : QStringLiteral("\nCompilation failed ✗ — see errors above\n"),
        success ? successFormat : failureFormat);

    QTextCursor cursor = logPanel->textCursor();
    cursor.movePosition(QTextCursor::Start);
    logPanel->setTextCursor(cursor);
}

void MainWindow::handleLogDoubleClick()
{
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QTextCursor cursor = logPanel->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString line = cursor.selectedText().trimmed();

    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch()) return;

    int lineNum = match.captured(1).toInt();
    if (lineNum < 1) return;

    cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNum - 1);
    ed->setTextCursor(cursor);
    ed->highlightCurrentLine();
    ed->setFocus();
}

void MainWindow::checkSystemDependencies()
{
    QStringList missing;

    if (!LatexCompiler::checkXelatexAvailable())
        missing << QStringLiteral("xelatex");

    QSettings settings("HiTikZ", "TikzManager");
    QString svgTool = settings.value("svg/tool", "pdftocairo").toString();
    if (svgTool == "inkscape") {
        if (!LatexCompiler::checkInkscapeAvailable())
            missing << QStringLiteral("inkscape");
    } else {
        if (!LatexCompiler::checkPdfToCairoAvailable())
            missing << QStringLiteral("pdftocairo");
    }

    if (missing.isEmpty()) return;

    QString msg = QStringLiteral("以下依赖工具未找到：\n\n");
    for (const QString &m : missing)
        msg += QStringLiteral("  • %1\n").arg(m);
    msg += QStringLiteral("\n这些工具是编译 TikZ 预览和格式转换所必需的。\n\n");
    msg += QStringLiteral("安装方法 (Arch/Manjaro):\n");
    msg += QStringLiteral("  sudo pacman -S texlive-core poppler\n\n");
    msg += QStringLiteral("安装方法 (Debian/Ubuntu):\n");
    msg += QStringLiteral("  sudo apt install texlive-xetex poppler-utils\n\n");
    msg += QStringLiteral("安装方法 (Fedora):\n");
    msg += QStringLiteral("  sudo dnf install texlive-xetex poppler-utils");

    QTimer::singleShot(200, this, [this, msg]() {
        QMessageBox::warning(this, QStringLiteral("缺少依赖"), msg);
    });
}

void MainWindow::updateFitActionStates()
{
    int pref = pdfPreview->zoomPreference();
    fitPageAct->setChecked(pref == 0);
    fitWidthAct->setChecked(pref == 1);
    fitHeightAct->setChecked(pref == 2);
}
