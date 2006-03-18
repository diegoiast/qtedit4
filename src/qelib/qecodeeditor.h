#ifndef __QE_CODE_EDITOR_H__
#define __QE_CODE_EDITOR_H__

#include <QTextEdit>

// forward declarations instead of including headers
class QWidget;
class QTextCodec;
class QString;
class QAction;
class QSyntaxHighlighter;

class QECodeEditor: public QTextEdit
{
public:
	QECodeEditor( QWidget *p, QSyntaxHighlighter *h=NULL );
	virtual ~QECodeEditor();
	QString getFileName();
	
	bool loadFile( QString fileName );
	bool loadFile( QString fileName, QTextCodec *c );
	bool saveFile( QString fileName  );
	bool saveFile( QString fileName, QTextCodec *c );
	void setHighlight( QString fileName );

private:
	
	QString    fileName;
	QSyntaxHighlighter *codeHighlighter;
};


#endif // __QE_CODE_EDITOR_H__
