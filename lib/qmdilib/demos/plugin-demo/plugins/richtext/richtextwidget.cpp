/**
 * \file richtext.cpp
 * \brief Implementation of the RichText class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see RichText
 */

// $Id: pluginmanager.h 146 2007-04-23 22:45:01Z elcuco $

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QFile>
#include <QFontComboBox>
#include <QKeySequence>
#include <QLayout>
#include <QPixmap>
#include <QTabWidget>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTextList>
#include <QTextStream>
#include <QVBoxLayout>

#include "richtextwidget.h"

RichTextWidget::RichTextWidget(QWidget *parent, QString fileName) : QWidget(parent) {
    initWidget(fileName, NULL);
}

RichTextWidget::RichTextWidget(QWidget *parent, QTextEdit *e) : QWidget(parent) {
    initWidget(QString(), e);
}

RichTextWidget::RichTextWidget(QWidget *parent, QString fileName, QTextEdit *e) : QWidget(parent) {
    initWidget(fileName, e);
}

RichTextWidget::~RichTextWidget() {}

QString RichTextWidget::getFileName() { return fileName; }

void RichTextWidget::initWidget(QString fileName, QTextEdit *e) {
    this->setObjectName("RichTextWidget (container)");
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("RichTextWidget::tabWidget");

    // create the basic GUI
    richText = new QTextEdit(tabWidget);
    richText->setAcceptRichText(true);
    richText->setFrameStyle(QFrame::NoFrame);
    richText->setObjectName("RichTextWidget::RichTextWidget");

    textEdit = (e == NULL) ? new QTextEdit(tabWidget) : e;
    textEdit->setAcceptRichText(false);
    textEdit->setFrameStyle(QFrame::NoFrame);
    textEdit->setObjectName("RichTextWidget::textEdit");

    tabWidget->setTabShape(QTabWidget::Triangular);
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->addTab(richText, tr("Rich text"));
    tabWidget->addTab(textEdit, tr("HTML"));

    QLayout *myLayout = new QVBoxLayout(this);
    myLayout->addWidget(tabWidget);
    myLayout->setContentsMargins(0, 0, 0, 0);
    myLayout->setSpacing(0);
    myLayout->setObjectName("RichTextWidget::myLayout");
    setLayout(myLayout);

    // create the actions
    actionBold = new QAction("B", this);
    actionItalic = new QAction("I", this);
    actionUnderline = new QAction("U", this);

    alignGroup = new QActionGroup(this);
    actionAlignLeft = new QAction("L", alignGroup);
    actionAlignCenter = new QAction("C", alignGroup);
    actionAlignJustify = new QAction("J", alignGroup);
    actionAlignRight = new QAction("R", alignGroup);

    directionGroup = new QActionGroup(this);
    actionLTR = new QAction("LTR", directionGroup);
    actionRTL = new QAction("RTL", directionGroup);

    listGroup = new QActionGroup(this);
    actionListDisc = new QAction("*)", listGroup);
    actionListCircle = new QAction("()", listGroup);
    actionListSquare = new QAction("[]", listGroup);
    actionListDecmial = new QAction("1)", listGroup);
    actionListLowerAlpha = new QAction("a)", listGroup);
    actionListUpperAlpha = new QAction("A)", listGroup);

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));

    fontComboBox = new QFontComboBox;
    fontComboBox->setObjectName("Choose font family");

    comboSize = new QComboBox;
    QFontDatabase db;
    comboSize->setObjectName("Choose font size");
    comboSize->setEditable(true);
    foreach (int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    actionLTR->setCheckable(true);
    actionRTL->setCheckable(true);

    actionBold->setCheckable(true);
    actionItalic->setCheckable(true);
    actionUnderline->setCheckable(true);
    actionAlignRight->setCheckable(true);
    actionAlignLeft->setCheckable(true);
    actionAlignCenter->setCheckable(true);
    actionAlignJustify->setCheckable(true);

    actionListDisc->setCheckable(true);
    actionListCircle->setCheckable(true);
    actionListSquare->setCheckable(true);
    actionListDecmial->setCheckable(true);
    actionListLowerAlpha->setCheckable(true);
    actionListUpperAlpha->setCheckable(true);

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(on_tabWidget_currentChanged(int)));
    connect(richText, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this,
            SLOT(currentCharFormatChanged(const QTextCharFormat &)));
    connect(richText, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
    connect(richText, SIGNAL(textChanged()), this, SLOT(on_richText_Modified()));

    connect(actionBold, SIGNAL(triggered()), this, SLOT(markBold()));
    connect(actionItalic, SIGNAL(triggered()), this, SLOT(markItalic()));
    connect(actionUnderline, SIGNAL(triggered()), this, SLOT(markUnderline()));
    connect(alignGroup, SIGNAL(triggered(QAction *)), this, SLOT(textAlign(QAction *)));
    connect(directionGroup, SIGNAL(triggered(QAction *)), this, SLOT(textDirection(QAction *)));
    connect(listGroup, SIGNAL(triggered(QAction *)), this, SLOT(setList_(QAction *)));
    connect(fontComboBox, SIGNAL(activated(const QString &)), this,
            SLOT(textFamily(const QString &)));
    connect(comboSize, SIGNAL(activated(const QString &)), this, SLOT(textSize(const QString &)));

    setMSWordShortCuts();

    if (!fileName.isEmpty())
        loadFile(fileName);
}

