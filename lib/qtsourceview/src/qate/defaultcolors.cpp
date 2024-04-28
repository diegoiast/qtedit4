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

#include "qate/defaultcolors.h"

#include <QtCore/Qt>
#include "highlighter.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPlainTextEdit>

using namespace Qate;

Theme::Theme()
{
//	setupDefaultColors();
}

void Theme::setupDefaultColors()
{
	mDefault = QTextCharFormat();
	mDefault.setForeground(Qt::black);

	mFormats[FormatKeyword].setForeground(Qt::black);
	mFormats[FormatKeyword].setFontWeight(75);
	mFormats[FormatKeyword].setFontItalic(true);

	mFormats[FormatDataType].setForeground(Qt::blue);

	mFormats[FormatDecimal].setForeground(Qt::darkYellow);

	mFormats[FormatBaseN].setForeground(Qt::darkYellow);

	mFormats[FormatFloat].setForeground(Qt::darkYellow);

	mFormats[FormatChar].setForeground(Qt::red);

	mFormats[FormatString].setForeground(Qt::red);

	mFormats[FormatComment].setForeground(Qt::gray); // #606060

	mFormats[FormatAlert].setForeground(Qt::red);
	mFormats[FormatAlert].setFontUnderline(true);
	mFormats[FormatAlert].setUnderlineStyle(QTextCharFormat::WaveUnderline);

	mFormats[FormatError].setForeground(Qt::red);
	mFormats[FormatError].setFontUnderline(true);
	mFormats[FormatError].setUnderlineStyle(QTextCharFormat::DashUnderline);

	mFormats[FormatFunction].setForeground(Qt::cyan);

	mFormats[FormatRegionMarker].setForeground(Qt::green);

	mFormats[FormatOthers].setForeground(Qt::darkBlue);
}

Theme &Theme::defaultColors()
{
	static Theme sDefaultColors;
	return sDefaultColors;
}

/*
QString Theme::name(const QTextCharFormat &format) const
{
    if (format == QTextCharFormat())
        return "Default format";
    else if (format == m_keywordFormat)
        return "Keyword";
    else if (format == m_dataTypeFormat)
        return "Data type format";
    else if (format == m_decimalFormat)
        return "Decimal format";
    else if (format == m_baseNFormat)
        return "Base N format";
    else if (format == m_floatFormat)
        return "Float format";
    else if (format == m_charFormat)
        return "Char format";
    else if (format == m_stringFormat)
        return "String format";
    else if (format == m_commentFormat)
        return "Comment format";
    else if (format == m_alertFormat)
        return "Alert format";
    else if (format == m_errorFormat)
        return "Error format";
    else if (format == m_functionFormat)
        return "Function format";
    else if (format == m_regionMarkerFormat)
        return "Region Marker format";
    else if (format == m_othersFormat)
        return "Others format";
    else
        return "Unidentified format";
}
*/

using namespace TextEditor::Internal;

QTextCharFormat textCharFormatFromJson(QJsonObject obj) {
	QTextCharFormat tcf;

	QJsonValue v;

	v = obj.value("text-color");
	if (!v.isNull()) {
		QColor color(v.toString());
		tcf.setForeground(color);
	}
	v = obj.value("selected-text-color");
	if (!v.isNull()) {
		QColor color(v.toString());
//		TODO?
//		tcf.setForeground(color);
	}
	v = obj.value("background-color");
	if (!v.isNull() && !v.toString().isEmpty()) {
		QColor color(v.toString());
		tcf.setBackground(color);
	}
	v = obj.value("bold");
	if (!v.isNull()) {
		if (v.toBool()) {
			tcf.setFontWeight(QFont::Bold);
		} else {
			tcf.setFontWeight(QFont::Normal);
		}
	}
	v = obj.value("italic");
	if (!v.isNull()) {
		tcf.setFontItalic(v.toBool());
	}
	v = obj.value("underline");
	if (!v.isNull()) {
		tcf.setFontUnderline(v.toBool());
	}
	v = obj.value("strike-through");
	if (!v.isNull()) {
		tcf.setFontStrikeOut(v.toBool());
	}


	return  tcf;
}


void Theme::load(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		 return;
	QByteArray data = file.readAll();
	QJsonDocument doc = QJsonDocument::fromJson(data);
	QJsonObject mainObj = doc.object();

	loadTextStyles(mainObj["text-styles"].toObject());
	loadTextColors(mainObj["editor-colors"].toObject());
}

void Theme::loadTextStyles(const QJsonObject &textStyles)
{
	mFormats.clear();
	for (QString s: textStyles.keys()) {
		mFormats[s] = textCharFormatFromJson(textStyles[s].toObject());
	}
	// TODO - should we assert if this is not available?
	if (mFormats.contains(FormatNormal)) {
		mDefault = mFormats[FormatNormal];
	}
}

void Theme::loadTextColors(const QJsonObject &editorColors)
{
	mEditorColors.clear();
	for (QString s: editorColors.keys()) {
		QString colorRGBA = editorColors[s].toString();
		mEditorColors[s] = QColor(colorRGBA);
	}
}

void Theme::applyToHighlighter(TextEditor::Internal::Highlighter *hl) const
{
//	hl->configureFormat(Highlighter::Normal,           m_othersFormat       );
//	hl->configureFormat(Highlighter::VisualWhitespace, m_othersFormat       );

	hl->configureFormat(Highlighter::Keyword,          mFormats.value(FormatKeyword, mDefault));
	hl->configureFormat(Highlighter::DataType,         mFormats.value(FormatDataType, mDefault));
	hl->configureFormat(Highlighter::Decimal,          mFormats.value(FormatDecimal, mDefault));
	hl->configureFormat(Highlighter::BaseN,            mFormats.value(FormatBaseN, mDefault));
	hl->configureFormat(Highlighter::Float,            mFormats.value(FormatFloat, mDefault));
	hl->configureFormat(Highlighter::Char,             mFormats.value(FormatChar, mDefault));
	hl->configureFormat(Highlighter::String,           mFormats.value(FormatString, mDefault));
	hl->configureFormat(Highlighter::Comment,          mFormats.value(FormatComment, mDefault));
	hl->configureFormat(Highlighter::Alert,            mFormats.value(FormatAlert, mDefault));
	hl->configureFormat(Highlighter::Error,            mFormats.value(FormatError, mDefault));
	hl->configureFormat(Highlighter::Function,         mFormats.value(FormatFunction, mDefault));
	hl->configureFormat(Highlighter::RegionMarker,     mFormats.value(FormatRegionMarker, mDefault));
	hl->configureFormat(Highlighter::Others,           mFormats.value(FormatOthers, mDefault));
}

void Theme::applyToEditor(QPlainTextEdit *editor) const
{
	QPalette p( editor->palette() );
	if (mEditorColors.contains(EditorBackgroundColor)) {
		p.setColor( QPalette::Base, mEditorColors[EditorBackgroundColor] );
		p.setColor( QPalette::Text, mDefault.foreground().color() );
		editor->setPalette(p);
	}
	editor->setCurrentCharFormat(mDefault);
}
