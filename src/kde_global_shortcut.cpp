#include "kde_global_shortcut.h"

#ifdef HAS_KGLOBALACCEL

KdeGlobalShortcut *KdeGlobalShortcut::instance()
{
    static KdeGlobalShortcut s;
    return &s;
}

KdeGlobalShortcut::KdeGlobalShortcut(QObject *parent)
    : QObject(parent) { }

void KdeGlobalShortcut::registerShortcut(const QString &id,
                                          const QString &description,
                                          const QString &defaultShortcut)
{
    unregisterShortcut(id);

    QAction *action = new QAction(this);
    action->setObjectName(id);
    action->setText(description);

    connect(action, &QAction::triggered, this, [this, id]() {
        emit activated(id);
    });

    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence(defaultShortcut);

    KGlobalAccel::self()->setShortcut(action, shortcuts, KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setDefaultShortcut(action, shortcuts);

    m_actions[id] = action;
}

void KdeGlobalShortcut::unregisterShortcut(const QString &id)
{
    if (m_actions.contains(id)) {
        QAction *action = m_actions.take(id);
        KGlobalAccel::self()->removeAllShortcuts(action);
        delete action;
    }
}

#include "moc_kde_global_shortcut.cpp"

#endif // HAS_KGLOBALACCEL
