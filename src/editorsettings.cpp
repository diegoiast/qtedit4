//
// C++ Implementation: editorsettings
//
// Description:
//
//
// Author: Diego Iastrubni <elcuco@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "editorsettings.h"

EditorSettings* EditorSettings::editorSettings = NULL;

EditorSettings::EditorSettings()
{
	showLineNumbers	=	true;
	lineWrap	=	false;
	markCurrentLine	=	true;

	font.setFamily("Courier New");
	font.setPointSize(10);
	font.setFixedPitch(true);
}

EditorSettings::~EditorSettings()
{
	// clear the singleton
	delete editorSettings;
}

EditorSettings* EditorSettings::getInstance()
{
	if (editorSettings == NULL)
		editorSettings = new EditorSettings();

	return editorSettings;
}

void 	EditorSettings::announceChange()
{
	emit updateSettings();
}
