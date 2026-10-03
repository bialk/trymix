#pragma once

#include <QString>
#include <QStringList>

class FileHistory
{
public:
  explicit FileHistory(
    QString settingsGroup, qsizetype maximumRecentFiles = 10,
    QString currentFileKey = QStringLiteral("currentFile"),
    QString recentFilesKey = QStringLiteral("recentFiles"));

  QString currentFile() const;
  void setCurrentFile(const QString& filePath) const;
  void clearCurrentFile() const;

  QStringList recentFiles() const;
  QStringList existingRecentFiles() const;
  void rememberFile(const QString& filePath) const;
  void removeFile(const QString& filePath) const;

private:
  QString normalizedPath(const QString& filePath) const;
  void setRecentFiles(const QStringList& filePaths) const;

  QString m_settingsGroup;
  QString m_currentFileKey;
  QString m_recentFilesKey;
  qsizetype m_maximumRecentFiles;
};
