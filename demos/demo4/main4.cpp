/**
 * \brief Forth demo - better text editor control
 *
 * The default text edit available for Qt is very simple. This example
 * shows a more advanced text editor, which has a built in find/replace/
 * goto line controls (inline), displays the current line with different
 * backgroud, has line numbering... and more.
 *
 * When building from QtCreator, open the Projects tab, in the Build &Run
 * section choose this demo, and in Working direcotry type %{sourceDir}.
 */

#include "mainwindow4.h"

int main( int argc, char* argv[] )
{
	QApplication app( argc, argv );
	MainWindow4 window("");
	window.show();
	return app.exec();
}
