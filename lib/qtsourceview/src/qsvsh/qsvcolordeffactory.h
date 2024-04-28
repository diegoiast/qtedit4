#pragma once

/**
 * \file qsvcolordeffactory.h
 * \brief Definition of the color defintion factory
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvColorDefFactory
 */

#include <QString>
#include <QList>

class QDomDocument;
class QsvColorDef;

class QsvColorDefFactory
{
public:
	QsvColorDefFactory();
	QsvColorDefFactory( QDomDocument doc );
	QsvColorDefFactory( QString fileName );
	~QsvColorDefFactory();

	bool load( QDomDocument doc );
	bool load( QString fileName );
	bool isValid() const;
	QsvColorDef getColorDef( QString name ) const;
public:
	QList<QsvColorDef> colorDefs;
	QString fileName;
	QString name;
	QString description;
	QString version;
	QString author;
};
