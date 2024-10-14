#include "thememanager.h"
#include <QDirIterator>

namespace Qutepart {

ThemeManager::ThemeManager() { empty = ThemeMetaData(); }

auto ThemeManager::clear() -> void { themes.clear(); }

auto ThemeManager::loadFromDir(const QString dirName) -> void {
    auto i = QDirIterator(dirName, {"*.theme"}, QDir::NoDotAndDotDot | QDir::Files,
                          QDirIterator::Subdirectories);

    while (i.hasNext()) {
        i.next();
        auto fname = i.fileInfo().absoluteFilePath();
        auto theme = Theme();
        if (theme.loadTheme(fname)) {
            themes.insert(fname, theme.getMetaData());
        }
    }
}

auto ThemeManager::getLoadedFiles() -> QStringList {
    auto s = themes.keys();
    return s;
}

auto ThemeManager::getThemeMetaData(const QString fileName) const -> ThemeMetaData {
    if (themes.contains(fileName)) {
        return themes[fileName];
    }
    return empty;
}

} // namespace Qutepart