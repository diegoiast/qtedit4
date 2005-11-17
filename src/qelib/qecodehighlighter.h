#ifndef __qe_CODE_HILIGHTER_H__
#define __qe_CODE_HILIGHTER_H__

#include <QString>
#include <QObject>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextBlock>

class QECodeHighlighter : public QObject
{
	Q_OBJECT
public:
	QECodeHighlighter();
	virtual ~QECodeHighlighter();
	virtual void setHighlight( QString fileName );
	void addToDocument(QTextDocument *doc);
	virtual QColor getDefaultColor();
	virtual QColor getDefaultBackground();

public slots:
	void highlight(int from, int removed, int added);

private slots:
	virtual void highlightBlock(QTextBlock block) = 0;
};

#endif //  __qe_CODE_HILIGHTER_H__
