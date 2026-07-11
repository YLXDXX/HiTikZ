#include "tikz_completer.h"
#include "tikz_words.h"
#include "tikz_keywords.h"
#include "tikz_document_state.h"
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTextCursor>
#include <QRegularExpression>
#include <QWindow>

TikzCompleter::Context TikzCompleter::detectContext(const QString &textBefore) const
{
    if (textBefore.isEmpty()) return TkzCtxNone;
    int len = textBefore.length();
    if (len == 0) return TkzCtxNone;

    QChar lastChar = textBefore.at(len - 1);

    // Trigger command completion when \ is preceded by any non-letter char,
    // not just whitespace — so {\draw and (\node also work.
    if (lastChar == '\\' && (len == 1 || !textBefore.at(len - 2).isLetter()))
        return TkzCtxCmd;

    static const QRegularExpression beginRe(QStringLiteral("\\\\begin\\s*\\{"));
    int lastBeginIdx = textBefore.lastIndexOf(beginRe);
    if (lastBeginIdx >= 0) {
        QString afterBegin = textBefore.mid(lastBeginIdx);
        if (afterBegin.count('{') > afterBegin.count('}'))
            return TkzCtxBeg;
    }

    static const QRegularExpression endRe(QStringLiteral("\\\\end\\s*\\{"));
    int lastEndIdx = textBefore.lastIndexOf(endRe);
    if (lastEndIdx >= 0) {
        QString afterEnd = textBefore.mid(lastEndIdx);
        if (afterEnd.count('{') > afterEnd.count('}'))
            return TkzCtxEnd;
    }

    if (textBefore.contains(QRegularExpression(QStringLiteral("\\\\usetikzlibrary\\s*\\{")))) {
        int idx = textBefore.lastIndexOf(QRegularExpression(QStringLiteral("\\\\usetikzlibrary")));
        QString after = textBefore.mid(idx);
        if (after.count('{') > after.count('}'))
            return TkzCtxLib;
    }

    if (lastChar == '=') {
        // Use brace-depth-aware scanning so {a=b} inside a value doesn't
        // falsely trigger Eq context.
        int eqIdx = governingEqIndex(textBefore);
        if (eqIdx > 0 && textBefore.left(eqIdx).contains(QRegularExpression(QStringLiteral("[\\w\\s]"))))
            return TkzCtxEq;
    }

    if (lastChar == '.') {
        if (len >= 2) {
            QChar prev = textBefore.at(len - 2);
            if (prev.isLetter() || prev == '/')
                return TkzCtxDot;
        }
    }

    {
        int dotIdx = textBefore.lastIndexOf('.');
        if (dotIdx >= 0 && dotIdx < len - 1) {
            QChar beforeDot = textBefore.at(dotIdx - 1);
            if (beforeDot.isLetterOrNumber() || beforeDot == '/') {
                QString afterDot = textBefore.mid(dotIdx + 1);
                bool allWordChars = true;
                for (const QChar &c : afterDot) {
                    if (c.isSpace()) break;
                    if (!c.isLetterOrNumber() && c != '_' && c != '-')
                    { allWordChars = false; break; }
                }
                if (allWordChars && afterDot.length() >= 1)
                    return TkzCtxDot;
            }
        }
    }

    {
        int atCount = textBefore.count("@@");
        if (atCount > 0 && atCount % 2 == 1)
            return TkzCtxAt;
        if (lastChar == '@' && !textBefore.endsWith("@@"))
            return TkzCtxAt;
    }

    // Check for coordinate context: (name after an opening (
    {
        int lastOpenParen = textBefore.lastIndexOf('(');
        int lastCloseParen = textBefore.lastIndexOf(')');
        if (lastOpenParen > lastCloseParen && lastOpenParen >= 0) {
            QString afterParen = textBefore.mid(lastOpenParen + 1);
            // Exclude coordinate pairs (x,y) and calc expressions ($...$), but
            // allow names with spaces (e.g. "critical 1") so they can be
            // completed after a '('.
            if (!afterParen.contains(',') &&
                !afterParen.startsWith('$') &&
                afterParen.length() >= 0 && afterParen.length() < 30)
                return TkzCtxCoord;
        }
    }

    int bracketDepth = 0;
    int braceDepth = 0;
    for (int i = len - 1; i >= 0; --i) {
        QChar ch = textBefore.at(i);
        if (ch == '}') braceDepth++;
        else if (ch == '{') { if (braceDepth > 0) braceDepth--; }
        else if (ch == ']' && braceDepth == 0) bracketDepth++;
        else if (ch == '[' && braceDepth == 0) {
            if (bracketDepth > 0) bracketDepth--;
            else {
                // Reached the unclosed '[' that encloses the cursor. The
                // bracketDepth/braceDepth tracking above already guarantees this
                // '[' is genuinely open, so any ']' remaining after it lives
                // inside a value's braces (e.g. >={Stealth[round]}) and must be
                // ignored — a naive contains(']') check would misfire here and
                // suppress completion. Decide key (TkzCtxBrk) vs value
                // (TkzCtxEq) using the same brace-depth-aware scan.
                const QString afterBracket = textBefore.mid(i + 1);
                int eqIdx = -1, commaIdx = -1, bd = 0;
                for (int k = afterBracket.length() - 1; k >= 0; --k) {
                    const QChar c = afterBracket.at(k);
                    if (c == '}') { bd++; }
                    else if (c == '{') { if (bd > 0) bd--; }
                    else if (bd == 0) {
                        if (c == '=' && eqIdx < 0) eqIdx = k;
                        else if (c == ',' && commaIdx < 0) commaIdx = k;
                    }
                    if (eqIdx >= 0 && commaIdx >= 0) break;
                }
                if (eqIdx >= 0 && (commaIdx < 0 || commaIdx <= eqIdx))
                    return TkzCtxEq;
                return TkzCtxBrk;
            }
        }
    }

    // Inside a brace group, the intersections library's 'of=' value takes two
    // path names joined by " and " (e.g. \path[name intersections={of=D--F and
    // circle K}]). Because that value contains spaces, the generic word-boundary
    // logic below would misclassify it; the single-word in-brace cases
    // (e.g. {fill=re, {>=s) are already handled there via the '=' boundary.
    // Scope this to the 'of' key only so ordinary node text containing '='
    // (e.g. \node {x = y}) is never mistaken for a value context.
    {
        const int eqIdx = governingEqIndex(textBefore);
        if (eqIdx >= 0) {
            // Extract the key immediately preceding this '='.
            int ks = eqIdx - 1;
            while (ks >= 0 && (textBefore.at(ks).isLetterOrNumber()
                               || textBefore.at(ks) == ' '))
                ks--;
            const QString key = textBefore.mid(ks + 1, eqIdx - ks - 1).trimmed();
            if (key.compare(QStringLiteral("of"), Qt::CaseInsensitive) == 0) {
                const QString afterEq = textBefore.mid(eqIdx + 1);
                if (!afterEq.contains(QLatin1Char(',')))
                    return TkzCtxEq;
            }
        }
    }

    if (lastChar.isLetterOrNumber() || lastChar == '_') {
        static const QRegularExpression wordBoundary(QStringLiteral("[\\s\\\\\\[\\{=(,\"]"));
        int wordStart = textBefore.lastIndexOf(wordBoundary);
        if (wordStart >= 0) {
            QChar bc = textBefore.at(wordStart);
            if (bc == '\\') return TkzCtxCmd;
            if (bc == '(') return TkzCtxCoord;
            if (bc == '=') return TkzCtxEq;
        }
        QString word;
        int j = len - 1;
        while (j >= 0 && (textBefore.at(j).isLetterOrNumber()
                          || textBefore.at(j) == '_' || textBefore.at(j) == '-'
                          || textBefore.at(j) == '@'))
            j--;
        word = textBefore.mid(j + 1);
        if (word.length() >= 2) {
            // Distinguish a bare word in the path body (e.g. "\draw (0,0)
            // rectangle") from one inside a '{...}' group (node text / math
            // expression such as "{reciprocal(x)}"). In the path body only path
            // operations are valid, so route there; inside braces keep the
            // generic word list (which includes math functions, etc.).
            if (textBefore.count('{') <= textBefore.count('}'))
                return TkzCtxPathWord;
            return TkzCtxWord;
        }
    }

    return TkzCtxNone;
}
