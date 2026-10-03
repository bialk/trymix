#include "FileHistory.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <utility>

namespace {

Qt::CaseSensitivity pathCaseSensitivity()
{
#ifdef Q_OS_WIN
  return Qt::CaseInsensitive;
#else
  return Qt::CaseSensitive;
#endif
}

}

FileHistory::FileHistory(
  QString settingsGroup, qsizetype maximumRecentFiles,
  QString currentFileKey, QString recentFilesKey)
  : m_settingsGroup(std::move(settingsGroup))
  , m_currentFileKey(std::move(currentFileKey))
  , m_recentFilesKey(std::move(recentFilesKey))
  , m_maximumRecentFiles(qMax<qsizetype>(0, maximumRecentFiles))
{}

QString FileHistory::currentFile() const
{
  QSettings settings;
  settings.beginGroup(m_settingsGroup);
  return settings.value(m_currentFileKey).toString();
}

void FileHistory::setCurrentFile(const QString& filePath) const
{
  if (filePath.isEmpty()) {
    clearCurrentFile();
    return;
  }

  QSettings settings;
  settings.beginGroup(m_settingsGroup);
  settings.setValue(m_currentFileKey, normalizedPath(filePath));
}

void FileHistory::clearCurrentFile() const
{
  QSettings settings;
  settings.beginGroup(m_settingsGroup);
  settings.remove(m_currentFileKey);
}

QStringList FileHistory::recentFiles() const
{
  QSettings settings;
  settings.beginGroup(m_settingsGroup);
  return settings.value(m_recentFilesKey).toStringList();
}

QStringList FileHistory::existingRecentFiles() const
{
  const QStringList storedFiles = recentFiles();
  QStringList existingFiles;
  for (const QString& filePath : storedFiles) {
    if (QFileInfo(filePath).isFile())
      existingFiles.append(filePath);
  }

  if (existingFiles != storedFiles)
    setRecentFiles(existingFiles);
  return existingFiles;
}

void FileHistory::rememberFile(const QString& filePath) const
{
  const QString path = normalizedPath(filePath);
  QStringList files = recentFiles();
  files.removeIf([&path](const QString& storedPath) {
    return QString::compare(storedPath, path, pathCaseSensitivity()) == 0;
  });
  files.prepend(path);
  if (files.size() > m_maximumRecentFiles)
    files.resize(m_maximumRecentFiles);

  setCurrentFile(path);
  setRecentFiles(files);
}

void FileHistory::removeFile(const QString& filePath) const
{
  QStringList files = recentFiles();
  files.removeIf([&filePath](const QString& storedPath) {
    return QString::compare(storedPath, filePath, pathCaseSensitivity()) == 0;
  });
  setRecentFiles(files);
}

QString FileHistory::normalizedPath(const QString& filePath) const
{
  return QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
}

void FileHistory::setRecentFiles(const QStringList& filePaths) const
{
  QSettings settings;
  settings.beginGroup(m_settingsGroup);
  settings.setValue(m_recentFilesKey, filePaths);
}
