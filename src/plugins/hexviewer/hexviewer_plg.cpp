/**
 * \file imageviewer_plg
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License MIT
 * \see class name
 */

#include "hexviewer_plg.h"
#include "qclipboard.h"
#include <QApplication>
#include <QFileInfo>
#include <QHexView/dialogs/hexfinddialog.h>
#include <QHexView/model/buffer/qmemorybuffer.h>
#include <QHexView/qhexview.h>

class qmdiHexViewer : public QHexView, public qmdiClient {
  public:
    QString thisFileName;
    qmdiHexViewer(QWidget *p, const QString &fileName) : QHexView(p), qmdiClient() {
        auto document = QHexDocument::fromMappedFile(fileName, this);
        auto fi = QFileInfo(fileName);
        auto actionCopyFileName = new QAction(tr("Copy filename to clipboard"), this);
        auto actionCopyFilePath = new QAction(tr("Copy full path to clipboard"), this);
        connect(actionCopyFileName, &QAction::triggered, this, [this]() {
            auto c = QApplication::clipboard();
            c->setText(this->mdiClientFileName());
        });
        connect(actionCopyFilePath, &QAction::triggered, this, [this]() {
            auto c = QApplication::clipboard();
            c->setText(mdiClientName);
        });

        this->setDocument(document);
        this->mdiClientName = fi.fileName();
        this->thisFileName = fileName;
        this->contextMenu.addSeparator();
        this->contextMenu.addAction(actionCopyFileName);
        this->contextMenu.addAction(actionCopyFilePath);

        auto actionFind = new QAction(tr("Find"), this);
        actionFind->setShortcut(QKeySequence::Find);
        actionFind = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), tr("&Find"), this);
        connect(actionFind, &QAction::triggered, this, [this]() {
            auto d = new HexFindDialog(HexFindDialog::Type::Find, this);
            d->exec();
        });
        toolbars[tr("main")]->addAction(actionFind);

#if 0     
        auto actionReplace =
            new QAction(QIcon::fromTheme("edit-find-replace"), tr("&Replace"), this);
        actionReplace->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
        actionReplace = new QAction(QIcon::fromTheme("edit-find-replace"), tr("&Replace"), this);
        connect(actionReplace, &QAction::triggered, this, [this]() {
            auto d = new HexFindDialog(HexFindDialog::Type::Replace, this);
            d->exec();
        });

        toolbars[tr("main")]->addAction(actionReplace);
#else
        this->setReadOnly(true);
#endif
    }

    virtual QString mdiClientFileName() override { return thisFileName; }

    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override { return {}; }
};

HexViewrPlugin::HexViewrPlugin() {
    name = tr("Image viewer plugin - based on QutePart");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
}

HexViewrPlugin::~HexViewrPlugin() {}

QStringList HexViewrPlugin::myExtensions() {
    QStringList s;
    // s << tr("Images", "ImageViewrPlugin::myExtensions") + " (*.jpg *.jpeg *.bmp *.png *.pcx
    // *.ico)";
    return s;
}

int HexViewrPlugin::canOpenFile(const QString fileName) {
    static const QStringList extensions = {".bin", ".img", "blob", ".so",   ".AppImage",
                                           ".a",   ".exe", ".dll", ".dlib", ".pdf"};
    for (const QString &ext : extensions) {
        if (fileName.endsWith(ext, Qt::CaseInsensitive)) {
            return 6;
        }
    }

    if (!fileName.contains(".")) {
        return 5;
    }
    return 2;
}

bool HexViewrPlugin::openFile(const QString fileName, int x, int y, int z) {
    auto tabWidget = dynamic_cast<QTabWidget *>(mdiServer);
    auto fi = QFileInfo(fileName);
    auto viewer = new qmdiHexViewer(tabWidget, fileName);
    mdiServer->addClient(viewer);
    return true;
}
