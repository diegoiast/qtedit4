#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include <qobject.h>
#include <QFont>

class QFont;

/**
*/
class EditorSettings : public QObject
{
	Q_OBJECT
private:
	EditorSettings();
	~EditorSettings();
	static EditorSettings *editorSettings;

signals:
	void updateSettings();

public:
	static	EditorSettings *getInstance();
	void 	announceChange();
	
	bool	showLineNumbers;
	bool	lineWrap;
	bool	markCurrentLine;

	QFont font;
};

#endif
