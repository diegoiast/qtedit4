#ifndef __KATE_LANGUAGE_H__
#define __KATE_LANGUAGE_H__

#include <QString>
#include <QDomDocument>

#include "kateqtglobal.h"
#include "katehighlight.h"
#include "kategeneral.h"


class kateLanguage
{
public:
	kateLanguage();
	kateLanguage( QString fileName );

	bool loadFromFile( QString fileName );
	bool saveToFile( QString fileName );

	bool load( QDomDocument doc );
	bool save( QDomDocument doc );
// private:
	QStringMap    attributes;
	kateHighlight highlight;
	kateGeneral   general;
};

#endif // __KATE_LANGUAGE_H__
