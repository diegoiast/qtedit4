/**
 * \brief Second demo - interfacting with the syntax highlighter
 *
 * This example opens a main window, which lets the user load a text file.
 * Then the user will be able to choose the syntax highlighter and color
 * definitions / pallete for the editor.
 *
 * When building from QtCreator, open the Projects tab, in the Build &Run
 * section choose this demo, and in Working direcotry type %{sourceDir}.
 */

#include <QApplication>
#include "mainwindow2.h"

int main(int argc, char *argv[])
{
        QApplication a(argc, argv);
        MainWindow2 w;
        w.show();
        a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
        return a.exec();
}
