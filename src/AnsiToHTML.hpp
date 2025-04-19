/**
 * \file AnsiToHTML.hpp
 * \brief Helper functions to convert ANSI codes to HTML - definitions
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPD-license: MIT

#pragma once

#include <QMap>

class QTextEdit;
class QString;

/// Returns true, if a string is binary, or plain text
auto isPlainText(const QString &str) -> bool;

// UNUSED: default theme for console forerground colors
auto defaultFgColorMap() -> const QMap<int, QString> &;
// UNUSED: default theme for console background colors
auto defaultBgColorMap() -> const QMap<int, QString> &;

/// Helper function, appends plain text to to a QTextEdit
auto appendAscii(QTextEdit *edit, const QString &plainText) -> void;

/// Helper function, converts ANSI escaped text to HTML and appends it to a QTextEdit
auto appendAnsiHtml(QTextEdit *edit, const QString &ansiText) -> void;

/// Convert an HTML string, linking all filenames inside "<a href>"
auto linkifyFileNames(const QString &html) -> QString;

/// Convert an HTML string, linking all URLS inside "<a href>"
auto linkifyUrls(const QString &html) -> QString;

/// Convert ANSI escaped string, to HTML
auto ansiToHtml(const QString &ansiText, bool linkifyFiles) -> QString;

/// Convert a text containing ANSI escaped code to raw text.
auto removeAnsiEscapeCodes(QString &input) -> QString;
