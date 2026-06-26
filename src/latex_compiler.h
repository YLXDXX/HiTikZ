#pragma once
#include <QProcess>
#include <QObject>

class LatexCompiler : public QObject {
    Q_OBJECT
public:
    explicit LatexCompiler(QObject *parent = nullptr) : QObject(parent), process(nullptr) {}
    void compile(const QString &texCode, const QString &templateId, const QString &snippetId) {
        (void)texCode; (void)templateId; (void)snippetId;
    }

signals:
    void compilationFinished(bool success, const QString &pdfPath, const QString &logOutput);

private:
    QProcess *process;
    QString tempDir;
    QString wrapCode(const QString &texCode, const QString &templateId) {
        (void)texCode; (void)templateId; return QString();
    }
    void convertToPng(const QString &pdfPath) { (void)pdfPath; }
};
