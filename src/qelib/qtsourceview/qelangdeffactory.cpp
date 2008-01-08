#include <QMessageBox>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "qelangdeffactory.h"
#include "qegtklangdef.h"

#include "debug_info.h"

QeLangDefFactory *QeLangDefFactory::LangFactory = NULL;


// public....
QeLangDefFactory *QeLangDefFactory::getInstanse()
{
	if (LangFactory == NULL)
	{
		LangFactory = new QeLangDefFactory();
	}

	return LangFactory;
}

QeGtkSourceViewLangDef *QeLangDefFactory::getHighlight( QString fileName )
{
	QeGtkSourceViewLangDef *langDef;
	QString langMimeType;

	foreach( langDef, langList )
	{
		foreach( langMimeType, langDef->getMimeTypes() )
		{
			// if do we recognize the mime type defined in that
			// syntax highligh definition, check if matches this file

			//if ( mimeTypes.find(langMimeType) )
			if ( mimeTypes.contains(langMimeType) )
			{
				for ( int j=0; j<mimeTypes[langMimeType].count(); j ++ ) 
				{
					QString s = "*." + mimeTypes[langMimeType][j];
					if (QDir::match( s, fileName) || QDir::match( mimeTypes[langMimeType][j], fileName))

// 					QString s = "*" + mimeTypes[langMimeType][j];
// 					if (QDir::match( s, fileName))
					{
#ifdef __DEBUG_FOUND_LANG_DEF__	
						qDebug( "QeLangDefFactory::getHighlight(): Found language definition %s [ %s,%s]", qPrintable(langDef->getName()),
							qPrintable(fileName), qPrintable( "*" + mimeTypes[langMimeType][j] )
						);
#endif						
						return langDef;
					}
				}
			}
			else
			{
				qDebug("Unknown mimetype [%s] at highlight file %s", 
					qPrintable(langMimeType), qPrintable(langDef->getName()) );
			}
		}
	}

	qDebug( "Not found any highlighter for [%s]", qPrintable(fileName) );
	return NULL;
}

void QeLangDefFactory::loadDirectory( QString directory )
{
	if (directory.isEmpty())
		directory = QDir::currentPath();
	QDir dir(directory, "*.lang");

	QStringList files = dir.entryList(QDir::Files | QDir::NoSymLinks);
	int fileCount =	files.count();

	if (fileCount == 0)
	{
		qDebug( "Error: no highlight definitions found at directory: %s", qPrintable(directory) );
		return;
	}

	for (int i = 0; i < fileCount; ++i)
	{
		QeGtkSourceViewLangDef *langDef = new QeGtkSourceViewLangDef ( directory + "/" + files[i] );
		langList << langDef;
		QString langMimeType;

#ifdef __DEBUG_LANGS_MIMES__
		foreach( langMimeType, langDef->getMimeTypes() )
		{
			if ( mimeTypes.find(langMimeType) == mimeTypes.end() )
			{
 				qDebug("Warning: highlight file %s - unknown mimetype [%s]", qPrintable(langDef->getName()), qPrintable(langMimeType) );
			}
		}
#endif		
	}
}

// private...
QeLangDefFactory::QeLangDefFactory(void)
{
	// TODO: fix the code to use
	// /usr/share/mime/globs
	//


	// load mime types
	QFile file( ":/mime.types" );

	file.open(QIODevice::ReadOnly | QIODevice::Text);
	
	// parse built in mime types definitions
	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		
		if (line.startsWith("#"))
			continue;

		QStringList l = line.split( QRegExp("\\s+") );
		QString     name = l[0];
		l.removeAt( 0 );

		if (!l.empty())
		{
// 			QString s;
// 			for ( int j=0; j<l.count(); j ++ ) s = s + "*." + l[j] + ",";
// 			qDebug( "%s -> %s", qPrintable(name), qPrintable(s) );		
			mimeTypes[name] = l;
		}
	}
	file.close();
}

QeLangDefFactory::~QeLangDefFactory(void)
{
}
