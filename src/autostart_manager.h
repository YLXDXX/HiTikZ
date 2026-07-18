#pragma once
#include <QString>

// Manages the XDG autostart entry (~/.config/autostart/hitikz.desktop).
//
// The entry launches the program with --hidden so that, after login, it goes
// straight to the system tray instead of popping up the main window.
// Free functions (namespace, no Qt object) so they are trivially unit-testable
// with an alternate directory.
namespace AutostartManager {

// ~/.config/autostart
QString autostartDirPath();

// ~/.config/autostart/hitikz.desktop
QString autostartFilePath();

// Full .desktop payload for the given executable path (always with --hidden).
QString desktopFileContents(const QString &execPath);

// True if the autostart entry exists. `autostartDir` overrides the default
// directory (used by tests).
bool isEnabled(const QString &autostartDir = QString());

// Creates (with --hidden) or removes the autostart entry. An empty `execPath`
// falls back to QCoreApplication::applicationFilePath().
bool setEnabled(bool enabled, const QString &execPath = QString(),
                const QString &autostartDir = QString());

// Upgrades a pre-existing entry whose Exec= line lacks --hidden (legacy
// entries popped the main window on every login). Returns true only when the
// file existed and was actually rewritten.
bool migrateEntryToHidden(const QString &desktopFilePath = QString());

} // namespace AutostartManager
