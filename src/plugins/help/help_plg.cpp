/**
 * \file help_plg.cpp
 * \brief Implementation of the help system plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see HelpPlugin
 */

#include "help_plg.h"
#include "iplugin.h"
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QSimpleUpdater.h>
#include <QUrl>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "CommandPaletteWidget/commandpalette.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(__linux__)
#define TESTING_CHANNEL "linux-testing"
#elif defined(_WIN32)
#define TESTING_CHANNEL "windows-testing"
#elif defined(__APPLE__)
#define TESTING_CHANNEL "osx-testing"
#else
#endif

// #define DEBUG_UPDATES

auto static createDesktopMenuItem(const std::string &execPath, const std::string &svgIconContent)
    -> std::string {
    const char *homeDir = std::getenv("HOME");
    if (!homeDir) {
        std::cerr << "Unable to get HOME directory" << std::endl;
        return {};
    }

    std::filesystem::path homePath(homeDir);
    std::filesystem::path iconFile = homePath / ".local/share/icons/qtedit4.svg";
    std::filesystem::path desktopFile = homePath / ".local/share/applications/qtedit4.desktop";
    std::filesystem::create_directories(iconFile.parent_path());
    std::filesystem::create_directories(desktopFile.parent_path());

    std::ofstream iconStream(iconFile);
    if (!iconStream.is_open()) {
        std::cerr << "Unable to create icon file: " << iconFile << std::endl;
        return {};
    }
    iconStream << svgIconContent;
    iconStream.close();

    std::ofstream file(desktopFile);
    if (!file.is_open()) {
        std::cerr << "Unable to create desktop file: " << desktopFile << std::endl;
        return {};
    }

    file << "[Desktop Entry]\n"
         << "Type=Application\n"
         << "Name=qtedit4\n"
         << "Comment=qtedit4 Text Editor\n"
         << "Exec=" << execPath << "\n"
         << "Icon=" << iconFile.string() << "\n"
         << "Categories=Utility;TextEditor;\n"
         << "Terminal=false\n";

    file.close();
    return desktopFile.string();
}

auto static getExecutablePath() -> std::string {
    std::string path;

#if defined(__unix__)
    const char *appImagePath = std::getenv("APPIMAGE");
    if (appImagePath) {
        return std::string(appImagePath);
    }

    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        path = std::string(result, (count > 0) ? count : 0);
    }
    if (path.empty()) {
        path = std::filesystem::absolute(std::filesystem::path(program_invocation_name)).string();
    }
#elif _WIN32
    char result[MAX_PATH];
    GetModuleFileNameA(NULL, result, MAX_PATH);
    path = std::string(result);
#endif

    return path;
}

auto static canInstallDesktopFile() -> bool {
#ifdef _WIN32
    // On Windows, always return false
    return false;
#else
    if (std::getenv("FLATPAK_ID") != nullptr) {
        return false;
    }

    auto exePath = getExecutablePath();
    auto path = std::filesystem::path(exePath);
    auto restrictedDirs = {"/usr/bin", "/opt", "/usr/local"};
    for (const auto &dir : restrictedDirs) {
        if (std::filesystem::path(exePath).string().find(dir) == 0) {
            return false;
        }
    }
    return true;
#endif
}

auto static refreshSystemMenus() -> void {
#if defined(__unix__)
    std::string desktopEnv =
        std::getenv("XDG_CURRENT_DESKTOP") ? std::getenv("XDG_CURRENT_DESKTOP") : "";

    if (desktopEnv.find("GNOME") != std::string::npos) {
        // std::system("killall gnome-panel || true");
        std::system("xdotool key F5");
    } else if (desktopEnv.find("KDE") != std::string::npos) {
        std::system("kbuildsycoca5");
    } else if (desktopEnv.find("XFCE") != std::string::npos) {
        std::system("xfce4-panel -r");
    } else {
        // Generic approach for other environments
        std::system("update-menus");
    }
#endif
}

