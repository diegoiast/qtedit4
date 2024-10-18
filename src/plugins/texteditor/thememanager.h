#pragma once

#include "qutepart/theme.h"
#include <QHash>

namespace Qutepart {

class ThemeManager {
  public:
    ThemeManager();
    auto clear() -> void;
    auto loadFromDir(const QString dirName) -> void;
    auto getLoadedFiles() -> QStringList;
    auto getThemeMetaData(const QString fileName) const -> ThemeMetaData;
    auto getNameFromDesc(const QString description) const -> QString;

  private:
    ThemeMetaData empty;
    QHash<QString, ThemeMetaData> themes;
};

} // namespace Qutepart