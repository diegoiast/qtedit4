#ifndef __OPTIONS_DIALOG_H__
#define __OPTIONS_DIALOG_H__

#include <QWidget>
#include <QDialog>
#include <QFont>
#include <QList>

#include "ui_optionsdialog.h"

/**
 * \file    optionsdialog.cpp
 * \brief   Declaration of the options dialog class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

struct MyCodecListEntry{
    QString CodecName;
    QString Encoding;
};

class OptionsDialog: public QDialog {
Q_OBJECT
public:
	OptionsDialog( QWidget *owner );
	~OptionsDialog();

public slots:
	void chooseFont();
	
private:
	Ui::OptionsDialog optionsDialogUI;
	QFont editorFont;
};

#endif // __OPTIONS_DIALOG_H__
