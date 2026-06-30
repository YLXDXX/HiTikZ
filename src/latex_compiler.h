#pragma once
#include <QProcess>
#include <QObject>
#include <QString>
#include <QStringList>

class LatexCompiler : public QObject {
    Q_OBJECT
public:
    explicit LatexCompiler(QObject *parent = nullptr);
    ~LatexCompiler();

    void compile(const QString &texCode, const QString &templateId, const QString &snippetId,
                 const QString &packages = QString(), const QString &tikzLibraries = QString());
    void cancelCompile();
    void setTemplateDir(const QString &dir);
    void setXelatexPath(const QString &path);
    void setPdfToCairoPath(const QString &path);
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
    QString xelatexCommand() const;
    QString pdfToCairoCommand() const;

    static bool checkXelatexAvailable();
    static bool checkPdfToCairoAvailable();

    QString wrapCode(const QString &texCode, const QString &templateId,
                    const QString &packages, const QString &tikzLibraries) const;

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
    QString texInputs;
    int m_userCodeStartLine = 1;

    QString loadTemplate(const QString &templateId) const;
};
