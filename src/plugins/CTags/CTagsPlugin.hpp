#pragma once

#include "iplugin.h"
#include "qmdidialogevents.hpp"

/**
 * \file CTagsPlugin.hpp
 * \brief Definition of the CTag support plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 * \see ProjectManager
 */

#pragma once

class CTagsLoader;

class CTagsPlugin : public IPlugin {

    struct Config {
        CONFIG_DEFINE(CTagsBinary, QString)
        CONFIG_DEFINE(DownloadCTags, QString)
        CONFIG_DEFINE(CTagsHomepage, QString)
        qmdiPluginConfig *config;
    };
    Config &getConfig();
    QHash<QString, CTagsLoader *> projects;

    QString ctagsBinary = "ctags";

  public:
    CTagsPlugin();
    ~CTagsPlugin();

    virtual int canHandleCommand(const QString &command, const CommandArgs &args) const override;
    virtual CommandArgs handleCommand(const QString &command, const CommandArgs &args) override;

    void setCTagsBinary(const QString &newBinary);
    void downloadCTags(qmdiConfigDialog *dialog);

    void extractArchive(const QString &archivePath, const QString &extractDir,
                        qmdiConfigDialog *dialog, const QString &downloadTextOriginal);

  protected:
    void newProjectAdded(const QString &projectName, const QString &sourceDir,
                         const QString &buildDirectory);
    void newProjectBuilt(const QString &projectName, const QString &sourceDir,
                         const QString &buildDirectory);
    CommandArgs symbolInfoRequested(const QString &fileName, const QString &symbol,
                                    bool exactMatch);

  private:
};
