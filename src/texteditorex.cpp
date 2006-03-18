#include <QFont>
#include <QPainter>
#include <QTextEdit>
#include "texteditorex.h"


TextEditorEx::TextEditorEx(QWidget *parent): QTextEdit(parent)
{
	currentLineColor = QColor( "#EEF6FF" );
	highlightCurrentLine = true;
        setFrameShape ( QFrame::NoFrame );
//         setFrameShadow ( QFrame::Sunken );

	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCurrentLine()));
}

void TextEditorEx::setCurrentLineColor( QColor newColor )
{
	currentLineColor = newColor;
	viewport()->update();
}

void TextEditorEx::setHightlighCurrentLine( bool enable )
{
	highlightCurrentLine = enable;
	viewport()->update();
}

void TextEditorEx::paintEvent(QPaintEvent *event)
{
	if (highlightCurrentLine)
	{
		QRect  rect = cursorRect();
		QPainter painter(viewport());
		const QBrush brush(currentLineColor);

		rect.setX(0);
		rect.setWidth(viewport()->width());
		painter.fillRect(rect, brush);
		painter.end();
	}

	QTextEdit::paintEvent(event);
}

void TextEditorEx::updateCurrentLine()
{
	if (highlightCurrentLine)
		viewport()->update();
}
