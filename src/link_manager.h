#pragma once
#include <QObject>
#include <QString>

// Manages the "复制链接" picture links: symbolic links (named by sequence
// number, e.g. 0001.pdf, 0002.pdf, ...) placed into a user-configured
// directory (default ~/PicTikZ). Each link points at a snippet's preview.pdf
// so LaTeX documents can reference the pictures via
// \includegraphics{~/PicTikZ/0001.pdf} and automatically pick up changes
// when the snippet is recompiled.
//
// The configured directory string is kept verbatim (it may contain "~") for
// use inside the \includegraphics command written to the clipboard; filesystem
// operations use the expanded absolute path. The class deliberately does not
// touch QSettings so it can be unit-tested with arbitrary directories.
class LinkManager : public QObject {
    Q_OBJECT
public:
    // Default directory used when nothing is configured.
    static QString defaultDir();
    // Directory stored in QSettings ("link/dir"); falls back to defaultDir()
    // when unset or blank.
    static QString settingDir();
    // Directory path as configured (may contain "~", trailing slash removed).
    explicit LinkManager(const QString &configuredDir = QString(),
                         QObject *parent = nullptr);

    // Directory exactly as configured, e.g. "~/PicTikZ" (never empty; falls
    // back to defaultDir()). Used verbatim in includeCommand().
    QString configuredDir() const;
    // Filesystem path with "~" expanded to the home directory.
    QString dirPath() const;

    // LaTeX command for referencing a link file, e.g.
    // "\includegraphics{~/PicTikZ/0002.pdf}".
    QString includeCommand(const QString &linkName) const;

    // Full filesystem path of a link file.
    QString linkFilePath(const QString &linkName) const;

    // Ensure the link directory exists on disk.
    bool ensureDirExists() const;

    // True when a directory entry with that name exists (even a dangling
    // symlink), so sequence numbers stay occupied and are never reused while
    // any entry is present.
    bool linkEntryExists(const QString &linkName) const;
    // True when the link file's target actually exists on disk.
    bool linkTargetExists(const QString &linkName) const;

    // Smallest free sequence name "NNNN.pdf" starting at 0001, filling gaps
    // left by deleted links (0001.pdf, 0003.pdf -> 0002.pdf). Only entries
    // matching "<digits>.pdf" occupy numbers.
    QString nextFreeLinkName() const;

    // Create `linkName` as a symlink to `pdfPath` (falling back to a plain
    // copy on filesystems without symlink support). Any existing entry with
    // the same name is replaced first.
    bool createLink(const QString &pdfPath, const QString &linkName) const;
    // Remove a link file entry; returns true when the name is gone afterwards.
    bool removeLink(const QString &linkName) const;

private:
    QString m_configuredDir;
};
