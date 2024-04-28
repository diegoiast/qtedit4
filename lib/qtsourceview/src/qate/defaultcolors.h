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

#ifndef QATE_DEFAULT_COLORS_H
#define QATE_DEFAULT_COLORS_H

#include <QTextCharFormat>
#include <QMap>

namespace TextEditor{
	namespace Internal {
		class Highlighter;
	}
}

class QTextEdit;
class QPlainTextEdit;

namespace Qate {

static constexpr const char* FormatNormal = "Normal";
static constexpr const char* FormatKeyword = "Keyword";
static constexpr const char* FormatDataType = "DataType";
static constexpr const char* FormatDecimal = "DecVal";
static constexpr const char* FormatBaseN = "BaseN";
static constexpr const char* FormatFloat = "Float";
static constexpr const char* FormatChar = "Char";
static constexpr const char* FormatString = "String";
static constexpr const char* FormatComment = "Comment";
static constexpr const char* FormatAlert = "Alert";
static constexpr const char* FormatError = "Error";
static constexpr const char* FormatFunction = "Function";
static constexpr const char* FormatRegionMarker = "RegionMarker";
static constexpr const char* FormatOthers = "Others";

static constexpr const char* EditorBackgroundColor  = "background-color";
static constexpr const char* EditorCodeFolding  = "code-folding";
static constexpr const char* EditorBracketMatching  = "bracket-matching";
static constexpr const char* EditorCurrentLine  = "current-line";
static constexpr const char* EditorIconBorder  = "icon-border";
static constexpr const char* EditorIndentationLine  = "indentation-line";
static constexpr const char* EditorLineNumbers  = "line-numbers";
static constexpr const char* EditorCurrentLineNumber  = "current-line-number";
static constexpr const char* EditorMarkBookmark  = "mark-bookmark";
static constexpr const char* EditorMarkBreakpointActive  = "mark-breakpoint-active";
static constexpr const char* EditorMarkBreakpointReached  = "mark-breakpoint-reached";
static constexpr const char* EditorMarkBreakpointDisabled  = "mark-breakpoint-disabled";
static constexpr const char* EditorMarkExecution  = "mark-execution";
static constexpr const char* EditorMarkWarning  = "mark-warning";
static constexpr const char* EditorMarkError  = "mark-error";
static constexpr const char* EditorModifiedLines  = "modified-lines";
static constexpr const char* EditorreplaceHighlight  = "replace-highlight";
static constexpr const char* EditorSsavedLines  = "saved-lines";
static constexpr const char* EditorSearchHighlight  = "search-highlight";
static constexpr const char* EditorSelection  = "selection";
static constexpr const char* EditorSeparator  = "separator";
static constexpr const char* EditorSpellChecking  = "spell-checking";
static constexpr const char* EditorTabMarker  = "tab-marker";
static constexpr const char* EditorTemplateBackground  = "template-background";
static constexpr const char* EditorTemplatePlaceholder  = "template-placeholder";
static constexpr const char* EditorTemplateFocusedPlaceholder  = "template-focused-placeholder";
static constexpr const char* EditorTemplateReadOnlyPlaceholder  = "template-read-only-placeholder";
static constexpr const char* EditorWordWrapMarker = "word-wrap-marker";

class Theme
{
private:
	QMap<QString, QTextCharFormat> mFormats;
	QMap<QString, QColor> mEditorColors;
	QTextCharFormat mDefault;

public:
    static Theme &defaultColors();

    void load(const QString &fileName);
    void loadTextStyles(const QJsonObject &textStyles);
    void loadTextColors(const QJsonObject &editorColors);
    void applyToHighlighter(TextEditor::Internal::Highlighter *hl) const;
    void applyToEditor(QPlainTextEdit* editor) const;

    inline QColor getEditorColor(const char* name) const { return  mEditorColors.value(name); }
    inline QTextCharFormat getFormat( const char* name) const { return  mFormats.value(name, mDefault); }

    Theme();
    void setupDefaultColors();
};

}

#endif // QATE_DEFAULT_COLORS_H
