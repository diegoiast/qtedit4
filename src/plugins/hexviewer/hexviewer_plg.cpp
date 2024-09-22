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
#include <model/buffer/qmemorybuffer.h>
#include <qhexview.h>

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
