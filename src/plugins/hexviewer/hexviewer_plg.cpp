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
#include <QLineEdit>
#include <QMessageBox>

class qmdiHexViewer : public QHexView, public qmdiClient {
  public:
    QString thisFileName;
    qmdiHexViewer(QWidget *p, const QString &fileName) : QHexView(p), qmdiClient() {
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

        auto fi = QFileInfo(fileName);
        auto document = QHexDocument::fromMappedFile(fileName, this);
        this->setDocument(document);
        this->mdiClientName = fi.fileName();
        this->thisFileName = fileName;
        this->contextMenu.addSeparator();
        this->contextMenu.addAction(actionCopyFileName);
        this->contextMenu.addAction(actionCopyFilePath);
        setupActions();
    }

    void setupActions() {
        auto actionFind =
            new QAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), tr("&Find"), this);
        auto actionReplace =
            new QAction(QIcon::fromTheme("edit-find-replace"), tr("&Replace"), this);
        auto actionCopyBinary =
            new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (binary"), this);
        auto actionCopyAsHex = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (HEX)"), this);
        auto actionPastBinary =
            new QAction(QIcon::fromTheme("edit-copy"), tr("Paste (binary)"), this);
        auto actionPastAsHex = new QAction(QIcon::fromTheme("edit-copy"), tr("Paste (hex)"), this);

        actionFind->setShortcut(QKeySequence::Find);
        actionReplace->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
        actionCopyBinary->setShortcut(QKeySequence::Copy);
        actionCopyAsHex->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
        actionPastBinary->setShortcut(QKeySequence::Paste);
        actionPastAsHex->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));

        connect(actionFind, &QAction::triggered, this, [this]() {
            auto d = new HexFindDialog(HexFindDialog::Type::Find, this);
            if (auto l = d->findChild<QLineEdit *>("qhexview_lefind")) {
                l->setFocus();
            }
            d->exec();
        });

        connect(actionReplace, &QAction::triggered, this, [this]() {
            auto d = new HexFindDialog(HexFindDialog::Type::Replace, this);
            d->exec();
        });

        toolbars[tr("main")]->addAction(actionFind);
        toolbars[tr("main")]->addAction(actionReplace);
        this->menus["&Search"]->addAction(actionFind);
        this->menus["&Search"]->addAction(actionReplace);
        this->menus["&Search"]->addAction(actionCopyBinary);
        this->menus["&Search"]->addAction(actionCopyAsHex);
        this->menus["&Search"]->addAction(actionPastBinary);
        this->menus["&Search"]->addAction(actionPastAsHex);
    }

    virtual bool canCloseClient() override {
        if (!this->hexDocument()->isModified()) {
            return true;
        }

        QMessageBox msgBox(
            QMessageBox::Warning, mdiClientName,
            tr("The (binary) document has been modified.\nDo you want to save your changes?"),
            QMessageBox::Yes | QMessageBox::Default, this);

        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes) {
            return doSave();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }

        this->hexDocument()->clearModified();
        return true;
    }

    virtual QString mdiClientFileName() override { return thisFileName; }

    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override { return {}; }

    bool doSave() {
        QFile file(thisFileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox msgBox(QMessageBox::Warning, mdiClientName, tr("Could not save file"),
                               QMessageBox::Ok, this);
            msgBox.exec();
            return false;
        }

        bool success = hexDocument()->saveTo(&file);
        file.close();
        return success;
    }
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

int HexViewrPlugin::canOpenFile(const QString &fileName) {
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

bool HexViewrPlugin::openFile(const QString &fileName, int, int, int) {
    auto tabWidget = dynamic_cast<QTabWidget *>(mdiServer);
    auto viewer = new qmdiHexViewer(tabWidget, fileName);
    mdiServer->addClient(viewer);
    return true;
}
