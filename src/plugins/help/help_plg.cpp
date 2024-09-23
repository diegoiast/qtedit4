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
#include <QMessageBox>
#include <QUrl>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <QFile>
#include <windows.h>
#else
#include <QDesktopServices>
#include <QFile>
#include <unistd.h>
#endif

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

    actionAbout = new QAction(tr("&About"), this);
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(actionAbout_triggered()));

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
        menus["&Help"]->addSeparator();
    }

    menus["&Help"]->addAction(actionVisitHomePage);
    menus["&Help"]->addAction(actionAboutQt);
    menus["&Help"]->addAction(actionAbout);
}

HelpPlugin::~HelpPlugin() {}

void HelpPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "A file system browser plugin");
}

void HelpPlugin::actionAbout_triggered() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "QtEdit4 - a text editor");
}