HelpPlugin::HelpPlugin() {
    name = tr("Help system browser");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    auto updateChannelStrings = QStringList() << tr("Do not check for updates")
                                              << tr("Check for updates every time program starts")
                                              << tr("Check for updates once per day")
                                              << tr("Check for updates once per week");
    auto stableChannelStrings = QStringList() << tr("Stable channel (recommended")
                                              << tr("Testing channel (pre-releases)");

    config.pluginName = tr("Global config");
    config.configItems.push_back(
        qmdiConfigItem::Builder()
            .setDisplayName(tr("Check for updates"))
            .setDescription(tr("When to check for updates for this program"))
            .setKey(Config::UpdatesChecksKey)
            .setType(qmdiConfigItem::OneOf)
            .setPossibleValue(updateChannelStrings)
            .setDefaultValue(UpdateCheck::Daily)
            .build());
    config.configItems.push_back(
        qmdiConfigItem::Builder()
            .setDisplayName(tr("Update channel"))
            .setDescription(tr("Keep at stable, unless you want to report bugs"))
            .setKey(Config::UpdatesChannelKey)
            .setType(qmdiConfigItem::OneOf)
            .setPossibleValue(stableChannelStrings)
            .setDefaultValue(UpdateChannels::Stable)
            .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::LastUpdateTimeKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("0")
                                     .setUserEditable(false)
                                     .build());

    auto actionAbout = new QAction(tr("&About"), this);
    connect(actionAbout, &QAction::triggered, this, &HelpPlugin::actionAbout_triggered);
    auto actionCheckForUpdates = new QAction(tr("&Check for updates"), this);
    connect(actionCheckForUpdates, &QAction::triggered, this,
            &HelpPlugin::checkForUpdates_triggered);

    auto actionVisitHomePage = new QAction(tr("Visit homepage"), this);
    connect(actionVisitHomePage, &QAction::triggered, this,
            []() { QDesktopServices::openUrl(QUrl("https://github.com/diegoiast/qtedit4/")); });
    auto actionAboutQt = new QAction(tr("About Qt"), this);
    connect(actionAboutQt, &QAction::triggered, this, []() { QApplication::aboutQt(); });

    if (canInstallDesktopFile()) {
        auto installDesktopFile = new QAction(tr("Install desktop file"), this);
        connect(installDesktopFile, &QAction::triggered, this, [this]() {
            auto svgResourcePath = ":qtedit4.svg";
            auto exe = getExecutablePath();
            auto svgFile = QFile(QString::fromStdString(svgResourcePath));
            if (!svgFile.open(QIODevice::ReadOnly)) {
                std::cerr << "Unable to open SVG resource: " << svgResourcePath << std::endl;
                return;
            }
            auto svgContent = svgFile.readAll().toStdString();
            svgFile.close();

            auto desktopFile = createDesktopMenuItem(exe, svgContent);
            if (!desktopFile.empty()) {
                this->getManager()->openFile(QString::fromStdString(desktopFile));
            }
            refreshSystemMenus();
        });
        menus["&Help"]->addAction(installDesktopFile);
    }

    auto searchAction = new QAction(tr("Search action in UI"), this);
    searchAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
    connect(searchAction, &QAction::triggered, this, [this]() {
        auto model = new ActionListModel(this);
        auto window = getManager();
        model->setActions(collectWidgetActions(window));
        auto commandPalette = new CommandPalette(window);
        commandPalette->setDataModel(model);
        connect(commandPalette, &CommandPalette::didChooseItem, this,
                [commandPalette](const QModelIndex &index, const QAbstractItemModel *model) {
                    auto data = model->data(index, Qt::UserRole);
                    auto action = data.value<QAction *>();
                    if (action) {
                        action->trigger();
                    }
                    commandPalette->deleteLater();
                });
        commandPalette->show();
    });

    menus["&Help"]->addAction(actionCheckForUpdates);
#if defined(DEBUG_UPDATES)
    auto debugChecks = new QAction("Debug check for updates", this);
    connect(debugChecks, &QAction::triggered, this, [this]() { doStartupChecksForUpdate(); });
    menus["&Help"]->addAction(debugChecks);
#endif
    menus["&Help"]->addAction(searchAction);
    menus["&Help"]->addSeparator();
    menus["&Help"]->addAction(actionVisitHomePage);
    menus["&Help"]->addAction(actionAboutQt);
    menus["&Help"]->addAction(actionAbout);
}

HelpPlugin::~HelpPlugin() {}

void HelpPlugin::on_client_merged(qmdiHost *) {}

void HelpPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "A file system browser plugin");
}

void HelpPlugin::loadConfig(QSettings &settings) {
    IPlugin::loadConfig(settings);
    doStartupChecksForUpdate(false);
}

