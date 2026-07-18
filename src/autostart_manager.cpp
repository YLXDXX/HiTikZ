#include "autostart_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

namespace AutostartManager {

static QString resolveDir(const QString &dir)
{
    if (!dir.isEmpty())
        return dir;
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/autostart");
}

QString autostartDirPath()
{
    return resolveDir(QString());
}

QString autostartFilePath()
{
    return autostartDirPath() + QStringLiteral("/hitikz.desktop");
}

QString desktopFileContents(const QString &execPath)
{
    QString exec = execPath;
    if (exec.contains(QLatin1Char(' ')))
        exec = QLatin1Char('"') + exec + QLatin1Char('"');
    return QStringLiteral(
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=HiTikZ\n"
        "Comment=TikZ 代码合集管理器\n"
        "Exec=%1 --hidden\n"
        "Icon=hitikz\n"
        "Terminal=false\n"
        "StartupWMClass=hitikz\n"
        "Categories=Utility;\n"
        "X-GNOME-Autostart-enabled=true\n"
        "X-KDE-autostart-after=panel\n").arg(exec);
}

bool isEnabled(const QString &autostartDir)
{
    return QFile::exists(resolveDir(autostartDir) + QStringLiteral("/hitikz.desktop"));
}

bool setEnabled(bool enabled, const QString &execPath, const QString &autostartDir)
{
    const QString dir = resolveDir(autostartDir);
    const QString filePath = dir + QStringLiteral("/hitikz.desktop");

    if (!enabled) {
        if (!QFile::exists(filePath))
            return true;
        return QFile::remove(filePath);
    }

    if (!QDir().mkpath(dir))
        return false;

    QString exec = execPath;
    if (exec.isEmpty())
        exec = QCoreApplication::applicationFilePath();

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    file.write(desktopFileContents(exec).toUtf8());
    return file.commit();
}

bool migrateEntryToHidden(const QString &desktopFilePath)
{
    const QString path = desktopFilePath.isEmpty() ? autostartFilePath() : desktopFilePath;
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return false;
    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    QStringList lines = content.split(QLatin1Char('\n'));
    bool changed = false;
    for (QString &line : lines) {
        if (!line.startsWith(QStringLiteral("Exec=")))
            continue;
        if (line.contains(QStringLiteral("--hidden"))
            || line.contains(QStringLiteral("--minimized")))
            continue;
        line = line.trimmed() + QStringLiteral(" --hidden");
        changed = true;
    }
    if (!changed)
        return false;

    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    out.write(lines.join(QLatin1Char('\n')).toUtf8());
    return out.commit();
}

} // namespace AutostartManager
