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

int TikzCompleter::governingOpenParenIndex(const QString &textBefore)
{
    int parenDepth = 0;
    int braceDepth = 0;
    for (int i = textBefore.length() - 1; i >= 0; --i) {
        const QChar ch = textBefore.at(i);
        if (ch == QLatin1Char('}')) {
            braceDepth++;
        } else if (ch == QLatin1Char('{')) {
            if (braceDepth > 0) braceDepth--;
        } else if (braceDepth == 0) {
            if (ch == QLatin1Char(')')) {
                parenDepth++;
            } else if (ch == QLatin1Char('(')) {
                if (parenDepth > 0) parenDepth--;
                else return i;
            }
        }
    }
    return -1;
}

int TikzCompleter::lastTopLevelCommaIndex(const QString &segment)
{
    // Forward scan so a still-unclosed brace value ("...={(0,1)--(2,3") also
    // shields its commas: everything after the unmatched '{' is at depth > 0.
    int braceDepth = 0;
    int last = -1;
    for (int i = 0; i < segment.length(); ++i) {
        const QChar ch = segment.at(i);
        if (ch == QLatin1Char('{')) braceDepth++;
        else if (ch == QLatin1Char('}')) { if (braceDepth > 0) braceDepth--; }
        else if (ch == QLatin1Char(',') && braceDepth == 0) last = i;
    }
    return last;
}

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
                // If a closing ')' appears between the dot and the cursor,
                // the dot belongs to an already-closed coordinate expression
                // (e.g. "(FF.pin 1) -- (FF" — the dot is from "FF.pin", not
                // the current group). Skip so the coordinate context can fire.
                if (afterDot.contains(QLatin1Char(')')))
                    goto dot_skip;
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
        dot_skip: ;
    }

    {
        int atCount = textBefore.count("@@");
        if (atCount > 0 && atCount % 2 == 1)
            return TkzCtxAt;
        if (lastChar == '@' && !textBefore.endsWith("@@"))
            return TkzCtxAt;
    }

    // Check for coordinate context: name/cs keys after the innermost unclosed
    // '('. The paren must be found with a depth-aware scan — closed pairs
    // nested in an already-typed value (e.g. "first line={(A)--(B)}") would
    // otherwise hide the governing paren and kill completion for the next key.
    {
        int lastOpenParen = governingOpenParenIndex(textBefore);
        if (lastOpenParen >= 0) {
            QString afterParen = textBefore.mid(lastOpenParen + 1);
            // Coordinate-system usage: "(<name> cs:key=value,key=value)".
            // After the "cs:" marker the comma-separated tokens are option keys
            // (e.g. angle/radius/z for "xyz cylindrical"). Offer key completion
            // for the segment currently being typed, but stay silent while the
            // value part (after '=') is entered. Only commas at brace depth 0
            // separate keys — commas inside a value ({(0,1)--(2,3)}) do not.
            int csIdx = afterParen.indexOf(QLatin1String("cs:"));
            if (csIdx >= 0) {
                QString afterCs = afterParen.mid(csIdx + 3);
                int lastComma = lastTopLevelCommaIndex(afterCs);
                QString seg = (lastComma >= 0) ? afterCs.mid(lastComma + 1)
                                               : afterCs;
                if (!seg.contains('='))
                    return TkzCtxCoordSysKey;
                return TkzCtxNone;
            }
            // Exclude coordinate pairs (x,y) and calc expressions ($...$), but
            // allow names with spaces (e.g. "critical 1") so they can be
            // completed after a '('. Nested closed pairs after the governing
            // paren mean the cursor is past a complete sub-coordinate — not a
            // name being typed — so keep the historical "no ')' after the open
            // paren" behavior for this branch.
            if (!afterParen.contains(',') &&
                !afterParen.contains(')') &&
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

    // Inside a brace group, some keys take values the generic word-boundary
    // logic below would misclassify:
    //   • the intersections 'of=' value joins two path names with " and "
    //     (e.g. \path[name intersections={of=D--F and circle K}]), and
    //   • 'font='/'node font=' values are backslash macros (\itshape, \ttfamily,
    //     ...) which would otherwise be rerouted to command completion when
    //     written in a style body (e.g. foo/.style={draw,font=\it}).
    // Scope this to those keys so ordinary node text containing '=' or '\'
    // (e.g. \node {x = y}) is never mistaken for a value context. The
    // single-word in-brace cases (e.g. {fill=re, {>=s) remain handled by the
    // '=' word boundary below.
    {
        const int eqIdx = governingEqIndex(textBefore);
        if (eqIdx >= 0) {
            // Extract the key immediately preceding this '='.
            int ks = eqIdx - 1;
            while (ks >= 0 && (textBefore.at(ks).isLetterOrNumber()
                               || textBefore.at(ks) == ' '))
                ks--;
            const QString key = textBefore.mid(ks + 1, eqIdx - ks - 1).trimmed();
            const QString lkey = key.toLower();
            const QString afterEq = textBefore.mid(eqIdx + 1);
            if (lkey == QLatin1String("of")) {
                if (!afterEq.contains(QLatin1Char(',')))
                    return TkzCtxEq;
            } else if (lkey == QLatin1String("font")
                       || lkey == QLatin1String("node font")) {
                // Value is a run of font macros like "\bfseries\itshape"; stay in
                // value context as long as there is no comma ending the option.
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
