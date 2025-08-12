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

/// Helper function, appends plain text to to a QTextEdit
auto appendAscii(QTextEdit *edit, const QString &plainText) -> void;

/// Helper function, converts ANSI escaped text to HTML and appends it to a QTextEdit
auto appendAnsiText(QTextEdit *edit, const QString &ansiText, const QString &baseDir) -> void;

/// Convert a text containing ANSI escaped code to raw text.
auto removeAnsiEscapeCodes(const QString &input) -> QString;
