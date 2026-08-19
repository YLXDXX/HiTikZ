#pragma once
#include <QMap>
#include <QString>
#include <QStringList>
#include <functional>

// Command-line project packaging for the "复制链接" workflow:
//
//   hitikz pack [选项] <TeX文件或目录> <目标目录>
//
// The shared picture directory (default ~/PicTikZ, configurable in the
// settings panel) holds the sequence-named PDF links created by the toolbar
// 「复制链接」 action. This tool copies those PDFs into a LaTeX document
// project as plain files (so the project can be shared) and rewrites the
// corresponding \includegraphics references inside the processed .tex files.
//
// Options:
//   --link-dir <目录>   链接图片所在目录（默认读取程序设置 link/dir）
//   --name-format <格式> 新文件名格式: 01 / 001 / 0001（默认 001）
//   --overwrite         覆盖目标目录中的同名文件（默认跳过并顺延编号）
//   --copy-sources      同时复制完整 TeX 源码及关联图片（默认只复制 PDF）
//   --help              显示帮助

class ProjectPackager {
public:
    struct PackOptions {
        // .tex file, wildcard pattern (e.g. *.tex) or directory name.
        QString texArg;
        // Directory receiving the copied PDFs; kept verbatim for the rewritten
        // \includegraphics references.
        QString destDir;
        // Picture-link directory exactly as configured (may contain "~");
        // empty means "read the program setting".
        QString linkDir;
        // New-name zero-padding: 2 (01), 3 (001) or 4 (0001).
        int nameWidth = 3;
        bool overwrite = false;
        bool copySources = false;
        bool showHelp = false;
    };

    struct PackResult {
        bool ok = false;
        QString errorMessage;
        // Lines printed to the terminal (skip / overwrite / copy / rewrite info).
        QStringList messages;
        // Processed .tex files (expanded from texArg).
        QStringList texFiles;
        // Numbered "<digits>.pdf" entries found in the link directory.
        QStringList linkFiles;
        // Successful copies: link file name -> new file name (e.g. 0001.pdf -> 023.pdf).
        QMap<QString, QString> renamed;
        // Total number of \includegraphics path replacements performed.
        int referenceReplacements = 0;
    };

    // Outcome of a full CLI run (parsing + execution), used by runCli() and
    // unit-testable without spawning a process.
    struct CliOutcome {
        int exitCode = 0; // 0 success, 1 runtime error, 2 usage error
        QStringList stdoutLines;
        QStringList stderrLines;
    };

    // Parse the arguments following the "pack" subcommand. On success fills
    // `opts` (showHelp=true when --help was given) and returns true;
    // otherwise `error` explains the problem and `usage` carries the help
    // text.
    static bool parsePackArgs(const QStringList &args, PackOptions &opts,
                              QString &error, QString &usage);

    // Expand the tex argument to a list of .tex file paths: a directory
    // yields its *.tex entries, a pattern (* / ?) is matched in its parent
    // directory, anything else is treated as a single file path.
    static QStringList expandTexArgument(const QString &texArg);

    // Numbered "<digits>.pdf" entries in the link directory (dangling
    // symlinks included), sorted numerically.
    static QStringList collectLinkFiles(const QString &linkDirAbs);

    // Zero-padded number with at least `width` digits (grows when needed).
    static QString formatNumber(int number, int width);

    // Rewrite \includegraphics[*][options]{path} occurrences in `content`.
    // For every non-commented occurrence the matcher receives the raw path
    // text (inside the braces, trimmed) and returns the replacement path, or
    // an empty string to keep the occurrence unchanged. Only the path text is
    // ever replaced — options, whitespace and everything else stay intact.
    // `count` (optional) receives the number of replacements performed.
    static QString rewriteIncludes(const QString &content,
                                   const std::function<QString(const QString &)> &matcher,
                                   int *count = nullptr);

    // Full packaging flow. Never touches QSettings for the link directory
    // unless opts.linkDir is empty.
    static PackResult pack(const PackOptions &opts);

    // Parse the arguments and execute the packaging without printing
    // (stdout/stderr lines are collected in the returned outcome).
    static CliOutcome runPack(const QStringList &args);

    // CLI entry point for the "pack" subcommand (args exclude the "pack"
    // token). Creates a QCoreApplication and returns the process exit code.
    static int runCli(const QStringList &args);
};
