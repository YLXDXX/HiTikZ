#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <QHash>
#include <QTextDocument>
#include <QTextBlock>
#include <QRegularExpression>

class TikzDocumentState {
public:
    struct Scope {
        QString env;            // "tikzpicture","scope","axis","circuitikz","groupplot",...
        QStringList options;    // entry options
        int startPos;           // document position of \begin
        int endPos;             // document position of \end (or -1 if open)
        int braceDepth;         // brace depth at entry
    };

    struct CommandContext {
        QString name;           // "draw","node","path","fill","filldraw","shade",...
        QStringList options;    // [options] content
        int optionStart;        // position of '['
        int optionEnd;          // position of ']' or -1
        QString nodeName;       // (name) for nodes
        int startPos;           // position of the command backslash
        int bracketDepth;       // bracket depth at command start
    };

    struct NamedElement {
        QString name;
        QString kind;           // "style","coordinate","node","color","command","pic","foreach"
        QString detail;         // extra info (e.g. style content)
        int definedAt;
    };

    TikzDocumentState();
    ~TikzDocumentState();

    void reparse(const QTextDocument *doc);
    void reparseBlock(const QTextBlock &block, int blockPos);

    // Scope queries
    const Scope *currentScope(int position) const;
    const Scope *currentEnvironmentScope(int position) const;
    QString currentEnvName(int position) const;

    // Command queries
    const CommandContext *currentCommand(int position) const;
    QString currentCmdName(int position) const;

    // Active libraries (from \usetikzlibrary + snippet metadata)
    const QSet<QString> &activeLibs() const { return m_activeLibs; }
    void setSnippetLibraries(const QStringList &libs);
    void setSnippetPackages(const QStringList &pkgs);

    // User definitions
    const QHash<QString, QString> &userStyles() const { return m_userStyles; }
    const QSet<QString> &userCoordinates() const { return m_userCoords; }
    const QSet<QString> &userNodes() const { return m_userNodes; }
    const QSet<QString> &foreachVars() const { return m_foreachVars; }
    const QSet<QString> &userCommands() const { return m_userCmds; }
    const QHash<QString, QStringList> &definedColors() const { return m_colors; }
    const QSet<QString> &userPics() const { return m_userPics; }

    // All named user elements for completion
    QStringList allUserDefinitions() const;

    // Clear all state
    void clear();

private:
    void parseLine(const QString &text, int blockStartPos, int &braceDepth,
                   int &bracketDepth, bool &inComment);

    void handleBeginScope(const QString &envName, int pos, int braceDepth);
    void handleEndScope(const QString &envName, int pos);
    void handleCommand(const QString &text, const QString &cmdName,
                       int cmdStart, int &pos, int bracketDepth);

    QRegularExpression m_beginRe;
    QRegularExpression m_endRe;
    QRegularExpression m_cmdRe;
    QRegularExpression m_uselibRe;
    QRegularExpression m_tikzsetRe;
    QRegularExpression m_tikzstyleRe;
    QRegularExpression m_definecolorRe;
    QRegularExpression m_colorletRe;
    QRegularExpression m_coordinateRe;
    QRegularExpression m_foreachRe;
    QRegularExpression m_newcmdRe;
    QRegularExpression m_defRe;
    QRegularExpression m_nodeRe;
    QRegularExpression m_commentRe;

    QVector<Scope> m_scopeStack;
    QSet<QString> m_activeLibs;
    QSet<QString> m_tikzEnvSet;       // all drawing environment names

    QHash<QString, QString> m_userStyles;       // name -> style content
    QSet<QString> m_userCoords;
    QSet<QString> m_userNodes;
    QSet<QString> m_foreachVars;
    QSet<QString> m_userCmds;
    QHash<QString, QStringList> m_colors;       // name -> {model, spec}
    QSet<QString> m_userPics;

    // Parsed command contexts
    QVector<CommandContext> m_cmdContexts;

    // Block-level state
    QVector<int> m_blockScopeDepth;   // scope depth at start of each block
    QVector<int> m_blockBraceDepth;   // brace depth at start of each block
    int m_totalBlocks = 0;

    QStringList m_snippetLibs;
    QStringList m_snippetPkgs;
};
