#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QDebug>
#include "autostart_manager.h"

static int g_failed = 0;

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            qDebug() << "FAIL:" << msg; \
            g_failed++; \
        } else { \
            qDebug() << "PASS:" << msg; \
        } \
    } while (0)

static QString readAll(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(f.readAll());
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // --- desktopFileContents -------------------------------------------------
    {
        const QString c = AutostartManager::desktopFileContents(
            QStringLiteral("/usr/local/bin/hitikz"));
        CHECK(c.startsWith(QStringLiteral("[Desktop Entry]\n")),
              "contents start with [Desktop Entry]");
        CHECK(c.contains(QStringLiteral("\nExec=/usr/local/bin/hitikz --hidden\n")),
              "Exec line carries --hidden");
        CHECK(c.contains(QStringLiteral("\nType=Application\n")), "Type=Application present");
        CHECK(c.contains(QStringLiteral("\nIcon=hitikz\n")), "Icon present");
        CHECK(c.contains(QStringLiteral("\nTerminal=false\n")), "Terminal=false present");
        CHECK(c.contains(QStringLiteral("\nX-KDE-autostart-after=panel\n")),
              "KDE autostart ordering hint present");
    }

    // Paths containing spaces must be quoted in Exec=
    {
        const QString c = AutostartManager::desktopFileContents(
            QStringLiteral("/opt/my apps/hitikz"));
        CHECK(c.contains(QStringLiteral("Exec=\"/opt/my apps/hitikz\" --hidden")),
              "Exec path with spaces is quoted");
    }

    // --- setEnabled / isEnabled ----------------------------------------------
    {
        QTemporaryDir tmp;
        CHECK(tmp.isValid(), "temporary dir created");
        const QString dir = tmp.path() + QStringLiteral("/autostart");

        CHECK(!AutostartManager::isEnabled(dir), "initially disabled");

        CHECK(AutostartManager::setEnabled(true, QStringLiteral("/usr/bin/hitikz"), dir),
              "setEnabled(true) succeeds");
        CHECK(AutostartManager::isEnabled(dir), "enabled after setEnabled(true)");

        const QString written = readAll(dir + QStringLiteral("/hitikz.desktop"));
        CHECK(written.contains(QStringLiteral("Exec=/usr/bin/hitikz --hidden")),
              "written entry starts hidden");

        CHECK(AutostartManager::setEnabled(false, QString(), dir),
              "setEnabled(false) succeeds");
        CHECK(!AutostartManager::isEnabled(dir), "disabled after setEnabled(false)");
        CHECK(AutostartManager::setEnabled(false, QString(), dir),
              "setEnabled(false) is idempotent");
    }

    // setEnabled with empty exec path falls back to the running binary
    {
        QTemporaryDir tmp;
        const QString dir = tmp.path();
        CHECK(AutostartManager::setEnabled(true, QString(), dir),
              "setEnabled(true) with default exec succeeds");
        const QString written = readAll(dir + QStringLiteral("/hitikz.desktop"));
        CHECK(written.contains(QCoreApplication::applicationFilePath()),
              "default exec is the running binary");
        CHECK(written.contains(QStringLiteral("--hidden")),
              "default exec entry still carries --hidden");
    }

    // --- migrateEntryToHidden -------------------------------------------------
    // Legacy entry (the exact shape users end up with when enabling autostart
    // from system settings): Exec without --hidden pops the window on login.
    {
        QTemporaryDir tmp;
        const QString path = tmp.path() + QStringLiteral("/hitikz.desktop");
        {
            QFile f(path);
            f.open(QIODevice::WriteOnly);
            f.write("[Desktop Entry]\n"
                    "Categories=Utility;\n"
                    "Exec=hitikz\n"
                    "Icon=hitikz\n"
                    "Name=hitikz\n"
                    "StartupWMClass=hitikz\n"
                    "Terminal=false\n"
                    "Type=Application\n");
        }

        CHECK(AutostartManager::migrateEntryToHidden(path),
              "legacy entry gets migrated");
        const QString migrated = readAll(path);
        CHECK(migrated.contains(QStringLiteral("\nExec=hitikz --hidden\n")),
              "migrated Exec line carries --hidden");
        CHECK(migrated.contains(QStringLiteral("StartupWMClass=hitikz")),
              "unrelated lines survive migration");
        CHECK(!AutostartManager::migrateEntryToHidden(path),
              "second migration is a no-op");
        CHECK(readAll(path) == migrated, "no-op migration leaves file unchanged");
    }

    // Entry already hidden → untouched
    {
        QTemporaryDir tmp;
        const QString path = tmp.path() + QStringLiteral("/hitikz.desktop");
        {
            QFile f(path);
            f.open(QIODevice::WriteOnly);
            f.write("[Desktop Entry]\nExec=/usr/bin/hitikz --hidden\n");
        }
        CHECK(!AutostartManager::migrateEntryToHidden(path),
              "entry already hidden is not rewritten");
    }

    // Missing file / file without Exec line
    {
        QTemporaryDir tmp;
        CHECK(!AutostartManager::migrateEntryToHidden(
                  tmp.path() + QStringLiteral("/nonexistent.desktop")),
              "missing file is not migrated");

        const QString path = tmp.path() + QStringLiteral("/broken.desktop");
        {
            QFile f(path);
            f.open(QIODevice::WriteOnly);
            f.write("[Desktop Entry]\nName=hitikz\n");
        }
        CHECK(!AutostartManager::migrateEntryToHidden(path),
              "file without Exec line is not rewritten");
        CHECK(readAll(path) == QStringLiteral("[Desktop Entry]\nName=hitikz\n"),
              "file without Exec line left byte-identical");
    }

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) failed!";
        return 1;
    }
    qDebug() << "\nAll autostart tests passed!";
    return 0;
}
