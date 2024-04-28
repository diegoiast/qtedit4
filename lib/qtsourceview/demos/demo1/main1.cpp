/**
 * \brief First Demo - a plain QTextEdit with QsvSyntaxHighlighter
 *
 * This demo shows how to load a color definition, then a language definition
 * for the syntax highlighter. Then the code creates a syntax highlighter and
 * applies it to an editor.
 *
 * Note that this demo needs to read files from the real file system, and
 * when building out of source, like QtCreator does, the app will fail at
 * runtime. Please run the app from the source directory.
 *
 * When building from command line:
 * \code
 * mkdir qbuild
 * cd qbuild
 * qmake ../
 * make
 * cd ..
 * ./qbuild/demo1
 * \endcode
 *
 * When building from QtCreator, open the Projects tab, in the Build &Run
 * section choose this demo, and in Working direcotry type %{sourceDir}.
 */

#include <QTextEdit>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "qsvsh/qsvcolordef.h"
#include "qsvsh/qsvcolordeffactory.h"
#include "qsvsh/qsvlangdef.h"
#include "qsvsh/qsvsyntaxhighlighter.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTextEdit *textEdit = new QTextEdit;
	
	// load a default color set
	QsvColorDefFactory *defColors = new QsvColorDefFactory("data/colors/kate.xml");
	
	// load a default language definition
	QsvLangDef *langCpp = new QsvLangDef("data/langs/cpp.lang");
	if (!defColors->isValid() || !langCpp->isValid()) {
		QMessageBox::information(textEdit, 
			textEdit->tr("Read documentation"),
			textEdit->tr("Cannot find color or language definition.\n\n"
			"Are you running the app from the top dir?\n"
			"If running from QtCreator, set up working directory to %{sourceDir}\n"
			"See documentation in main1.cpp"
		));
	}
	
	// new syntax highlighter, with the default colors and language
	QsvSyntaxHighlighter *highlight = new QsvSyntaxHighlighter(textEdit, defColors, langCpp);
	
	textEdit->show();
	QFile file("demos/demo1/main1.cpp");
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QTextStream in(&file);
		textEdit->setPlainText(in.readAll());
	}
	
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}
