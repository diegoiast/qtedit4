#include "CTagsPlugin.hpp"
#include "CTagsLoader.hpp"
#include "GlobalCommands.hpp"

#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>

CTagsPlugin::CTagsPlugin() {
    name = tr("CTags support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    ctagsBinary = "ctags.exe";

    config.pluginName = tr("CTags");
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Universal CTags binary"))
                                     .setDescription(tr("Where is Universal CTags installed"))
                                     .setKey(Config::CTagsBinaryKey)
                                     .setType(qmdiConfigItem::Path)
                                     .setDefaultValue("ctags-universal")
                                     .setPossibleValue(true) // Must be an existing file
                                     .build());

    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Download CTags"))
                                     .setDescription(tr("Download and install Universal CTags"))
                                     .setKey(Config::DownloadCTagsKey)
                                     .setType(qmdiConfigItem::Button)
                                     .build());

    config.configItems.push_back(
        qmdiConfigItem::Builder()
            .setDisplayName(tr("<a href='https://github.com/universal-ctags/ctags'>Visit Universal "
                               "CTags home page</a>"))
            .setKey(Config::CTagsHomepageKey)
            .setType(qmdiConfigItem::Label)
            .build());

    connect(&qmdiDialogEvents::instance(), &qmdiDialogEvents::buttonClicked, this,
            [this](qmdiConfigDialog *dialog, const QString &buttonKey) {
                if (buttonKey == Config::DownloadCTagsKey) {
                    downloadCTags(dialog);
                }
            });
    connect(&qmdiDialogEvents::instance(), &qmdiDialogEvents::linkClicked, this,
            [=](qmdiConfigDialog *dialog, const QString &labelKey, const QString &url) {
                // todo - is this the correct way?
                QDesktopServices::openUrl(url);
            });
}

CTagsPlugin::~CTagsPlugin() { projects.clear(); }

CTagsPlugin::Config &CTagsPlugin::getConfig() {
    static Config configObject{&this->config};
    return configObject;
}

void CTagsPlugin::downloadCTags(qmdiConfigDialog *dialog) {
#ifdef Q_OS_LINUX
    auto url = QString("https://github.com/universal-ctags/ctags-nightly-build/releases/download/"
                       "2025.04.12%2Bfccad9b77c17c60e908a69af8d251eec5f5296b5/"
                       "uctags-2025.04.12-1-x86_64.pkg.tar.xz");
    auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto archivePath = dataDir + "/uctags-2025.04.12-1-x86_64.pkg.tar.xz";
    auto extractDir = dataDir + "/uctags-2025-04-12/";
    auto ctagsHomeOriginal = getConfig().getCTagsHomepage();

    QDir().mkpath(dataDir);
    QDir().mkpath(extractDir);

    if (!QFile::exists(archivePath)) {
        getConfig().setCTagsHomepage(tr("Downloading ctags binary"));
        dialog->updateWidgetValues();

        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl{url});
        QNetworkReply *reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(nullptr, "Download Error", reply->errorString());
            reply->deleteLater();
            getConfig().setCTagsHomepage(ctagsHomeOriginal);
            dialog->updateWidgetValues();
            return;
        }

        QFile file(archivePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            qDebug("Downloaded archive to %s", qPrintable(archivePath));
        } else {
            QMessageBox::critical(nullptr, "File Error", "Cannot write archive to disk.");
            reply->deleteLater();

            getConfig().setCTagsHomepage(ctagsHomeOriginal);
            dialog->updateWidgetValues();
            return;
        }
        reply->deleteLater();
    } else {
        qDebug("Archive already exists: %s", qPrintable(archivePath));
    }

    getConfig().setCTagsHomepage(tr("Extracting ctags"));

    QStringList args;
    args << "-xJvf" << archivePath << "-C" << extractDir;
    QProcess tar;
    tar.setWorkingDirectory(extractDir);
    tar.start("tar", args);
    tar.waitForFinished(-1);

#if 0
    auto stdoutStr = tar.readAllStandardOutput();
    auto stderrStr = tar.readAllStandardError();
    qDebug().noquote() << stdoutStr.trimmed();
    qWarning().noquote() << stderrStr.trimmed();
