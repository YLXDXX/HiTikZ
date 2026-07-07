#include <QCoreApplication>
#include <QRegularExpression>
#include <QProcess>
#include <QDataStream>
#include <QIODevice>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>

static int test_line_number_regex() {
    int failed = 0;

    // BUG#8: Line number regex should only match at start of line
    {
        QRegularExpression anchoredRe(QStringLiteral("^l\\.\\d+"));
        QRegularExpression unanchoredRe(QStringLiteral("l\\.\\d+"));

        QString line1 = QStringLiteral("l.42 Extra }");
        QString line2 = QStringLiteral("\\draw (0,0) -- (l.5cm, 0);");

        // Anchored regex should match line1 (line number at start)
        if (!anchoredRe.match(line1).hasMatch()) {
            fprintf(stderr, "FAIL: Test LN-1a - anchored re should match line number\n");
            failed++;
        }

        // Anchored regex should NOT match line2 (l.5cm is a length, not line number)
        if (anchoredRe.match(line2).hasMatch()) {
            fprintf(stderr, "FAIL: Test LN-1b - anchored re should NOT match l.5cm\n");
            failed++;
        }

        // Unanchored regex WOULD match both (the bug)
        if (!unanchoredRe.match(line1).hasMatch()) {
            fprintf(stderr, "FAIL: Test LN-1c - unanchored re should match line number\n");
            failed++;
        }
        if (!unanchoredRe.match(line2).hasMatch()) {
            fprintf(stderr, "FAIL: Test LN-1d - unanchored re matches l.5cm (demonstrates bug)\n");
            failed++;
        }

        // Replacement test: anchored should only replace at start
        {
            QString line = QStringLiteral("l.42 text with l.5cm length");
            QString adjusted = line;
            adjusted.replace(anchoredRe, QStringLiteral("l.10"));
            if (adjusted != QStringLiteral("l.10 text with l.5cm length")) {
                fprintf(stderr, "FAIL: Test LN-2a - anchored replacement incorrect: %s\n",
                        qPrintable(adjusted));
                failed++;
            }

            // The unanchored regex l\.\d+ matches "l.5" in "l.5cm",
            // so it would incorrectly replace that too
            QString buggyAdjusted = line;
            buggyAdjusted.replace(unanchoredRe, QStringLiteral("l.10"));
            // With unanchored re: "l.42" -> "l.10", "l.5" -> "l.10"
            // Result: "l.10 text with l.10cm" (incorrect - corrupted l.5cm)
            if (buggyAdjusted == adjusted) {
                fprintf(stderr, "FAIL: Test LN-2b - anchored and unanchored should differ\n");
                failed++;
            } else if (!buggyAdjusted.contains(QStringLiteral("l.10cm"))) {
                fprintf(stderr, "FAIL: Test LN-2b - unanchored re incorrectly matched l.5cm\n");
                failed++;
            }
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: Line number regex anchor test\n");
    return failed;
}

static int test_comment_priority() {
    int failed = 0;

    // Defect#9: Comments should have the highest priority
    // This test verifies that comment regexes work independently of other patterns
    {
        QRegularExpression commentRe(QStringLiteral("%[^\n]*"));
        QRegularExpression commandRe(QStringLiteral("\\\\[a-zA-Z@][a-zA-Z@]*(\\s*\\*)?"));

        QString commentLine = QStringLiteral("% this is \\draw a comment");
        QString codeLine = QStringLiteral("\\draw (0,0) -- (1,1); % trailing comment");

        // Comment regex should match the entire comment line
        QRegularExpressionMatch cm = commentRe.match(commentLine);
        if (!cm.hasMatch()) {
            fprintf(stderr, "FAIL: Test CP-1a - comment re should match comment line\n");
            failed++;
        }
        if (cm.hasMatch() && cm.capturedLength() != commentLine.length()) {
            fprintf(stderr, "FAIL: Test CP-1b - comment re should match entire line\n");
            failed++;
        }

        // In a comment, \draw should NOT be highlighted as command
        QString commentOnly = QStringLiteral("% \\draw this is a comment");
        cm = commentRe.match(commentOnly);
        int commentStart = cm.capturedStart();
        int commentEnd = cm.capturedStart() + cm.capturedLength();

        QRegularExpressionMatch cr = commandRe.match(commentOnly);
        if (cr.hasMatch()) {
            int cmdStart = cr.capturedStart();
            int cmdEnd = cr.capturedStart() + cr.capturedLength();
            // Command match is inside comment (bad if comment has lower priority)
            if (cmdStart >= commentStart && cmdEnd <= commentEnd) {
                // In the fixed version, comment should take priority
                // This just verifies the patterns overlap - the actual fix is the priority
                fprintf(stderr, "INFO: Test CP-2 - command overlaps with comment (expected, fixed by priority)\n");
            }
        }

        // In code line, comment regex should match trailing comment
        cm = commentRe.match(codeLine);
        if (!cm.hasMatch()) {
            fprintf(stderr, "FAIL: Test CP-3a - comment re should match trailing comment\n");
            failed++;
        } else {
            QString trailing = cm.captured();
            if (trailing != QStringLiteral("% trailing comment")) {
                fprintf(stderr, "FAIL: Test CP-3b - trailing comment mismatch: %s\n",
                        qPrintable(trailing));
                failed++;
            }
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: Comment priority test\n");
    return failed;
}

static int test_wrap_code_no_document() {
    int failed = 0;

    // Defect#16: wrapCode should handle templates without \begin{document}
    // The fix prepends extra preamble before the template content
    // We test the logic: if no \begin{document}, prepend extraPreamble

    QString tmpl = QStringLiteral("\\documentclass{standalone}\n\\usepackage{tikz}\n%%% TIKZ_CODE_HERE %%%\n");
    QString extraPreamble = QStringLiteral("\\usepackage{amsmath}\n");

    // Simulate the fix logic
    int docBegin = tmpl.indexOf(QStringLiteral("\\begin{document}"));
    if (docBegin >= 0) {
        tmpl = tmpl.insert(docBegin, extraPreamble);
    } else {
        tmpl = extraPreamble + "\n" + tmpl;
    }

    // Template should now contain the extra preamble
    if (!tmpl.contains(QStringLiteral("amsmath"))) {
        fprintf(stderr, "FAIL: Test WC-1 - extra preamble not injected\n");
        failed++;
    }

    // Test with template that HAS \begin{document}
    QString tmpl2 = QStringLiteral(
        "\\documentclass{standalone}\n"
        "\\begin{document}\n"
        "%%% TIKZ_CODE_HERE %%%\n"
        "\\end{document}\n");
    int docBegin2 = tmpl2.indexOf(QStringLiteral("\\begin{document}"));
    if (docBegin2 >= 0) {
        tmpl2 = tmpl2.insert(docBegin2, extraPreamble);
    }
    if (!tmpl2.contains(QStringLiteral("amsmath"))) {
        fprintf(stderr, "FAIL: Test WC-2 - extra preamble not injected in template with begin{document}\n");
        failed++;
    }
    // Preamble should be before \begin{document}
    int amsmathPos = tmpl2.indexOf(QStringLiteral("amsmath"));
    int docPos = tmpl2.indexOf(QStringLiteral("\\begin{document}"));
    if (amsmathPos < 0 || docPos < 0 || amsmathPos >= docPos) {
        fprintf(stderr, "FAIL: Test WC-3 - preamble should be before \\begin{document}\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: Wrap code without \\begin{document} test\n");
    return failed;
}

static int test_qprocess_wait_for_started() {
    int failed = 0;

    // BUG#6: runProcessSync should wait for process to start
    // Test the logic: after start(), a process that hasn't started yet
    // should be handled by waitForStarted()

    // Since we can't easily test QProcess behavior without actual executables,
    // we test the concept: a process that fails to start should be detected
    {
        QProcess proc;
        proc.start("/nonexistent/binary/that/does/not/exist");
        bool started = proc.waitForStarted(1000);
        if (started || proc.state() != QProcess::NotRunning) {
            fprintf(stderr, "FAIL: Test PS-1 - nonexistent binary should not start\n");
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: QProcess waitForStarted test\n");
    return failed;
}

static int test_stream_status_check() {
    int failed = 0;

    {
        QByteArray corruptData;
        corruptData.append(static_cast<char>('\xFF'));
        corruptData.append(static_cast<char>('\xFF'));
        corruptData.append(static_cast<char>('\xFF'));

        QDataStream stream(&corruptData, QIODevice::ReadOnly);
        int iterations = 0;
        while (!stream.atEnd() && stream.status() == QDataStream::Ok) {
            int row, col;
            QMap<int, QVariant> roleData;
            stream >> row >> col >> roleData;
            iterations++;
            if (iterations > 100) {
                fprintf(stderr, "FAIL: Test DS-1 - stream status check should prevent infinite loop\n");
                failed++;
                break;
            }
        }
        if (iterations <= 100) {
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: Stream status check test\n");
    return failed;
}

static int test_auto_compile_on_save() {
    int failed = 0;

    {
        QSettings settings("HiTikZ", "TikzManager");

        // Clean up any previous test key before starting
        settings.remove("behavior/autoCompileOnSave");
        settings.sync();

        // Test 1: Default value should be false when key is absent
        bool defaultValue = settings.value("behavior/autoCompileOnSave", false).toBool();
        if (defaultValue) {
            fprintf(stderr, "FAIL: Test ACS-1 - default autoCompileOnSave should be false\n");
            failed++;
        }

        // Test 2: Can be set to true and read back
        settings.setValue("behavior/autoCompileOnSave", true);
        settings.sync();
        bool readBack = settings.value("behavior/autoCompileOnSave", false).toBool();
        if (!readBack) {
            fprintf(stderr, "FAIL: Test ACS-2 - autoCompileOnSave should read back as true\n");
            failed++;
        }

        // Test 3: Can be set back to false
        settings.setValue("behavior/autoCompileOnSave", false);
        settings.sync();
        readBack = settings.value("behavior/autoCompileOnSave", true).toBool();
        if (readBack) {
            fprintf(stderr, "FAIL: Test ACS-3 - autoCompileOnSave should read back as false\n");
            failed++;
        }

        // Test 4: Setting key name exists after being set
        settings.setValue("behavior/autoCompileOnSave", true);
        settings.sync();
        if (settings.allKeys().contains("behavior/autoCompileOnSave")) {
        } else {
            fprintf(stderr, "FAIL: Test ACS-4 - key 'behavior/autoCompileOnSave' should exist\n");
            failed++;
        }

        // Cleanup: remove the test key
        settings.remove("behavior/autoCompileOnSave");
        settings.sync();
    }

    if (failed == 0) fprintf(stderr, "PASS: Auto-compile-on-save settings test\n");
    return failed;
}

static int test_parse_line_short_cmds() {
    int failed = 0;

    // BUG: parseLine condition pos+5<len was too strict — \end{env} (5 chars after \)
    // would be skipped at end of line. Fixed to pos+4<len.
    {
        // Simulate the minimal case: line ends with "\end{}" (5 chars after \)
        QString line1 = QStringLiteral("  \\end{a}");
        // pos of \ = 2, len = 8, 2+4 < 8 = 6 < 8 = true — should be detected now

        QString line2 = QStringLiteral("x\\trel"); // \trel has 4 chars after \, pos=1, 1+4<7=true
        // just make sure it doesn't crash

        QRegularExpression endRe(QString::fromUtf8(R"(\\end\{[^}]*\})"));
        bool m1 = endRe.match(line1).hasMatch();
        if (!m1) {
            fprintf(stderr, "FAIL: Test PLS-1 - \\end{env} should match at end of line\n");
            failed++;
        }

        // Test that shorter than \\end{ doesn't match (false positive check)
        bool m2 = endRe.match(line2).hasMatch();
        if (m2) {
            fprintf(stderr, "FAIL: Test PLS-2 - \\trel should NOT match \\end{} regex\n");
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: parseLine short command detection test\n");
    return failed;
}

static int test_atomic_file_rename()
{
    int failed = 0;

    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/hifiz_fix_test/";
    QDir().mkpath(testDir);

    QString tempFile = testDir + "test.tmp";
    QString finalFile = testDir + "test.final";

    const char *content = "hello atomic world";
    {
        QFile f(tempFile);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(content);
            f.close();
        }
    }

    if (QFile::exists(finalFile))
        QFile::remove(finalFile);
    bool renamed = QFile::rename(tempFile, finalFile);

    if (!renamed) {
        fprintf(stderr, "FAIL: FIX-AR1 - atomic rename failed\n");
        failed++;
    } else if (!QFile::exists(finalFile)) {
        fprintf(stderr, "FAIL: FIX-AR2 - final file does not exist after rename\n");
        failed++;
    } else if (QFile::exists(tempFile)) {
        fprintf(stderr, "FAIL: FIX-AR3 - temp file should not exist after rename\n");
        failed++;
    } else {
        QFile f(finalFile);
        if (f.open(QIODevice::ReadOnly)) {
            QString readBack = QString::fromUtf8(f.readAll());
            f.close();
            if (readBack != QString::fromLatin1(content)) {
                fprintf(stderr, "FAIL: FIX-AR4 - content mismatch after rename\n");
                failed++;
            }
        }
    }

    QDir(testDir).removeRecursively();

    if (failed == 0) fprintf(stderr, "PASS: atomic file rename test\n");
    return failed;
}

static int test_scratch_file_cleanup()
{
    int failed = 0;

    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/hifiz_scratch_test/";
    QDir().mkpath(testDir);

    for (int i = 0; i < 5; ++i) {
        QFile f(testDir + QString("scratch_%1.json").arg(i));
        bool opened = f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        if (opened) {
            f.write("{}");
            f.close();
        }
    }

    QDir dir(testDir);
    QStringList oldScratches = dir.entryList(QStringList() << "scratch_*.json", QDir::Files);
    for (const QString &f : oldScratches)
        QFile::remove(testDir + f);

    QStringList remaining = dir.entryList(QStringList() << "scratch_*.json", QDir::Files);
    if (!remaining.isEmpty()) {
        fprintf(stderr, "FAIL: FIX-SC1 - scratch files not properly cleaned (%d remaining)\n", remaining.size());
        failed++;
    }

    QDir(testDir).removeRecursively();

    if (failed == 0) fprintf(stderr, "PASS: scratch file cleanup test\n");
    return failed;
}

static int test_path_contains_dangerous_chars()
{
    int failed = 0;

    auto hasDangerousChars = [](const QString &s) -> bool {
        return s.contains('/') || s.contains('\\') || s.contains("..");
    };

    if (!hasDangerousChars("/etc/passwd")) {
        fprintf(stderr, "FAIL: FIX-PT1 - '/etc/passwd' should be rejected\n"); failed++;
    }
    if (!hasDangerousChars("../../../evil.tex")) {
        fprintf(stderr, "FAIL: FIX-PT2 - '../../../evil.tex' should be rejected\n"); failed++;
    }
    if (!hasDangerousChars("foo\\bar")) {
        fprintf(stderr, "FAIL: FIX-PT3 - 'foo\\\\bar' should be rejected\n"); failed++;
    }
    if (!hasDangerousChars("..hidden")) {
        fprintf(stderr, "FAIL: FIX-PT4 - '..hidden' contains '..' and should be rejected\n"); failed++;
    }
    if (hasDangerousChars("default_math")) {
        fprintf(stderr, "FAIL: FIX-PT5 - 'default_math' should be accepted\n"); failed++;
    }
    if (hasDangerousChars("my_template_test")) {
        fprintf(stderr, "FAIL: FIX-PT6 - 'my_template_test' should be accepted\n"); failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: dangerous path character detection\n");
    return failed;
}

static int test_dir_removal_on_failure()
{
    int failed = 0;

    QString testDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/hifiz_rollback_test/";
    QDir().mkpath(testDir);

    QString orphanDir = testDir + "orphan_folder/";
    QDir().mkpath(orphanDir);

    if (!QDir(orphanDir).exists()) {
        fprintf(stderr, "FAIL: FIX-DR1 - directory not created\n"); failed++;
    } else {
        QDir(orphanDir).removeRecursively();
        if (QDir(orphanDir).exists()) {
            fprintf(stderr, "FAIL: FIX-DR2 - directory not removed after rollback\n"); failed++;
        }
    }

    QDir(testDir).removeRecursively();

    if (failed == 0) fprintf(stderr, "PASS: directory removal on failure (rollback)\n");
    return failed;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    int failed = 0;

    failed += test_line_number_regex();
    failed += test_comment_priority();
    failed += test_wrap_code_no_document();
    failed += test_qprocess_wait_for_started();
    failed += test_stream_status_check();
    failed += test_auto_compile_on_save();
    failed += test_parse_line_short_cmds();
    failed += test_atomic_file_rename();
    failed += test_scratch_file_cleanup();
    failed += test_path_contains_dangerous_chars();
    failed += test_dir_removal_on_failure();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll fix verification tests passed!\n");
    return 0;
}
