#ifndef __HELP_DISPLAY_H__
#define __HELP_DISPLAY_H__

class QString;
class QTextBrowser;
//class TextDisplay;

#include "textdisplay.h"


class HelpDisplay: public TextDisplay
{
	Q_OBJECT
public:
	HelpDisplay( QString helpSource, QWidget *parent=NULL );
	~HelpDisplay();
	
public slots:
	void goHome();

private:
	QTextBrowser *helpBrowser;
	QString homeURL;
	QAction *actionBackward;
	QAction *actionForward;
	QAction *actionGoHome;
	QAction *actionZoomIn;
	QAction *actionZoomOut;
};


#endif // __HELP_DISPLAY_H__
