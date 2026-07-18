#pragma once
#include <QAbstractItemView>
#include <QColor>
#include <QCompleter>
#include <QIcon>
#include <QLineEdit>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStandardItemModel>
#include <QStringList>

// A QCompleter for QLineEdit fields that hold a comma-separated list (e.g. the
// snippet metadata "额外宏包" packages field and "TikZ库" libraries field).
// Only the segment currently being typed — the text after the last comma — is
// matched against the word list, and accepting a completion replaces just that
// segment while preserving the earlier entries. Whitespace around each segment
// is tolerated.
//
// The popup is deliberately high-contrast so completion items are recognizable
// at a glance (users found the stock popup indistinguishable from the fields
// underneath): every item carries a colored dot in the field's accent color,
// the popup background is tinted with the same accent (theme-aware: blended
// with the palette base so it works in light and dark themes), the list uses a
// monospace font (entries are code identifiers), and the selected row is
// filled with the accent color.
//
// Only QCompleter virtuals are overridden (no new signals/slots), so the
// Q_OBJECT macro / moc are intentionally not required.
class CommaListCompleter : public QCompleter {
public:
    explicit CommaListCompleter(const QStringList &words, QObject *parent = nullptr,
                                const QColor &accent = QColor(0x2e, 0x86, 0xc1))
        : QCompleter(parent)
    {
        auto *model = new QStandardItemModel(this);
        const QIcon dot = makeDotIcon(accent);
        for (const QString &w : words) {
            auto *item = new QStandardItem(dot, w);
            item->setEditable(false);
            model->appendRow(item);
        }
        setModel(model);
        setCaseSensitivity(Qt::CaseInsensitive);
        setCompletionMode(QCompleter::PopupCompletion);
        setMaxVisibleItems(12);
        stylePopup(accent);
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

private:
    // Weighted blend: t = 0 → a, t = 1 → b.
    static QColor blend(const QColor &a, const QColor &b, qreal t)
    {
        return QColor(qRound(a.red()   * (1 - t) + b.red()   * t),
                      qRound(a.green() * (1 - t) + b.green() * t),
                      qRound(a.blue()  * (1 - t) + b.blue()  * t));
    }

    static QIcon makeDotIcon(const QColor &color)
    {
        QPixmap pm(24, 24);
        pm.setDevicePixelRatio(2.0);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawEllipse(QRectF(1.5, 1.5, 9.0, 9.0));
        p.end();
        return QIcon(pm);
    }

    void stylePopup(const QColor &accent)
    {
        QAbstractItemView *view = popup();
        if (!view) return;

        QFont f = view->font();
        f.setFamily(QStringLiteral("monospace"));
        view->setFont(f);
        view->setIconSize(QSize(12, 12));

        const QPalette pal = view->palette();
        // Tint the popup with the accent so it visibly stands out from the
        // plain line edits underneath, in light and dark themes alike.
        const QColor bg = blend(pal.color(QPalette::Base), accent, 0.10);
        const QColor fg = pal.color(QPalette::Text);
        // Accent-filled selection; pick black/white text by accent luminance.
        const QColor selFg = (accent.lightness() < 140) ? QColor(Qt::white)
                                                        : QColor(Qt::black);
        view->setStyleSheet(QStringLiteral(
            "QListView {"
            " background-color: %1;"
            " color: %2;"
            " border: 1px solid %3;"
            " border-radius: 4px;"
            " padding: 2px;"
            " outline: none;"
            "}"
            "QListView::item {"
            " padding: 3px 6px;"
            " border-radius: 3px;"
            "}"
            "QListView::item:selected {"
            " background-color: %3;"
            " color: %4;"
            "}")
            .arg(bg.name(), fg.name(), accent.name(), selFg.name()));
    }
};
