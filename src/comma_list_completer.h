#pragma once
#include <QCompleter>
#include <QLineEdit>
#include <QStringList>

// A QCompleter for QLineEdit fields that hold a comma-separated list (e.g. the
// snippet metadata "额外宏包" packages field and "TikZ库" libraries field).
// Only the segment currently being typed — the text after the last comma — is
// matched against the word list, and accepting a completion replaces just that
// segment while preserving the earlier entries. Whitespace around each segment
// is tolerated.
//
// Only QCompleter virtuals are overridden (no new signals/slots), so the
// Q_OBJECT macro / moc are intentionally not required.
class CommaListCompleter : public QCompleter {
public:
    explicit CommaListCompleter(const QStringList &words, QObject *parent = nullptr)
        : QCompleter(words, parent)
    {
        setCaseSensitivity(Qt::CaseInsensitive);
        setCompletionMode(QCompleter::PopupCompletion);
        setMaxVisibleItems(12);
    }

    // Filter the popup on the last comma-separated segment (trimmed) instead of
    // the whole field, so "calc, thr" matches libraries starting with "thr".
    QStringList splitPath(const QString &path) const override
    {
        const int comma = path.lastIndexOf(QLatin1Char(','));
        const QString last = (comma < 0) ? path : path.mid(comma + 1);
        return { last.trimmed() };
    }

    // Rebuild the full field text on accept: keep everything up to (and
    // including) the last comma, then append the chosen word. A single space is
    // inserted after the comma to match the placeholder style "calc, er, angles".
    QString pathFromIndex(const QModelIndex &index) const override
    {
        const QString completion = QCompleter::pathFromIndex(index);

        const auto *edit = qobject_cast<const QLineEdit *>(widget());
        if (!edit) return completion;

        const QString current = edit->text();
        const int comma = current.lastIndexOf(QLatin1Char(','));
        if (comma < 0)
            return completion;

        QString head = current.left(comma + 1);
        if (!head.endsWith(QLatin1Char(' ')))
            head += QLatin1Char(' ');
        return head + completion;
    }
};
