#pragma once
#include <QProcess>
#include <QProcessEnvironment>
#include <QObject>
#include <QString>
#include <QStringList>

class LatexCompiler : public QObject {
    Q_OBJECT
public:
    explicit LatexCompiler(QObject *parent = nullptr);
    ~LatexCompiler();

    void compile(const QString &texCode, const QString &templateId, const QString &snippetId,
                 const QString &packages = QString(), const QString &tikzLibraries = QString(),
                 const QString &compileCommand = QString());
    bool compileBlocking(const QString &texCode, const QString &templateId, const QString &snippetId,
                         const QString &packages, const QString &tikzLibraries,
                         int timeoutMs, QString &outPdfPath, QString &outLog,
                         const QString &compileCommand = QString());
    void cancelCompile();
    void setTemplateDir(const QString &dir);
    void setXelatexPath(const QString &path);
    void setPdfToCairoPath(const QString &path);
    void setInkscapePath(const QString &path);
    void setSvgTool(const QString &tool);
    void setTexInputs(const QString &texInputs);

    QString tempDirPath() const;
    QString pdfPath() const;
    QString pngPath() const;
    QString svgPath() const;
    QString logPath() const;

    void convertToPng(int dpi = 300);
    void convertToPng(const QString &pdfPath, int dpi = 300);
    void convertToSvg();
    void convertToSvg(const QString &pdfPath);
    bool convertToSvgBlocking(const QString &pdfPath, const QString &outSvgPath);
    QString xelatexCommand() const;
    QString pdfToCairoCommand() const;
    QString inkscapeCommand() const;
    QString svgTool() const;
    QString lastFullCommand() const { return m_lastFullCommand; }

    bool checkXelatexAvailable() const;
    bool checkPdfToCairoAvailable() const;
    bool checkInkscapeAvailable() const;

    QString wrapCode(const QString &texCode, const QString &templateId,
                    const QString &packages, const QString &tikzLibraries,
                    const QString &customCommands = QString()) const;

    static QString extractCustomCommands(const QString &texCode, QString &outCode);

    int userCodeStartLine() const;

signals:
    void compilationFinished(bool success, const QString &pdfPath, const QString &logOutput);
    void conversionFinished(bool success, const QString &outputPath);

private:
    QProcess *process;
    QString tempDir;
    QString currentCompileDir;
    QString templateDir;
    QString xelatexPath;
    QString pdfToCairoPath;
    QString inkscapePath;
    QString svgTool_;
    QString texInputs;
    int m_userCodeStartLine = 1;
    QString m_lastFullCommand;

    // Build the environment used for every launched subprocess (compile,
    // conversions, availability checks). The user-configured extra paths are
    // prepended to PATH (so tools like xelatex are found even when the app is
    // launched from a desktop entry with a minimal PATH) and added to TEXINPUTS.
    QProcessEnvironment processEnvironment() const;

    // Resolve a tool name to an absolute path, searching the user-configured
    // directories first and then the system PATH. Returns the input unchanged
    // if it is already an explicit path or cannot be found.
    QString resolveTool(const QString &name) const;

    QString loadTemplate(const QString &templateId) const;
};
