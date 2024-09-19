/**
 * \file imageviewer_plg
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License MIT
 * \see class name
 */

#include "imageviewer_plg.h"
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <pal/image-viewer.h>

class qmdiImageViewer : public pal::ImageViewer, public qmdiClient {
  public:
    QString thisFileName;
    qmdiImageViewer(QWidget *p, const QString &fileName) : pal::ImageViewer(p) {
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
        this->setImage(QImage(fileName));
        this->mdiClientName = fi.fileName();
        this->thisFileName = fileName;
        this->contextMenu.addSeparator();
        this->contextMenu.addAction(actionCopyFileName);
        this->contextMenu.addAction(actionCopyFilePath);
    }

    virtual QString mdiClientFileName() override { return thisFileName; }

    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override { return {}; }
};

ImageViewrPlugin::ImageViewrPlugin() {
    name = tr("Image viewer plugin - based on QutePart");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
}

ImageViewrPlugin::~ImageViewrPlugin() {}

QStringList ImageViewrPlugin::myExtensions() {
    QStringList s;
    s << tr("Images", "ImageViewrPlugin::myExtensions") + " (*.jpg *.jpeg *.bmp *.png *.pcx *.ico)";
    return s;
}

int ImageViewrPlugin::canOpenFile(const QString fileName) {
    if (fileName.endsWith(".jpg", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".pcx", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".ico", Qt::CaseInsensitive)) {
        return 5;
    } else {
        return -1;
    }
}

bool ImageViewrPlugin::openFile(const QString fileName, int x, int y, int z) {
    auto tabWidget = dynamic_cast<QTabWidget *>(mdiServer);
    auto fi = QFileInfo(fileName);
    auto viewer = new qmdiImageViewer(tabWidget, fileName);
    mdiServer->addClient(viewer);
    return true;
}
