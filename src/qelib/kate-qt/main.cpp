#include <QtDebug>
#include "katehighlight.h"
#include "katelanguage.h"

int main( int argc, char *argv)
{
 	kateLanguage l("c.xml");
	qDebug() << "Name: "    << l.attributes["name"];
	qDebug() << "Version: " << l.attributes["version"];
	qDebug() << "Author: "    << l.attributes["author"];
	return 0;
}
