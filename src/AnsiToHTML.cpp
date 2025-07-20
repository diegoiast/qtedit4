/**
 * \file AnsiToHTML.hpp
 * \brief Helper functions to convert ANSI codes to HTML - implementation
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPD-license: MIT

#include "AnsiToHTML.hpp"

#include <QRegularExpression>
#include <QString>
#include <QTextBlock>
#include <QTextEdit>
#include <QUrl>

auto isPlainText(const QString &str) -> bool {
    if (str.isEmpty()) {
        return true;
    }
    auto textCharCount = 0;
    auto totalCharCount = 0;
    for (const auto &ch : str) {
        totalCharCount++;
        if (ch == '\0') {
            return false;
        }
        if (ch.isLetterOrNumber() || ch.isPunct() || ch.isSpace() || ch.isSymbol() || ch.isMark()) {
            textCharCount++;
        } else if (ch.category() == QChar::Other_Control && ch.unicode() < 32) {
            if (ch == '\n' || ch == '\r' || ch == '\t') {
                textCharCount++;
            }
        }
    }
    auto textRatio = static_cast<double>(textCharCount) / totalCharCount;
    return textRatio > 0.90;
}

const QMap<int, QString> &defaultFgColorMap() {
    static const QMap<int, QString> fg = {
        {30, "#000000"}, {31, "#ff0000"}, {32, "#00ff00"}, {33, "#ffff00"},
        {34, "#0000ff"}, {35, "#ff00ff"}, {36, "#00ffff"}, {37, "#ffffff"},
        {90, "#888888"}, {91, "#ff5555"}, {92, "#55ff55"}, {93, "#ffff55"},
        {94, "#5555ff"}, {95, "#ff55ff"}, {96, "#55ffff"}, {97, "#ffffff"}};
    return fg;
}

const QMap<int, QString> &defaultBgColorMap() {
    static const QMap<int, QString> bg = {
        {40, "#000000"},  {41, "#ff0000"},  {42, "#00ff00"},  {43, "#ffff00"},
        {44, "#0000ff"},  {45, "#ff00ff"},  {46, "#00ffff"},  {47, "#ffffff"},
        {100, "#888888"}, {101, "#ff5555"}, {102, "#55ff55"}, {103, "#ffff55"},
        {104, "#5555ff"}, {105, "#ff55ff"}, {106, "#55ffff"}, {107, "#ffffff"}};
    return bg;
}

auto appendAscii(QTextEdit *edit, const QString &ansiText) -> void {
    edit->moveCursor(QTextCursor::End);
    edit->insertPlainText(ansiText);

    const auto blockCount = edit->document()->blockCount();
    const auto lastBlock = edit->document()->findBlockByNumber(blockCount - 1);
    if (lastBlock.isValid()) {
        QTextCursor cursor(lastBlock);
        cursor.movePosition(QTextCursor::StartOfBlock);
        edit->setTextCursor(cursor);
        // edit->centerCursor();
    }
}

void applyAnsiCodeToFormat(QTextCharFormat &fmt, const QString &codeStr) {
    const int code = codeStr.toInt();

    const auto &fgMap = defaultFgColorMap();
    const auto &bgMap = defaultBgColorMap();

    // Reset
    if (code == 0) {
        fmt = QTextCharFormat();
        return;
    }

    // Text style attributes
    switch (code) {
    case 1:
        fmt.setFontWeight(QFont::Bold);
        break;
    case 2:
        fmt.setFontWeight(QFont::Normal);
        break; // Faint fallback
    case 3:
        fmt.setFontItalic(true);
        break;
    case 4:
        fmt.setFontUnderline(true);
        break;
    case 9:
        fmt.setFontStrikeOut(true);
        break;

    case 22:
        fmt.setFontWeight(QFont::Normal);
        break;
    case 23:
        fmt.setFontItalic(false);
        break;
    case 24:
        fmt.setFontUnderline(false);
        break;
    case 29:
        fmt.setFontStrikeOut(false);
        break;
    }

    // Foreground color
    if (fgMap.contains(code)) {
        fmt.setForeground(QColor(fgMap.value(code)));
    }

    // Background color
    if (bgMap.contains(code)) {
        fmt.setBackground(QColor(bgMap.value(code)));
    }
}

void insertLinkifiedText(QTextCursor &cursor, const QString &text,
                         const QTextCharFormat &baseFormat, bool linkifyFiles) {
    static QRegularExpression urlRegex(R"((https?|file)://[^\s<>()]+)");
    static QRegularExpression pathRegex(
        R"((([A-Za-z]:[\\/][\w\-.+\\/ ]+|~?/?[\w\-.+/]+)\.[a-zA-Z0-9+_-]{1,8})(:\d+)?(:\d+)?(:)?)");

    auto lastPos = 0;
    auto view = QStringView{text};
    auto matchIter = urlRegex.globalMatch(text);
    if (linkifyFiles) {
        matchIter = pathRegex.globalMatch(text);
    }

    while (matchIter.hasNext()) {
        auto match = matchIter.next();
        auto start = match.capturedStart();
        auto end = match.capturedEnd();
        auto full = match.captured(0);

        if (start > lastPos) {
            cursor.insertText(view.sliced(lastPos, start - lastPos).toString(), baseFormat);
        }

        auto linkFmt = baseFormat;
        auto linkUrl = QUrl();

        if (match.captured(0).startsWith("http") || match.captured(0).startsWith("file")) {
            linkUrl = QUrl(full);
        } else {
            auto filePart = match.captured(1);
            auto line = match.captured(3).mid(1);
            auto column = match.captured(4).mid(1);
            QString fragment;
            if (!line.isEmpty()) {
                fragment += line;
            }
            if (!column.isEmpty()) {
                fragment += "," + column;
            }
            QString filePath = filePart;
            filePath.replace("\\", "/");
            linkUrl = QUrl::fromLocalFile(filePath);
            if (!fragment.isEmpty()) {
                linkUrl.setFragment(fragment);
            }
        }

        linkFmt.setAnchor(true);
        linkFmt.setAnchorHref(linkUrl.toString());
        linkFmt.setForeground(QBrush(Qt::blue));
        linkFmt.setFontUnderline(true);

        cursor.insertText(full, linkFmt);
        lastPos = end;
    }

    if (lastPos < text.length()) {
        cursor.insertText(view.sliced(lastPos).toString(), baseFormat);
    }
}

void appendAnsiHtml(QTextEdit *edit, const QString &ansiText) {
    auto cursor = edit->textCursor();
    cursor.movePosition(QTextCursor::End);

    static QRegularExpression ansiRegex("\x1b\\[([0-9;]*)m");
    static QRegularExpression removeRegex("\x1b\\[K");
    static QRegularExpression urlRegex(R"((https?|file)://[^\s<>()]+)");
    static QRegularExpression pathRegex(
        R"((([A-Za-z]:[\\/][\w\-.+\\/ ]+|~?/?[\w\-.+/]+)\.[a-zA-Z0-9+_-]{1,8})(:\d+)?(:\d+)?(:)?)");

    auto cleanedText = ansiText;
    cleanedText.remove(removeRegex);
    auto view = QStringView{cleanedText};

    auto matchIter = ansiRegex.globalMatch(cleanedText);
    auto lastPos = 0;
    auto fmt = QTextCharFormat{};
    auto codes = QStringList{};
    auto start = 0;
    auto linkifyFiles = true;

    while (matchIter.hasNext()) {
        auto match = matchIter.next();
        codes = match.captured(1).split(';');
        start = match.capturedStart();

        if (start > lastPos) {
            auto chunk = view.sliced(lastPos, start - lastPos).toString();
            insertLinkifiedText(cursor, chunk, fmt, linkifyFiles);
        }

        if (codes.isEmpty() || codes.contains('0')) {
            fmt = QTextCharFormat(); // Reset
        } else {
            for (const auto &code : std::as_const(codes)) {
                applyAnsiCodeToFormat(fmt, code);
            }
        }

        lastPos = match.capturedEnd();
    }

    if (lastPos < view.length()) {
        insertLinkifiedText(cursor, view.sliced(lastPos).toString(), fmt, linkifyFiles);
    }

    edit->setTextCursor(cursor);
    edit->ensureCursorVisible();
}

auto appendAnsiHtml2(QTextEdit *edit, const QString &ansiText) -> void {
    edit->moveCursor(QTextCursor::End);

#if 1
    auto html = ansiToHtml(ansiText, true);
    edit->insertHtml(ansiText);
#else
    edit->insertPlainText(ansiText);
#endif
    edit->moveCursor(QTextCursor::End);
    edit->ensureCursorVisible();
}

auto static isInsideAnchor(const QString &html, int pos) -> bool {
    auto open = html.lastIndexOf("<a ", pos, Qt::CaseInsensitive);
    auto close = html.lastIndexOf("</a>", pos, Qt::CaseInsensitive);
    return open != -1 && (close == -1 || open > close);
};

auto linkifyFileNames(const QString &html) -> QString {
    auto pathRegex = QRegularExpression(
        R"((([A-Za-z]:[\\/][\w\-.+\\/ ]+|~?/?[\w\-.+/]+)\.[a-zA-Z0-9+_-]{1,8})(:\d+)?(:\d+)?(:)?)");

    auto result = QString{};
    auto lastPos = 0;
    auto it = pathRegex.globalMatch(html);

    while (it.hasNext()) {
        auto match = it.next();
        auto start = match.capturedStart();
        auto end = match.capturedEnd();
        auto fullMatch = match.captured(0);
        auto filePart = match.captured(1);
        auto line = match.captured(3).mid(1);
        auto column = match.captured(4).mid(1);
        auto insideAnchor = isInsideAnchor(html, start);

        result += html.mid(lastPos, start - lastPos);
        if (insideAnchor) {
            result += fullMatch;
        } else {
            auto fragment = QString{};
            auto urlPath = filePart;
            // Convert backslashes to slashes for file:// URLs
            urlPath.replace("\\", "/");
            auto fileUrl = QUrl::fromLocalFile(urlPath);

            if (!line.isEmpty()) {
                fragment += line;
            }
            if (!column.isEmpty()) {
                if (!fragment.isEmpty()) {
                    fragment += ",";
                }
                fragment += column;
            }
            if (!fragment.isEmpty()) {
                fileUrl.setFragment(fragment);
            }
            auto link =
                QString("<a href=\"%1\">%2</a>").arg(fileUrl.toString(), fullMatch.toHtmlEscaped());
            result += link;
        }
        lastPos = end;
    }
    result += html.mid(lastPos);
    return result;
}

QString linkifyUrls(const QString &html) {
    static const QRegularExpression urlRegex(
        R"((([a-z][a-z0-9+\-.]*://[^\s<>"']+)|www\.[^\s<>"']+))",
        QRegularExpression::CaseInsensitiveOption);

    auto output = QString{};
    auto lastPos = 0;
    auto matchIter = urlRegex.globalMatch(html);
    while (matchIter.hasNext()) {
        auto match = matchIter.next();
        auto start = match.capturedStart();
        auto end = match.capturedEnd();

        if (isInsideAnchor(html, start)) {
            continue;
        }
        output += html.mid(lastPos, start - lastPos);
        auto url = match.captured(0);
        // Only prepend http:// if the url starts with www.
        auto href = url.startsWith("www.", Qt::CaseInsensitive) ? "http://" + url : url;
        output += "<a href=\"" + href.toHtmlEscaped() + "\">" + url.toHtmlEscaped() + "</a>";
        lastPos = end;
    }
    output += html.mid(lastPos);
    return output;
}

auto ansiToHtml(const QString &ansiText, bool linkifyFiles) -> QString {
    static auto ansiRegex = QRegularExpression("\x1b\\[([0-9;]*)m");
    static auto removeRegex = QRegularExpression("\x1b\\[K");

    auto htmlText = ansiText;
    htmlText.replace("&", "&amp;");
    htmlText.replace("<", "&lt;");
    htmlText.replace(">", "&gt;");
    htmlText.replace(" ", "&nbsp;");
    htmlText.replace("\n", "<br>\n");

    htmlText.remove(removeRegex);

    auto ansiToCss = QMap<QString, QString>{
        {"30", "black"},        {"31", "red"},        {"32", "green"},      {"33", "yellow"},
        {"34", "blue"},         {"35", "magenta"},    {"36", "cyan"},       {"37", "white"},
        {"90", "gray"},         {"91", "lightcoral"}, {"92", "lightgreen"}, {"93", "khaki"},
        {"94", "lightskyblue"}, {"95", "violet"},     {"96", "lightcyan"},  {"97", "whitesmoke"},
    };

    auto lastPos = 0;
    auto it = ansiRegex.globalMatch(htmlText);
    QStringList openTags;
    QString result;

    while (it.hasNext()) {
        auto match = it.next();
        auto codes = match.captured(1).split(';');
        auto isReset = (codes.isEmpty() || codes.contains("0"));

        result += htmlText.mid(lastPos, match.capturedStart() - lastPos);
        if (isReset) {
            while (!openTags.isEmpty()) {
                result += openTags.takeLast();
            }
        } else {
            auto style = QString{};
            for (auto const &code : std::as_const(codes)) {
                if (ansiToCss.contains(code)) {
                    style += QString("color:%1;").arg(ansiToCss.value(code));
                } else if (code == "1") {
                    style += "font-weight:bold;";
                }
                // Add more style handlers here if needed
            }
            if (!style.isEmpty()) {
                result += QString("<span style=\"%1\">").arg(style);
                openTags.append("</span>");
            }
        }

        lastPos = match.capturedEnd();
    }

    result += htmlText.mid(lastPos);
    while (!openTags.isEmpty()) {
        result += openTags.takeLast();
    }

    result = linkifyUrls(result);

    if (linkifyFiles) {
        result = linkifyFileNames(result);
    }

    return result + "</br>";
}

auto removeAnsiEscapeCodes(const QString &input) -> QString {
    static const QRegularExpression ansiRegex(
        R"((\x1B\[[0-9;?]*[ -/]*[@-~])|(\x1B\]8;[^\x07\x1B]*[\x07\x1B\\]))");
    auto result = input;
    return result.replace(ansiRegex, "");
}
