/**
 * \file texteditor_plg
 * \brief Definition of 
 * \author Diego Iastrubni elcuco@kde.org
 * License GPL 2008
 * \see class name
 */

#include "iplugin.h"

#include "qate/mimedatabase.h"
#include "qate/highlightdefinitionmanager.h"

class TextEditorPlugin: public IPlugin
{
	Q_OBJECT
public:
	TextEditorPlugin();
	~TextEditorPlugin();

	void		showAbout();
	QWidget*	getConfigDialog();
	QActionGroup*	newFileActions();
	QStringList	myExtensions();
	int		canOpenFile( const QString fileName );
	bool		openFile( const QString fileName, int x=-1, int y=-1, int z=-1 );
	void		getData();
	void		setData();

	QString findDefinitionId(const Qate::MimeType &mimeType, bool considerParents) const;
	Qate::MimeType getMimeByExt(const QString &fileName) const;
	public slots:
	void fileNew( QAction * );

private:
	QActionGroup *myNewActions;
	QAction		*actionNewFile;
	QAction		*actionNewCPP;
	QAction		*actionNewHeader;

	Qate::MimeDatabase                                        *mimes;
	Qate::HighlightDefinitionManager                          *hl_manager;
};

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 7; mixedindent off; indent-mode cstyle; 
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
