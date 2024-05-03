#include "tests/generic-item-complete-window.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GenericItemWindow w;
    w.show();
    return app.exec();
}
