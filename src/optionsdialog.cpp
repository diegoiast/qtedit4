#include <QFontDialog>

#include "optionsdialog.h"

/**
 * \file    optionsdialog.cpp
 * \brief   Implementation of the options dialog class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */
 
OptionsDialog::OptionsDialog( QWidget *owner ):
	QDialog( owner )
{
	optionsDialogUI.setupUi( this );
	connect( optionsDialogUI.btnChooseFont, SIGNAL(clicked()), this, SLOT(chooseFont()) );

// 	codecList = 0;

//     // this is the only one which MUST exist
//     { QString("System default"),     QString("") },
// 
//     // all others are optional and can be removed, the order is not important as well
//     { QString("US English"),         QString("ISO 8859-1") },
//     { QString("UTF8"),               QString("UTF-8") },
//     { QString("Western European"),   QString("ISO 8859-1") },
//     { QString("Central European"),   QString("ISO 8859-2") },
//     { QString("Southern European"),  QString("ISO 8859-3") },
//     { QString("Baltic"),             QString("ISO 8859-4") },
//     { QString("Nordic"),             QString("ISO 8859-14") },
//     { QString("Celtic"),             QString("ISO 8859-10") },
//     { QString("Turkish"),            QString("ISO 8859-9") },
//     { QString("Vietnamese"),         QString("CP 1258") },
//     { QString("Cryllic"),            QString("ISO 8859-5") },
//     { QString("Greek"),              QString("ISO 8859-7") },
//     { QString("Arabic"),             QString("ISO 8859-6") },
//     { QString("Hebrew"),             QString("ISO 8859-8-I") },
// //    { QString("Tahi"),               QString("TIS620") },
//     { QString("Chinese"),            QString("Big5") },
//     { QString("Japanese"),           QString("eucJP") },
//     { QString("Korean"),             QString("eucKR") },
//     { QString(""),                   QString("") }
// };

}

OptionsDialog::~OptionsDialog()
{
}

void OptionsDialog::chooseFont()
{
	bool ok;
	
	QFont f =
		QFontDialog::getFont( &ok,  optionsDialogUI.labelFontName->font() );

	if (ok)
	{
		optionsDialogUI.labelFontName->setText( f.toString() );
		optionsDialogUI.labelFontName->setFont( f );
	}
}
