/**
 * \file TerminalPlugin.cpp
 * \brief Implementation of the Terminal plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 */

#include <QDockWidget>
#include <QFontDatabase>
#include <QKeySequence>
#include <QSettings>

#include <KodoTerm/KodoTerm.hpp>
#include <qmdidialogevents.hpp>

#include "plugins/Terminal/TerminalPlugin.hpp"

static QString systemCurrentShell() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

#if defined(Q_OS_WIN)
    // Windows shell (usually cmd.exe or powershell)
    if (env.contains("ComSpec")) {
        return env.value("ComSpec");
    }
    return QString();
#else
    // Unix / Linux / macOS shell (bash, zsh, fish, etc.)
    if (env.contains("SHELL")) {
        return env.value("SHELL");
    }
    return QString();
#endif
}

TerminalPlugin::TerminalPlugin() {
    name = tr("Terminal support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    toggleTerminal = new QAction(tr("Toggle terminal"), this);
    toggleTerminal->setShortcut(QKeySequence("Ctrl+T"));
    connect(toggleTerminal, &QAction::triggered, this, [this]() {
        if (terminalDock->isVisible()) {
            terminalDock->hide();
        } else {
            terminalDock->show();
            terminalDock->raise();
            terminalDock->widget()->setFocus();
            console->setFocus();
        }
    });

    menus[tr("&File")]->addAction(toggleTerminal);

    theme = TerminalTheme::defaultTheme();
#ifdef Q_OS_WIN
    auto pseudoPrompt = QString("C:\\> ver<br>Microsoft Windows [Version 10.0.19045.4170]");
#else
    auto pseudoPrompt =
        QString("user@localhost:~$ uptime<br>12:34:56 up 10 days,  1:23,  2<br>users, "
                " load average: 0.05, 0.01, 0.00");
#endif

    auto colorsSpan = QString("<br>");
    for (auto i = 0; i < 16; i++) {
        auto c1 = theme.palette[i].name();
        auto c2 = theme.palette[16 - i].name();
        auto c3 = QString("<span style='background-color: %1; color: %2'>x</span>").arg(c1, c1);
        colorsSpan += c3;
    }
    auto monospacedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monospacedFont.setFixedPitch(true);

    config.pluginName = tr("Terminal");

    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Display font"))
                                     .setKey(Config::FontKey)
                                     .setType(qmdiConfigItem::Font)
                                     .setDefaultValue(monospacedFont)
                                     .setValue(monospacedFont)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Theme"))
                                     .setDescription(tr("Which theme to use for the terminal"))
                                     .setKey(Config::ThemeFileKey)
                                     .setType(qmdiConfigItem::Label)
                                     //.setDefaultValue("ctags-universal")
                                     //.setPossibleValue(true)
                                     .build());
    config.configItems.push_back(
        qmdiConfigItem::Builder()
            .setDisplayName(tr("Choose theme"))
            .setDescription(tr("Click to choose form internal themes avilable"))
            .setKey(Config::ThemeFileChooseKey)
            .setType(qmdiConfigItem::Button)
            .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(pseudoPrompt + colorsSpan)
                                     .setKey(Config::PromptPreviewKey)
                                     .setType(qmdiConfigItem::Label)
                                     .build());

    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Double click selects line"))
                                     .setKey(Config::DoubleClickKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Tripple click selects whole line"))
                                     .setKey(Config::TrippleClickClickKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Copy text on selection"))
                                     .setKey(Config::CopyOnSelectKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Paste on middle mouse click"))
                                     .setKey(Config::PasteOnMiddleClickKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());

    connect(&qmdiDialogEvents::instance(), &qmdiDialogEvents::buttonClicked, this,
            [this](qmdiConfigDialog *dialog, const QString &buttonKey) {
                if (buttonKey == Config::ThemeFileKey) {
                    // downloadCTags(dialog);
                }
            });
    connect(&qmdiDialogEvents::instance(), &qmdiDialogEvents::widgetCreated, this,
            [this, monospacedFont](auto dialog, auto const &item, auto label, auto widget) {
                qDebug() << "Created widget" << item.key;
                if (item.key == Config::PromptPreviewKey) {
                    label->setFrameStyle(QFrame::Panel);
                    label->setFont(monospacedFont);
                    auto pal = label->palette();
                    pal.setColor(QPalette::Window, this->theme.background);
                    pal.setColor(QPalette::WindowText, this->theme.foreground);
                    label->setAutoFillBackground(true);
                    label->setPalette(pal);
                }

                Q_UNUSED(dialog);
                Q_UNUSED(widget);
                // Q_UNUSED(this);
            });
}

TerminalPlugin::~TerminalPlugin() {
    // TODO
}

// IPlugin interface
void TerminalPlugin::on_client_merged(qmdiHost *host) {
    auto manager = dynamic_cast<PluginManager *>(host);
    console = new KodoTerm(manager);
    console->setProgram(systemCurrentShell());
    console->start();
    terminalDock = manager->createNewPanel(Panels::South, "terminalPanel", tr("Terminal"), console);
}

void TerminalPlugin::on_client_unmerged(qmdiHost *) { delete terminalDock; }

void TerminalPlugin::loadConfig(QSettings &settings) {
    // TODO
}
