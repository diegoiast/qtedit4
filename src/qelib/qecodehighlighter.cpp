#include <QTextLayout>
#include <QList>

#include "qecodehighlighter.h"

/**
 * \class QECodeHighlighter 
 * \brief An abstruct class for drawing syntax
 *
 * An implemenation of QSyntaxHighlighter which can handle
 * highlight defintions from the kate project.
 *
 * This will make a normal QTextEdit show text just as kate does.
 *
 */


/**
 * Default constructor.
 * All it does is setting the highlightData to NULL and call
 * QSyntaxHighlighter's constructor.
 */
QECodeHighlighter::QECodeHighlighter()
{
}

QECodeHighlighter::~QECodeHighlighter()
{
}

void QECodeHighlighter::setHighlight( QString fileName )
{
	// hack to shut up warnings
// 	fileName[0];
	highlight( 0, 0, 0 );
}

void QECodeHighlighter::addToDocument(QTextDocument *doc)
{
	connect(doc, SIGNAL(contentsChange(int, int, int)), this, SLOT(highlight(int, int, int)));
}

QColor QECodeHighlighter::getDefaultColor()
{
	return Qt::black;
}

QColor QECodeHighlighter::getDefaultBackground()
{
	return Qt::white;
}

void QECodeHighlighter::highlight(int from, int removed, int added)
{
	QTextDocument *doc = qobject_cast<QTextDocument *>(sender());
	
	QTextBlock block = doc->findBlock(from);
	if (!block.isValid())
		return;
	
	QTextBlock endBlock;
	if (added > removed)
		endBlock = doc->findBlock(from + added);
	else
		endBlock = block;
	
	while (block.isValid() && !(endBlock < block))
	{
		highlightBlock(block);
		block = block.next();
	}
}
