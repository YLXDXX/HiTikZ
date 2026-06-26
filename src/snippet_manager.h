#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>

struct Snippet {
    QString id;
    QString name;
    QString description;
    QString category;
    QString templateId;
    QString code;
};

class SnippetManager : public QObject {
    Q_OBJECT
public:
    SnippetManager(QObject *parent = nullptr) : QObject(parent) {}
    QString getBasePath() const { return QString(); }
    Snippet loadSnippet(const QString &id) { (void)id; return Snippet(); }
    bool saveSnippet(const Snippet &s) { (void)s; return false; }
    QList<Snippet> getAllSnippets() { return QList<Snippet>(); }

private:
    QString basePath;
    QString getSnippetPath(const QString &id) { (void)id; return QString(); }
    QJsonObject snippetToJson(const Snippet &s) { (void)s; return QJsonObject(); }
    Snippet jsonToSnippet(const QJsonObject &obj) { (void)obj; return Snippet(); }
};
