#include "FileLauncher.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QUrl>

namespace {

QString findVSCodeCommand()
{
  const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  const QStringList candidates{
    QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
      .filePath(QStringLiteral("Programs/Microsoft VS Code/bin/code.cmd")),
    QDir(environment.value(QStringLiteral("ProgramFiles")))
      .filePath(QStringLiteral("Microsoft VS Code/bin/code.cmd")),
    QDir(environment.value(QStringLiteral("ProgramFiles(x86)")))
      .filePath(QStringLiteral("Microsoft VS Code/bin/code.cmd"))
  };

  for (const QString& candidate : candidates) {
    if (QFileInfo::exists(candidate))
      return candidate;
  }

  QString command = QStandardPaths::findExecutable(QStringLiteral("code.cmd"));
  if (command.isEmpty())
    command = QStandardPaths::findExecutable(QStringLiteral("code"));
  return command;
}

}

namespace FileLauncher {

bool openInVSCode(const QString& filePath)
{
  if (!QFileInfo::exists(filePath))
    return false;

  const QString commandScript = findVSCodeCommand();
  if (commandScript.isEmpty())
    return false;

  QDir installDirectory(QFileInfo(commandScript).absolutePath());
  installDirectory.cdUp();
  const QString executable = installDirectory.filePath(QStringLiteral("Code.exe"));

  QString cliScript = installDirectory.filePath(
    QStringLiteral("resources/app/out/cli.js"));
  if (!QFileInfo::exists(cliScript)) {
    const QFileInfoList versionDirectories = installDirectory.entryInfoList(
      QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    for (const QFileInfo& versionDirectory : versionDirectories) {
      const QString candidate = QDir(versionDirectory.absoluteFilePath())
        .filePath(QStringLiteral("resources/app/out/cli.js"));
      if (QFileInfo::exists(candidate)) {
        cliScript = candidate;
        break;
      }
    }
  }

  if (QFileInfo(executable).isExecutable() && QFileInfo::exists(cliScript)) {
    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("ELECTRON_RUN_AS_NODE"), QStringLiteral("1"));
    environment.insert(QStringLiteral("VSCODE_DEV"), QString());
    process.setProcessEnvironment(environment);
    process.setProgram(executable);
    process.setArguments(
      {cliScript, QStringLiteral("--reuse-window"),
       QStringLiteral("--goto"), QFileInfo(filePath).absoluteFilePath()});
    return process.startDetached();
  }

  if (QFileInfo(executable).isExecutable())
    return QProcess::startDetached(
      executable, {QFileInfo(filePath).absoluteFilePath()});

  return false;
}

bool showInFileBrowser(const QString& filePath)
{
  const QFileInfo fileInfo(filePath);
  if (!fileInfo.exists())
    return false;

#ifdef Q_OS_WIN
  return QProcess::startDetached(
    QStringLiteral("explorer.exe"),
    {QStringLiteral("/select,"), QDir::toNativeSeparators(fileInfo.absoluteFilePath())});
#else
  return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
#endif
}

}
