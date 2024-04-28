/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef HIGHLIGHTERSETTINGS_H
#define HIGHLIGHTERSETTINGS_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QRegExp>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace TextEditor {

class HighlighterSettings
{
public:
    HighlighterSettings();

    void toSettings(const QString &category, QSettings *s) const;
    void fromSettings(const QString &category, QSettings *s);

    void setDefinitionFilesPath(const QString &path) { m_definitionFilesPath = path; }
    const QString &definitionFilesPath() const { return m_definitionFilesPath; }

    void setFallbackDefinitionFilesPath(const QString &path){ m_fallbackDefinitionFilesPath = path;}
    const QString &fallbackDefinitionFilesPath() const { return m_fallbackDefinitionFilesPath; }

    void setAlertWhenNoDefinition(const bool alert) { m_alertWhenNoDefinition = alert; }
    bool alertWhenNoDefinition() const { return m_alertWhenNoDefinition; }

    void setUseFallbackLocation(const bool use) { m_useFallbackLocation = use; }
    bool useFallbackLocation() const { return m_useFallbackLocation; }

    void setIgnoredFilesPatterns(const QString &patterns);
    QString ignoredFilesPatterns() const;
    bool isIgnoredFilePattern(const QString &fileName) const;

    bool equals(const HighlighterSettings &highlighterSettings) const;

private:
    void assignDefaultIgnoredPatterns();
    void assignDefaultDefinitionsPath();

    void setExpressionsFromList(const QStringList &patterns);
    QStringList listFromExpressions() const;

    bool m_alertWhenNoDefinition;
    bool m_useFallbackLocation;
    QString m_definitionFilesPath;
    QString m_fallbackDefinitionFilesPath;
    QList<QRegExp> m_ignoredFiles;
};

inline bool operator==(const HighlighterSettings &a, const HighlighterSettings &b)
{ return a.equals(b); }

inline bool operator!=(const HighlighterSettings &a, const HighlighterSettings &b)
{ return !a.equals(b); }

namespace Internal {
QString findFallbackDefinitionsLocation();
}

} // namespace TextEditor

#endif // HIGHLIGHTERSETTINGS_H
