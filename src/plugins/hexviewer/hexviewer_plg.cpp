/**
 * \file imageviewer_plg
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License MIT
 * \see class name
 */

#include "hexviewer_plg.h"
#include <QFileInfo>
#include <model/buffer/qmemorybuffer.h>
#include <qhexview.h>

class qmdiHexViewer : public QHexView, public qmdiClient {
  public:
    QString thisFileName;
    qmdiHexViewer(QWidget *p, const QString &fileName) : QHexView(p) {
        QHexDocument *document = QHexDocument::fromMappedFile(fileName, this);
        this->setDocument(document);

        auto fi = QFileInfo(fileName);
        this->mdiClientName = fi.fileName();
        this->thisFileName = fileName;
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
    static const QStringList extensions = {".bin", ".img", "blob", ".so",  ".AppImage",
                                           ".a",   ".exe", ".dll", ".dlib"};
    for (const QString &ext : extensions) {
        if (fileName.endsWith(ext, Qt::CaseInsensitive)) {
            return 5;
        }
    }

    if (!fileName.contains(".")) {
        return 5;
    }
    return 1;
}

bool HexViewrPlugin::openFile(const QString fileName, int x, int y, int z) {
    auto tabWidget = dynamic_cast<QTabWidget *>(mdiServer);
    auto fi = QFileInfo(fileName);
    auto viewer = new qmdiHexViewer(tabWidget, fileName);
    mdiServer->addClient(viewer);
    return true;
}