void RichTextWidget::loadFile(QString fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString content;
    QTextStream in(&file);
    while (!in.atEnd())
        content += in.readLine() + "\n";

    textEdit->setPlainText(content);
    richText->setHtml(content);
    dirtyFlag = false;
    this->fileName = fileName;
}

void RichTextWidget::setMSWordShortCuts() {
    actionBold->setShortcut(QKeySequence("Ctrl+B"));
    actionItalic->setShortcut(QKeySequence("Ctrl+I"));
    actionUnderline->setShortcut(QKeySequence("Ctrl+U"));

    actionAlignRight->setShortcut(QKeySequence("Ctrl+R"));
    actionAlignLeft->setShortcut(QKeySequence("Ctrl+L"));
    actionAlignCenter->setShortcut(QKeySequence("Ctrl+E"));
    actionAlignJustify->setShortcut(QKeySequence());
}

void RichTextWidget::markBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(actionBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::markItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(actionItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::markUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::setList(QTextListFormat::Style type) {
    QTextCursor cursor = richText->textCursor();

    if (type != 0) {
        cursor.beginEditBlock();
        QTextBlockFormat blockFmt = cursor.blockFormat();
        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(type);
        cursor.createList(listFmt);
        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

QTextList *RichTextWidget::getList() {
    QTextCursor cursor = richText->textCursor();

    return cursor.currentList();
}

void RichTextWidget::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
    QTextCursor cursor = richText->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    richText->mergeCurrentCharFormat(format);
}

void RichTextWidget::cursorPositionChanged() {
    alignmentChanged(richText->alignment());
    directionChanged(richText->textCursor().blockFormat().layoutDirection());

    QTextList *l = getList();
    if (l == NULL) {
        actionListDisc->setChecked(false);
        actionListCircle->setChecked(false);
        actionListSquare->setChecked(false);
        actionListDecmial->setChecked(false);
        actionListLowerAlpha->setChecked(false);
        actionListUpperAlpha->setChecked(false);
    } else {
        QTextListFormat::Style style = l->format().style();

        if (style == QTextListFormat::ListDisc)
            actionListDisc->setChecked(true);
        else if (style == QTextListFormat::ListCircle)
            actionListCircle->setChecked(true);
        else if (style == QTextListFormat::ListSquare)
            actionListSquare->setChecked(true);
        else if (style == QTextListFormat::ListDecimal)
            actionListDecmial->setChecked(true);
        else if (style == QTextListFormat::ListLowerAlpha)
            actionListLowerAlpha->setChecked(true);
        else if (style == QTextListFormat::ListUpperAlpha)
            actionListUpperAlpha->setChecked(true);
        else {
            actionListDisc->setChecked(false);
            actionListCircle->setChecked(false);
            actionListSquare->setChecked(false);
            actionListDecmial->setChecked(false);
            actionListLowerAlpha->setChecked(false);
            actionListUpperAlpha->setChecked(false);
        }
    }
}

void RichTextWidget::on_tabWidget_currentChanged(int index) {
    if (index == 1) {
        // moved from the RichTextWidget to the textEdit
        if (dirtyFlag) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            // QTextEdit just messes up the makrup:
            // if the user is editing the raw html, and moves to the
            // html tab to preview, dont rewrite the html code
            textEdit->setPlainText(richText->toHtml());
            QApplication::restoreOverrideCursor();
        } else {
            // stub block
        }
    } else {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        // moved from the textEdit to the RichTextWidget
        richText->setHtml(textEdit->toPlainText());
        QApplication::restoreOverrideCursor();
    }

    dirtyFlag = false;

    actionBold->setEnabled(index == 0);
    actionItalic->setEnabled(index == 0);
    actionUnderline->setEnabled(index == 0);
    listGroup->setEnabled(index == 0);
    alignGroup->setEnabled(index == 0);
    fontComboBox->setEnabled(index == 0);
    comboSize->setEnabled(index == 0);
    actionTextColor->setEnabled(index == 0);
}

void RichTextWidget::on_richText_Modified() { dirtyFlag = true; }

void RichTextWidget::alignmentChanged(Qt::Alignment a) {
    if (a & Qt::AlignLeft) {
        actionAlignLeft->setChecked(true);
    } else if (a & Qt::AlignHCenter) {
        actionAlignCenter->setChecked(true);
    } else if (a & Qt::AlignRight) {
        actionAlignRight->setChecked(true);
    } else if (a & Qt::AlignJustify) {
        actionAlignJustify->setChecked(true);
    }
}

void RichTextWidget::directionChanged(Qt::LayoutDirection d) {
    if (d == Qt::RightToLeft)
        actionRTL->setChecked(true);
    else if (d == Qt::LeftToRight)
        actionLTR->setChecked(true);
    // WTF?
    else {
        actionRTL->setChecked(false);
        actionLTR->setChecked(false);
    }
}

void RichTextWidget::setShowSource(bool shouldShow) {
    // 	tabWidget->tabBar()->setVisible( shouldShow );
    if (!shouldShow)
        tabWidget->setCurrentIndex(0);
}

bool RichTextWidget::getShowSource() {
    // TODO
    return false;
}

void RichTextWidget::currentCharFormatChanged(const QTextCharFormat &format) {
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void RichTextWidget::textDirection(QAction *a) {
    QTextCursor qtc = richText->textCursor();
    QTextBlockFormat qtbf = qtc.blockFormat();
    Qt::Alignment qta = richText->alignment();

    if (a == actionLTR) {
        qtbf.setLayoutDirection(Qt::LeftToRight);
        qtc.setBlockFormat(qtbf);

        if (qta == Qt::AlignRight)
            qta = Qt::AlignLeft;
        else if (qta == Qt::AlignLeft)
            qta = Qt::AlignRight;
    } else if (a == actionRTL) {
        qtbf.setLayoutDirection(Qt::RightToLeft);
        qtc.setBlockFormat(qtbf);

        if (qta == Qt::AlignRight)
            qta = Qt::AlignLeft;
        else if (richText->alignment() == Qt::AlignLeft)
            qta = Qt::AlignRight;
    }
    //	else WTF?

    richText->setTextCursor(qtc);
    richText->setAlignment(qta);
}

void RichTextWidget::textAlign(QAction *a) {
    if (a == actionAlignLeft)
        richText->setAlignment(Qt::AlignLeft);
    else if (a == actionAlignCenter)
        richText->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        richText->setAlignment(Qt::AlignRight);
    else if (a == actionAlignJustify)
        richText->setAlignment(Qt::AlignJustify);
}

void RichTextWidget::setList_(QAction *a) {
    if (a == actionListDisc)
        setList(QTextListFormat::ListDisc);
    else if (a == actionListCircle)
        setList(QTextListFormat::ListCircle);
    else if (a == actionListSquare)
        setList(QTextListFormat::ListSquare);
    else if (a == actionListDecmial)
        setList(QTextListFormat::ListDecimal);
    else if (a == actionListLowerAlpha)
        setList(QTextListFormat::ListLowerAlpha);
    else if (a == actionListUpperAlpha)
        setList(QTextListFormat::ListUpperAlpha);
}

void RichTextWidget::fontChanged(const QFont &f) {
    fontComboBox->setCurrentIndex(fontComboBox->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionBold->setChecked(f.bold());
    actionItalic->setChecked(f.italic());
    actionUnderline->setChecked(f.underline());
}

void RichTextWidget::colorChanged(const QColor &c) {
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void RichTextWidget::textColor() {
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void RichTextWidget::textSize(QString p) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(p.toFloat());
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::textFamily(const QString &f) {
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

/*virtual */
/*
bool RichTextWidget::event ( QEvent * event )
{
        if (event->type() == QEvent::KeyPress)
        {
                setCursor(Qt::IBeamCursor);
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
                setCursor(Qt::IBeamCursor);
        }
        return false;
}
*/
void RichTextWidget::timerEvent(QTimerEvent *event) { Q_UNUSED(event); }
