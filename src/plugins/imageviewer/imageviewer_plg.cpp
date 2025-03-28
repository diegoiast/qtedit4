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

        auto zoomInAction = new QAction(this);
        zoomInAction->setAutoRepeat(true);
        zoomInAction->setToolTip(tr("Zoom int image"));
        zoomInAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn));
        connect(zoomInAction, &QAction::triggered, this, [this]() { this->zoomIn(); });

        auto zoomOutAction = new QAction(this);
        zoomOutAction->setAutoRepeat(true);
        zoomOutAction->setToolTip(tr("Fit image out"));
        zoomOutAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut));
        connect(zoomOutAction, &QAction::triggered, this, [this]() { this->zoomOut(); });

        auto zoomFitAction = new QAction(this);
        zoomFitAction->setToolTip(tr("Fit image to window"));
        zoomFitAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest));
        connect(zoomFitAction, &QAction::triggered, this, &ImageViewer::zoomFit);

        auto zoomOriginalAction = new QAction(this);
        zoomOriginalAction->setToolTip(tr("Show image at original size"));
        zoomOriginalAction->setIcon(QIcon::fromTheme("zoom-original"));
        connect(zoomOriginalAction, &QAction::triggered, this, &ImageViewer::zoomOriginal);

        this->toolbars[tr("main")]->addAction(zoomInAction);
        this->toolbars[tr("main")]->addAction(zoomOutAction);
        this->toolbars[tr("main")]->addAction(zoomFitAction);
        this->toolbars[tr("main")]->addAction(zoomOriginalAction);
    }

    ~qmdiImageViewer() { mdiServer = nullptr; }

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

int ImageViewrPlugin::canOpenFile(const QString &fileName) {
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

bool ImageViewrPlugin::openFile(const QString &fileName, int, int, int) {
    auto parentWidget = dynamic_cast<QWidget *>(mdiServer);
    auto viewer = new qmdiImageViewer(parentWidget, fileName);
    mdiServer->addClient(viewer);
    return true;
}