#endif

    if (tar.exitStatus() != QProcess::NormalExit || tar.exitCode() != 0) {
        qWarning("tar failed with code %d", tar.exitCode());
        QMessageBox::critical(nullptr, "Extraction Error", "Failed to extract required files.");
        return;
    }

    qDebug("Extracted ctags tools to %s", qPrintable(extractDir));
    getConfig().setCTagsBinary(extractDir + "usr/local/bin/ctags");
    getConfig().setCTagsHomepage(ctagsHomeOriginal);
    dialog->updateWidgetValues();
#else
    QMessageBox::information(nullptr, tr("Download CTags"),
                             tr("Universal CTags can be downloaded from:\n\n"
                                "https://github.com/universal-ctags/ctags\n\n"
                                "Please follow the installation instructions for your platform."));
#endif
}

int CTagsPlugin::canHandleCommand(const QString &command, const CommandArgs &) const {
    if (command == GlobalCommands::BuildFinished) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::ProjectLoaded) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::VariableInfo) {
        return CommandPriority::HighPriority;
    }
    return CommandPriority::CannotHandle;
}

CommandArgs CTagsPlugin::handleCommand(const QString &command, const CommandArgs &args) {
    if (command == GlobalCommands::BuildFinished) {
        auto sourceDir = args["sourceDir"].toString();
        auto buildDirectory = args["buildDirectory"].toString();
        auto projectName = args["projectName"].toString();
        newProjectBuilt(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::ProjectLoaded) {
        auto projectName = args[GlobalArguments::ProjectName].toString();
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString();
        newProjectAdded(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::VariableInfo) {
        auto filename = args[GlobalArguments::FileName].toString();
        auto symbol = args[GlobalArguments::RequestedSymbol].toString();
        auto exactMatch = args[GlobalArguments::RequestedSymbol].toBool();
        return symbolInfoRequested(filename, symbol, exactMatch);
    }
    return {};
}

void CTagsPlugin::setCTagsBinary(const QString &newBinary) {
    this->ctagsBinary = newBinary;
    for (const auto &k : projects.keys()) {
        projects[k]->setCTagsBinary(newBinary.toStdString());
    }
}

void CTagsPlugin::newProjectAdded(const QString &projectName, const QString &sourceDir,
                                  const QString &buildDirectory) {
    CTagsLoader *ctags = nullptr;
    if (projectName.isEmpty() || buildDirectory.isEmpty()) {
        return;
    }

    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (projects.contains(nativeSourceDir)) {
        ctags = projects.value(nativeSourceDir);
    } else {
        ctags = new CTagsLoader(ctagsBinary.toStdString());
        projects[nativeSourceDir] = ctags;
    }

    auto ctagsFile =
        QDir::toNativeSeparators(buildDirectory) + QDir::separator() + projectName + ".tags";
    ctags->loadFile(ctagsFile.toStdString());
}

void CTagsPlugin::newProjectBuilt(const QString &projectName, const QString &sourceDir,
                                  const QString &buildDirectory) {
    // project should be loaded first, something is borked
    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (!projects.contains(nativeSourceDir)) {
        qDebug() << "Project build, but not added first! ctags will not suppor it"
                 << nativeSourceDir;
        return;
    }
    auto ctags = projects.value(nativeSourceDir);
    auto ctagsFile = buildDirectory + QDir::separator() + projectName + ".tags";
    ctags->scanDirs(ctagsFile.toStdString(),
                    QDir::toNativeSeparators(nativeSourceDir).toStdString());
}

CommandArgs CTagsPlugin::symbolInfoRequested(const QString &fileName, const QString &symbol,
                                             bool exactMatch) {
    CTagsLoader *project = nullptr;
    for (const auto &k : projects.keys()) {
        if (fileName.startsWith(k)) {
            project = projects[k];
            break;
        }
    }

    if (!project) {
        return {};
    }

    QVariantList tagList;
    auto tags = project->findTags(symbol.toStdString(), exactMatch);
    for (auto &tagRef : tags) {
        const CTag &tag = tagRef.get();

        tagList.append(QVariant::fromValue(CommandArgs{
            {GlobalArguments::FileName, QString::fromStdString(tag.file)},
            {"fieldType", QString::fromStdString(tagFieldKeyToString(tag.field))},
            {"fieldValue", QString::fromStdString(tag.fieldValue)},
            {"raw", QString::fromStdString(tag.address)},
        }));
    }

    CommandArgs res;
    res["symbol"] = symbol;
    res["fileName"] = fileName;
    res["tags"] = tagList;

    return res;
}
