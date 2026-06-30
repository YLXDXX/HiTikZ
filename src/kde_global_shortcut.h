#pragma once
#include <QObject>
#include <QAction>
#include <QKeySequence>

#ifdef HAS_KGLOBALACCEL
#include <KGlobalAccel>

class KdeGlobalShortcut : public QObject
{
    Q_OBJECT
public:
    static KdeGlobalShortcut *instance();

    void registerShortcut(const QString &id, const QString &description,
                          const QString &defaultShortcut);
    void unregisterShortcut(const QString &id);

signals:
    void activated(const QString &id);

private:
    explicit KdeGlobalShortcut(QObject *parent = nullptr);
    QMap<QString, QAction *> m_actions;
};
#endif // HAS_KGLOBALACCEL
