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

// FIXME: this is an ugly workaround. This is private API for Qt, and
//        might break. I thing this is stable enough for now.
//        Copied from 6.8.3\include\QtCore\6.8.3\QtCore\private
class QZipReaderPrivate;
class Q_CORE_EXPORT QZipReader {
  public:
    explicit QZipReader(const QString &fileName, QIODevice::OpenMode mode = QIODevice::ReadOnly);

    explicit QZipReader(QIODevice *device);
    ~QZipReader();

    QIODevice *device() const;

    bool isReadable() const;
    bool exists() const;

    struct FileInfo {
        FileInfo() noexcept : isDir(false), isFile(false), isSymLink(false), crc(0), size(0) {}

        bool isValid() const noexcept { return isDir || isFile || isSymLink; }

        QString filePath;
        uint isDir : 1;
        uint isFile : 1;
        uint isSymLink : 1;
        QFile::Permissions permissions;
        uint crc;
        qint64 size;
        QDateTime lastModified;
    };

    QList<FileInfo> fileInfoList() const;
    int count() const;

    FileInfo entryInfoAt(int index) const;
    QByteArray fileData(const QString &fileName) const;
    bool extractAll(const QString &destinationDir) const;

    enum Status { NoError, FileReadError, FileOpenError, FilePermissionsError, FileError };

    Status status() const;

    void close();

  private:
    QZipReaderPrivate *d;
    Q_DISABLE_COPY_MOVE(QZipReader)
};

Q_DECLARE_TYPEINFO(QZipReader::FileInfo, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(QZipReader::Status, Q_PRIMITIVE_TYPE);

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
            [=](qmdiConfigDialog *, const QString &labelKey, const QString &url) {
                if (Config::CTagsHomepageKey == labelKey) {
                    QDesktopServices::openUrl(url);
                }
            });
}

CTagsPlugin::~CTagsPlugin() { projects.clear(); }

CTagsPlugin::Config &CTagsPlugin::getConfig() {
    static Config configObject{&this->config};
    return configObject;
}

void CTagsPlugin::downloadCTags(qmdiConfigDialog *dialog) {
#if !defined(Q_OS_LINUX) && !defined(Q_OS_WIN)
    QMessageBox::information(nullptr, tr("Download CTags"),
                             tr("Universal CTags can be downloaded from:\n\n"
                                "https://github.com/universal-ctags/ctags\n\n"
                                "Please follow the installation instructions for your platform."));
    return;
#endif

    QString url;
    QString fileName;
    QString extractDir;
    auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

#if defined(Q_OS_LINUX)
    url = "https://github.com/universal-ctags/ctags-nightly-build/releases/download/"
          "2025.04.12%2Bfccad9b77c17c60e908a69af8d251eec5f5296b5/"
          "uctags-2025.04.12-1-x86_64.pkg.tar.xz";
    fileName = "uctags-2025.04.12-1-x86_64.pkg.tar.xz";
    extractDir = dataDir + "/uctags-2025-04-12/";
#elif defined(Q_OS_WIN)
    url = "https://github.com/universal-ctags/ctags-win32/releases/download/"
          "2025-04-14%2Fp6.1.20250413.0-3-g00ae476/"
          "ctags-2025-04-14_p6.1.20250413.0-3-g00ae476-clang-x64.zip";
    fileName = "ctags-2025-04-14_p6.1.20250413.0-3-g00ae476-clang-x64.zip";
    extractDir = dataDir + "/ctags-2025-04-14/";
#endif

    auto archivePath = dataDir + "/" + fileName;
    auto downloadTextOriginal = getConfig().getDownloadCTags();
    QDir().mkpath(extractDir);

#if 0
    if (QFile::exists(archivePath)) {
        qDebug() << "Archive already exists at" << archivePath;
        extractArchive(archivePath, extractDir, dialog, downloadCtagsOriginal);
        return;
    }
#endif
    getConfig().setDownloadCTags(tr("Downloading..."));

    dialog->updateWidgetValues();
    auto request = QNetworkRequest(QUrl{url});
    auto manager = new QNetworkAccessManager(this);
    auto reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(nullptr, "Download Error", reply->errorString());
            getConfig().setDownloadCTags(downloadTextOriginal);
            dialog->updateWidgetValues();
            return;
        }

        QFile file(archivePath);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(nullptr, tr("File Error"), tr("Cannot write archive to disk."));
            getConfig().setDownloadCTags(downloadTextOriginal);
            dialog->updateWidgetValues();
            return;
        }
        file.write(reply->readAll());
        file.close();
        extractArchive(archivePath, extractDir, dialog, downloadTextOriginal);
    });
}

