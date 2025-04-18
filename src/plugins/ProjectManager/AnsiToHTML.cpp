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

auto appendAnsiHtml(QTextEdit *edit, const QString &ansiText) -> void {
    auto html = ansiToHtml(ansiText, true);
    edit->moveCursor(QTextCursor::End);
    // edit->insertHtml(html + "<br>");
    edit->insertHtml(html);

    const auto blockCount = edit->document()->blockCount();
    const auto lastBlock = edit->document()->findBlockByNumber(blockCount - 1);
    if (lastBlock.isValid()) {
        QTextCursor cursor(lastBlock);
        cursor.movePosition(QTextCursor::StartOfBlock);
        edit->setTextCursor(cursor);
        // edit->centerCursor();
    }
}

auto linkifyFileNames(const QString &html) -> QString {
    auto pathRegex = QRegularExpression(
        R"((([A-Za-z]:[\\/][\w\-.+\\/ ]+|~?/?[\w\-.+/]+)\.[a-zA-Z0-9+_-]{1,8})(:\d+)?(:\d+)?(:)?)");

    QString result;
    int lastPos = 0;
    auto it = pathRegex.globalMatch(html);

    while (it.hasNext()) {
        auto match = it.next();
        auto start = match.capturedStart();
        auto end = match.capturedEnd();
        auto fullMatch = match.captured(0);
        auto filePart = match.captured(1);
        auto line = match.captured(3).mid(1);
        auto column = match.captured(4).mid(1);

        // Check if inside an anchor tag
        bool insideAnchor = false;
        int anchorOpen = html.lastIndexOf("<a ", start, Qt::CaseInsensitive);
        int anchorClose = html.lastIndexOf("</a>", start, Qt::CaseInsensitive);
        if (anchorOpen != -1 && (anchorClose == -1 || anchorOpen > anchorClose)) {
            insideAnchor = true;
        }

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

auto ansiToHtml(const QString &ansiText, bool linkifyFiles) -> QString {
    auto htmlText = ansiText;
    htmlText.replace("&", "&amp;");
    htmlText.replace("<", "&lt;");
    htmlText.replace(">", "&gt;");
    htmlText.replace(" ", "&nbsp;");
    htmlText.replace("\n", "<br>\n");

    // Remove "erase to end of line" codes
    htmlText.remove(QRegularExpression("\x1b\\[K"));

    // ANSI color code map
    auto ansiToCss = QMap<QString, QString>{
        {"30", "black"},        {"31", "red"},        {"32", "green"},      {"33", "yellow"},
        {"34", "blue"},         {"35", "magenta"},    {"36", "cyan"},       {"37", "white"},
        {"90", "gray"},         {"91", "lightcoral"}, {"92", "lightgreen"}, {"93", "khaki"},
        {"94", "lightskyblue"}, {"95", "violet"},     {"96", "lightcyan"},  {"97", "whitesmoke"},
    };

    auto ansiRegex = QRegularExpression("\x1b\\[([0-9;]*)m");
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
            for (auto const &code : codes) {
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

    if (linkifyFiles) {
        result = linkifyFileNames(result);
    }

    return result + "</br>";
}

auto removeAnsiEscapeCodes(QString &input) -> QString {
    static const QRegularExpression ansiRegex(R"(\x1B\[[0-9;]*[A-Za-z])");
    return input.replace(ansiRegex, "");
}
