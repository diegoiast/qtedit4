#ifndef __qe_CODE_EDITOR_H__
#define __qe_CODE_EDITOR_H__

// forward declarations instead of including headers
class QWidget;
class QTextCodec;
class QString;

class QECodeHighlighter;

class QECodeEditor: public QTextEdit
{
public:
	QECodeEditor( QWidget *p, QECodeHighlighter *h=NULL );
	virtual ~QECodeEditor();
	QString getFileName();
	
	bool loadFile( QString fileName );
	bool loadFile( QString fileName, QTextCodec *c );
	bool saveFile( QString fileName  );
	bool saveFile( QString fileName, QTextCodec *c );
	void setHighlight( QString fileName );

private:
	
	QString    fileName;
	QECodeHighlighter *codeHighlighter;
};


#endif // __qe_CODE_EDITOR_H__
