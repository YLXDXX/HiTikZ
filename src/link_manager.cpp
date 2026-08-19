#include "link_manager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>

QString LinkManager::defaultDir()
{
    return QStringLiteral("~/PicTikZ");
}

QString LinkManager::settingDir()
{
    QSettings settings(QStringLiteral("HiTikZ"), QStringLiteral("TikzManager"));
    QString dir = settings.value(QStringLiteral("link/dir"), defaultDir()).toString().trimmed();
    if (dir.isEmpty())
        dir = defaultDir();
    return dir;
}

LinkManager::LinkManager(const QString &configuredDir, QObject *parent)
    : QObject(parent)
{
    m_configuredDir = configuredDir.trimmed();
    if (m_configuredDir.isEmpty())
        m_configuredDir = defaultDir();
    while (m_configuredDir.size() > 1 && m_configuredDir.endsWith(QLatin1Char('/')))
        m_configuredDir.chop(1);
}

QString LinkManager::configuredDir() const
{
    return m_configuredDir;
}

QString LinkManager::dirPath() const
{
    QString dir = m_configuredDir;
    if (dir == QLatin1Char('~') || dir.startsWith(QStringLiteral("~/")))
        dir = QDir::homePath() + dir.mid(1);
    while (dir.size() > 1 && dir.endsWith(QLatin1Char('/')))
        dir.chop(1);
    return dir;
}

QString LinkManager::includeCommand(const QString &linkName) const
{
    return QStringLiteral("\\includegraphics{%1/%2}").arg(m_configuredDir, linkName);
}

QString LinkManager::linkFilePath(const QString &linkName) const
{
    return dirPath() + QLatin1Char('/') + linkName;
}

bool LinkManager::ensureDirExists() const
{
    return QDir().mkpath(dirPath());
}

bool LinkManager::linkEntryExists(const QString &linkName) const
{
    if (linkName.isEmpty())
        return false;
    QDir dir(dirPath());
    if (!dir.exists())
        return false;
    // Match the scan used by nextFreeLinkName(): plain files and symlinks
    // (including dangling ones) count as occupying their name.
    return dir.entryList(QDir::Files | QDir::System).contains(linkName);
}

bool LinkManager::linkTargetExists(const QString &linkName) const
{
    if (linkName.isEmpty())
        return false;
    return QFile::exists(linkFilePath(linkName));
}

QString LinkManager::nextFreeLinkName() const
{
    QDir dir(dirPath());
    QSet<int> used;
    if (dir.exists()) {
        const QRegularExpression re(QStringLiteral("^(\\d+)\\.pdf$"));
        // QDir::Files alone may skip dangling symlinks; QDir::System keeps
        // them so a broken link still occupies its sequence number.
        const QStringList entries = dir.entryList(QDir::Files | QDir::System);
        for (const QString &entry : entries) {
            const QRegularExpressionMatch match = re.match(entry);
            if (match.hasMatch())
                used.insert(match.captured(1).toInt());
        }
    }
    int number = 1;
    while (used.contains(number))
        ++number;
    return QStringLiteral("%1.pdf").arg(number, 4, 10, QLatin1Char('0'));
}

bool LinkManager::createLink(const QString &pdfPath, const QString &linkName) const
{
    if (pdfPath.isEmpty() || linkName.isEmpty())
        return false;

    const QFileInfo srcInfo(pdfPath);
    if (!srcInfo.exists() || !srcInfo.isFile())
        return false;

    if (!ensureDirExists())
        return false;

    const QString dst = linkFilePath(linkName);

    // Replace whatever entry currently occupies the name (regular file,
    // valid symlink or dangling symlink).
    const QFileInfo dstInfo(dst);
    if (dstInfo.exists() || dstInfo.isSymLink())
        QFile::remove(dst);

    if (QFile::link(srcInfo.absoluteFilePath(), dst))
        return true;

    // Fall back to a plain copy on filesystems without symlink support.
    return QFile::copy(srcInfo.absoluteFilePath(), dst);
}

bool LinkManager::removeLink(const QString &linkName) const
{
    if (linkName.isEmpty())
        return true;

    const QString path = linkFilePath(linkName);
    const QFileInfo info(path);
    if (!info.exists() && !info.isSymLink())
        return true;
    return QFile::remove(path);
}
