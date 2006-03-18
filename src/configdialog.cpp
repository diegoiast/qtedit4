#include <QFontDialog>

#include "editorsettings.h"
#include "configdialog.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent):QDialog(parent)
{
	setupUi( this );
	updateWidgetsFromSettings();
}

void ConfigurationDialog::updateWidgetsFromSettings()
{
	EditorSettings *editSettings = EditorSettings::getInstance();

	cbLineNumbers->setChecked( editSettings->showLineNumbers );
	cbWordWrap->setChecked( editSettings->lineWrap );
	cbMarkCurrentLine->setChecked( editSettings->markCurrentLine );

	fontPreview->setText( editSettings->font.toString() );
	fontPreview->setFont( editSettings->font );
	fontPreview->setCursorPosition( 0 );
}

void ConfigurationDialog::on_applyButton_clicked()
{
	EditorSettings *editSettings = EditorSettings::getInstance();

	editSettings->showLineNumbers	= cbLineNumbers->isChecked();
	editSettings->markCurrentLine	= cbMarkCurrentLine->isChecked();
	editSettings->lineWrap		= cbWordWrap->isChecked();
	editSettings->font		= fontPreview->font();

	editSettings->announceChange();
}

void ConfigurationDialog::on_buttonChooseFont_clicked()
{
	EditorSettings *editSettings = EditorSettings::getInstance();
	bool ok;
	QFont font = QFontDialog::getFont(&ok, editSettings->font, this);
	if (!ok)
		return;

	fontPreview->setText( font.toString() );
	fontPreview->setFont( font );
	fontPreview->setCursorPosition( 0 );
}
