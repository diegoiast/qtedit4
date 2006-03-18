#ifndef __QE_CPP_HIGHLIGHTER_H__
#define __QE_CPP_HIGHLIGHTER_H__

#include <QSyntaxHighlighter>

#include "kateitemdatamanager.h"
#include "qemymap.h"

class QECPPHighlighter: public QSyntaxHighlighter
{
public:
	QECPPHighlighter( kateItemDataManager *d=NULL );
	QColor getDefaultColor();
	QColor getDefaultBackground();

private:
	virtual void highlightBlock( QTextBlock block );
	void setHighlight( QString fileName );
	
	void addMapping( const QString &pattern, const QTextCharFormat &format, bool fullWord=false  );
	void addMapping( const QString &pattern, const QString &formatName, bool fullWord=false );
	void addMappings_cpp();
	void addMappings_pro();

	QEMyMap mappings;
	kateItemDataManager *defaultColors;
};

#endif // __QE_CPP_HIGHLIGHTER_H__
