#pragma once

#include <QString>

namespace FileLauncher {

bool openInVSCode(const QString& filePath);
bool showInFileBrowser(const QString& filePath);

}
