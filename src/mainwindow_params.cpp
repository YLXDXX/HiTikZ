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

void MainWindow::clearParams()
{
    QLayoutItem *child;
    while ((child = paramsLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    currentParams.clear();
}

void MainWindow::parseParams()
{
    if (m_parseParamsTimer)
        m_parseParamsTimer->start();
}

void MainWindow::performParseParams()
{
    int tabIdx = tabWidget ? tabWidget->currentIndex() : -1;
    QString tabSid = (tabIdx >= 0) ? tabWidget->tabBar()->tabData(tabIdx).toString() : QString();

    if (!currentParams.isEmpty()) {
        QMap<QString, QString> vals;
        for (const ParamInfo &param : currentParams) {
            vals[param.name] = param.edit->text();
        }
        QString key = !tabSid.isEmpty() ? tabSid : (currentSnippetId.isEmpty() ? QString() : currentSnippetId);
        if (!key.isEmpty())
            m_perSnippetParamValues[key] = vals;
    }

    clearParams();

    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QString code = ed->toPlainText();
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

    QStringList paramNames;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ParamInfo param;
        param.name = match.captured(1);
        param.defaultValue = match.captured(2);

        QWidget *row = new QWidget;
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *label = new QLabel(param.name + ":");
        QLineEdit *edit = new QLineEdit(param.defaultValue);
        edit->setMaximumWidth(100);
        rowLayout->addWidget(label);
        rowLayout->addWidget(edit, 1);

        if (!tabSid.isEmpty() && m_perSnippetParamValues.contains(tabSid)
            && m_perSnippetParamValues[tabSid].contains(param.name)) {
            edit->setText(m_perSnippetParamValues[tabSid][param.name]);
        }

        param.edit = edit;
        currentParams.append(param);
        paramsLayout->addWidget(row);
        paramNames.append(param.name);

        connect(edit, &QLineEdit::textChanged, this, [this, tabSid]() {
            if (tabSid.isEmpty()) return;
            QMap<QString, QString> vals;
            for (const ParamInfo &p : currentParams)
                vals[p.name] = p.edit->text();
            m_perSnippetParamValues[tabSid] = vals;
        });
    }

    paramsLayout->addStretch();
    ed->refreshParamWords(paramNames);
}

QString MainWindow::applyParams(const QString &code)
{
    QString result = code;
    for (const ParamInfo &param : currentParams) {
        result.replace("@@" + param.name + "@@", param.edit->text());
    }
    return result;
}

QString MainWindow::resolveParamsFromCode(const QString &code)
{
    QString result = code;
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(1);
        QString defaultValue = match.captured(2);
        result.replace("@@" + varName + "@@", defaultValue);
    }
    return result;
}