void HelpPlugin::doStartupChecksForUpdate(bool notifyUserNoUpdates) {
    QString lastCheckStr = getConfig().getLastUpdateTime();
    auto updatesCheck = getConfig().getUpdatesChecks();
    auto lastCheck = lastCheckStr.toULongLong();
    auto currentTime = QDateTime::currentSecsSinceEpoch();
    auto timeDiff = 0UL;

    switch (updatesCheck) {
    case UpdateCheck::NoChecks:
#if defined(DEBUG_UPDATES)
        qDebug() << ">No check at all";
#endif
        timeDiff = INT32_MAX;
        break;
    case UpdateCheck::EveryTime:
#if defined(DEBUG_UPDATES)
        qDebug() << ">Checks when app starts";
#endif
        break;
    case UpdateCheck::Daily:
#if defined(DEBUG_UPDATES)
        qDebug() << ">Checks Daily";
#endif
        timeDiff = 60 * 60 * 24;
        break;
    case UpdateCheck::Weekly:
#if defined(DEBUG_UPDATES)
        qDebug() << ">Checks wheekly";
#endif
        timeDiff = 60 * 60 * 24 * 7;
        break;
    }

#if defined(DEBUG_UPDATES)
    qDebug() << ">Current time = " << currentTime;
    qDebug() << ">Last check = " << lastCheck;
    qDebug() << ">Delta = " << currentTime - lastCheck << " < " << timeDiff;
#endif
    if (currentTime - lastCheck > timeDiff) {
        doChecksForUpdate(notifyUserNoUpdates);
        getConfig().setLastUpdateTime(QString::number(currentTime));
#if defined(DEBUG_UPDATES)
    } else {
        qDebug() << " - No need to check for updates";
#endif
    }
}

void HelpPlugin::doChecksForUpdate(bool notifyUserNoUpdates) {
    auto url = "https://raw.githubusercontent.com/diegoiast/qtedit4/refs/heads/main/updates.json";

    switch (getConfig().getUpdatesChannel()) {
    case UpdateChannels::Stable:
#if defined(DEBUG_UPDATES)
        qDebug() << "Checking updates from stable channel";
#endif
        break;
    case UpdateChannels::Testing:
#if defined(DEBUG_UPDATES)
        qDebug() << "Checking updates from testing channel";
#endif
        QSimpleUpdater::getInstance()->setPlatformKey(url, TESTING_CHANNEL);
        break;
    }
    QSimpleUpdater::getInstance()->setNotifyOnUpdate(url, true);
    QSimpleUpdater::getInstance()->setNotifyOnFinish(url, notifyUserNoUpdates);
    QSimpleUpdater::getInstance()->setDownloaderEnabled(url, true);
    QSimpleUpdater::getInstance()->checkForUpdates(url);

    auto currentTime = QDateTime::currentSecsSinceEpoch();
    getConfig().setLastUpdateTime(QString::number(currentTime));
}

void HelpPlugin::actionAbout_triggered() {
    auto appName = QCoreApplication::applicationName();
    auto version = QCoreApplication::applicationVersion();
    auto aboutText =
        QString(
            tr("<h2>%1 %2</h2>"
               "<p>A versatile text editor</p>"
               "<p>Home page: <a href='%3'>%3</a></p>"
               "<p>Licensed under the GNU General Public License v2 (GPLv2)</p>"
               "<p>This project uses <a href='https://www.qt.io/'>Qt6</a>, and the following "
               "libraries:</p>"
               "<ul>"
               "<li><a href='https://github.com/diegoiast/qmdilib'>qmdilib</a></li>"
               "<li><a href='https://github.com/diegoiast/qutepart-cpp'>qutepart-cpp</a></li>"
               "<li><a "
               "href='https://github.com/diegoiast/"
               "command-palette-widget'>command-palette-widget</a></li>"
               "<li><a href='https://github.com/palacaze/image-viewer'>image-viewer</a></li>"
               "<li><a href='https://github.com/Dax89/QHexView'>QHexView</a></li>"
               "<li><a "
               "href='https://github.com/alex-spataru/QSimpleUpdater'>QSimpleUpdater</a></li>"
               "</ul>"
               "<p>Copyright © 2024 <a href='mailto:diegoiast@gmail.com'>Diego Iastrubni</a> </p>"))
            .arg(appName, version, "https://github.com/diegoiast/qtedit4");

    auto aboutBox = QMessageBox(getManager());
    aboutBox.setWindowTitle(tr("About %1").arg(appName));
    aboutBox.setText(aboutText);
    aboutBox.setTextFormat(Qt::RichText);
    aboutBox.setIconPixmap(QPixmap(":/icons/qtedit4.png")
                               .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    aboutBox.exec();
}

void HelpPlugin::checkForUpdates_triggered() { doChecksForUpdate(true); }
