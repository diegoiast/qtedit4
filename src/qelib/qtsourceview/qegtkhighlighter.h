#ifndef __QE_GTK_HIGHLIGHTER_H__
#define __QE_GTK_HIGHLIGHTER_H__

#include <QSyntaxHighlighter>
#include "qelib/qtsourceview/qorderedmap.h"

class QTextCharFormat;
class QeGtkSourceViewLangDef;
class kateItemDataManager;

class QeGTK_Highlighter: QSyntaxHighlighter
{
public:
	QeGTK_Highlighter( QTextDocument *parent = 0, kateItemDataManager *manager=0 );
	QeGTK_Highlighter( QTextEdit *parent = 0, kateItemDataManager *manager=0 );
	void setHighlight( QeGtkSourceViewLangDef *lang );

protected:
	void highlightBlock(const QString &text);

private:
	void addMapping(const QString &pattern, const QTextCharFormat &format, bool fullWord=false );
	void addMapping(const QString &pattern, const QString formatName, bool fullWord=false );

	QOrderedMap<QString,QTextCharFormat> mappings;
	kateItemDataManager *manager;
	QeGtkSourceViewLangDef *language;
};

#endif  // __QE_GTK_HIGHLIGHTER_H__
