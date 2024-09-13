#include "imageviewer_plg.h"
#include "QImageViewer.h"

#include <QFileInfo>

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
    auto viewer = new QImageViewer(getManager());
    viewer->loadFile(fileName);
    auto fi = QFileInfo(fileName);
    tabWidget->addTab(viewer, fi.fileName());
    return true;
}

void ImageViewrPlugin::showAbout() {}

void ImageViewrPlugin::actionAbout_triggered() {}
