#ifndef __TEXT_EDITOR_EX_H__
#define __TEXT_EDITOR_EX_H__

// class QTextEdit;
#include <QTextEdit>

class TextEditorEx: public QTextEdit
{
        Q_OBJECT
public:
	TextEditorEx(QWidget *parent = 0);
	void setCurrentLineColor( QColor newColor );
	void setHightlighCurrentLine( bool enable );
	bool getHightlighCurrentLine() { return highlightCurrentLine; };


private slots:
	void	updateCurrentLine();

protected:
	void	paintEvent(QPaintEvent *event);

private:
	QColor	currentLineColor;
	bool	highlightCurrentLine;
};

#endif //__TEXT_EDITOR_EX_H__
