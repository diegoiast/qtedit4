#ifndef __QE_LANGDEF_FACTORY_H__
#define __QE_LANGDEF_FACTORY_H__

#include <QMap>

class QeGtkSourceViewLangDef;
class QString;

class QeLangDefFactory
{
public:
	static QeLangDefFactory *getInstanse();
	QeGtkSourceViewLangDef *getHighlight( QString fileName );
	void loadDirectory( QString directory );

private:	
	QeLangDefFactory(void);
	~QeLangDefFactory(void);
	
	static QeLangDefFactory *LangFactory;
	QList<QeGtkSourceViewLangDef*> langList;
	QMap<QString,QStringList> mimeTypes;
};

#endif // __QE_LANGDEF_FACTORY_H__