void CTagsPlugin::extractArchive(const QString &archivePath, const QString &extractDir,
                                 qmdiConfigDialog *dialog, const QString &downloadTextOriginal) {

    getConfig().setDownloadCTags(tr("Extracting ctags"));
    dialog->updateWidgetValues();

    // usually on Unix systems
    if (archivePath.endsWith(".tar.xz")) {
        QStringList args{"-xvf", archivePath, "-C", extractDir};
        QProcess *tar = new QProcess(this);
        tar->setWorkingDirectory(extractDir);
        connect(tar, &QProcess::finished, this,
                [=, this](int exitCode, QProcess::ExitStatus exitStatus) {
                    tar->deleteLater();

                    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                        qWarning("tar failed with code %d", exitCode);
                        QMessageBox::critical(nullptr, "Extraction Error",
                                              "Failed to extract required files.");
                        getConfig().setDownloadCTags(downloadTextOriginal);
                        dialog->updateWidgetValues();
                        return;
                    }

                    getConfig().setCTagsBinary(extractDir + "usr/local/bin/ctags");
                    getConfig().setDownloadCTags(downloadTextOriginal);
                    dialog->updateWidgetValues();
                });
        tar->start("tar", args);
    }

    // usually on Windows systems
    if (archivePath.endsWith(".zip")) {
        auto binaryPath = extractDir + "ctags.exe";
        QZipReader z(archivePath);
        z.extractAll(extractDir);

        getConfig().setCTagsBinary(QDir::toNativeSeparators(binaryPath));
        getConfig().setCTagsHomepage(downloadTextOriginal);
        dialog->updateWidgetValues();
    }
}

int CTagsPlugin::canHandleCommand(const QString &command, const CommandArgs &) const {
    if (command == GlobalCommands::BuildFinished) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::ProjectLoaded) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::ProjectRemoved) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::VariableInfo) {
        return CommandPriority::HighPriority;
    }
    return CommandPriority::CannotHandle;
}

CommandArgs CTagsPlugin::handleCommand(const QString &command, const CommandArgs &args) {
    if (command == GlobalCommands::BuildFinished) {
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString();
        auto projectName = args[GlobalArguments::Name].toString();
        newProjectBuilt(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::ProjectLoaded) {
        auto projectName = args[GlobalArguments::ProjectName].toString();
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString();
        newProjectAdded(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::ProjectRemoved) {
        auto projectName = args[GlobalArguments::ProjectName].toString();
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString();
        projectRemoved(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::VariableInfo) {
        auto filename = args[GlobalArguments::FileName].toString();
        auto symbol = args[GlobalArguments::RequestedSymbol].toString();
        auto exactMatch = args[GlobalArguments::ExactMatch].toBool();
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
    if (projectName.isEmpty() || buildDirectory.isEmpty()) {
        return;
    }

    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (!projects.contains(nativeSourceDir)) {
        projects[nativeSourceDir] = new CTagsLoader(ctagsBinary.toStdString());
    }

    auto ctagsFile = buildDirectory + QDir::separator() + projectName + ".tags";
    auto ctags = projects[nativeSourceDir];
    ctags->setCTagsBinary(getConfig().getCTagsBinary().toStdString());
    ctags->loadFile(ctagsFile.toStdString());
}

void CTagsPlugin::projectRemoved(const QString &projectName, const QString &sourceDir,
                                 const QString &buildDirectory) {
    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (!projects.contains(nativeSourceDir)) {
        qDebug() << "CTagsPlugin: Tried unloading project, but not found" << nativeSourceDir
                 << projectName << buildDirectory;
        return;
    }
    auto ctags = projects[nativeSourceDir];
    ctags->clear();
    projects.remove(nativeSourceDir);
    delete ctags;
}

void CTagsPlugin::newProjectBuilt(const QString &projectName, const QString &sourceDir,
                                  const QString &buildDirectory) {
    // project should be loaded first, something is borked
    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (!projects.contains(nativeSourceDir)) {
        qDebug() << "CTagsPlugin: Project build, but not added first! ctags will not support it"
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
        qDebug() << "CTagsPlugin: " << fileName
                 << "did not find project, will not return symbol info";
        return {};
    }

    QVariantList tagList;
    auto tags = project->findTags(symbol.toStdString(), exactMatch);
    for (auto &tagRef : tags) {
        const CTag &tag = tagRef.get();
        tagList.append(QVariant::fromValue(CommandArgs{
            {GlobalArguments::FileName, QString::fromStdString(tag.file)},
            {GlobalArguments::Type, QString::fromStdString(tagFieldKeyToString(tag.field))},
            {GlobalArguments::Value, QString::fromStdString(tag.fieldValue)},
            {GlobalArguments::Raw, QString::fromStdString(tag.address)},
            {GlobalArguments::Name, QString::fromStdString(tag.name)},
        }));
    }

    CommandArgs res;
    res["symbol"] = symbol;
    res["fileName"] = fileName;
    res["tags"] = tagList;
    return res;
}
