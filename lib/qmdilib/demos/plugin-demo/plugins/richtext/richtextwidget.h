#pragma once

/**
 * \file richtext.h
 * \brief Definition of the RichText class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see QexRichTextBrowser
 * \see RichText
 */

#include <QTextListFormat>
#include <QWidget>

class QTextEdit;
class QTabWidget;
class QTextCharFormat;
class QFont;
class QColor;
class QActionGroup;
class QTextList;
class QFontComboBox;
class QComboBox;
class QVBoxLayout;

class RichTextWidget : public QWidget {
    Q_OBJECT
  public:
    RichTextWidget(QWidget *parent, QString fileName = QString());
    RichTextWidget(QWidget *parent, QTextEdit *e);
    RichTextWidget(QWidget *parent, QString fileName, QTextEdit *e);
    ~RichTextWidget();
    QString getFileName();

  public slots:
    void initWidget(QString fileName, QTextEdit *e);
    void loadFile(QString fileName);
    void setMSWordShortCuts();

    void markBold();
    void markItalic();
    void markUnderline();

    void setList(QTextListFormat::Style type);
    QTextList *getList();

    void setShowSource(bool shouldShow);
    bool getShowSource();

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

  private slots:
    void on_tabWidget_currentChanged(int index);
    void on_richText_Modified();
    void alignmentChanged(Qt::Alignment a);
    void directionChanged(Qt::LayoutDirection d);
    void cursorPositionChanged();
    void currentCharFormatChanged(const QTextCharFormat &format);
    void textAlign(QAction *a);
    void textDirection(QAction *a);
    void setList_(QAction *a);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void textFamily(const QString &f);
    void textColor();
    void textSize(QString p);

  protected:
    // 	bool event ( QEvent * event );
    void timerEvent(QTimerEvent *event);

  public:
    QAction *actionRTL;
    QAction *actionLTR;
    QActionGroup *directionGroup;

    QAction *actionBold;
    QAction *actionItalic;
    QAction *actionUnderline;

    QAction *actionAlignRight;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignJustify;
    QActionGroup *alignGroup;

    QAction *actionListDisc;
    QAction *actionListCircle;
    QAction *actionListSquare;
    QAction *actionListDecmial;
    QAction *actionListLowerAlpha;
    QAction *actionListUpperAlpha;
    QActionGroup *listGroup;

    QAction *actionTextColor;
    QFontComboBox *fontComboBox;
    QComboBox *comboSize;

  private:
    QVBoxLayout *myLayout;
    QTextEdit *textEdit;
    QTextEdit *richText;
    QTabWidget *tabWidget;
    QString fileName;

    bool dirtyFlag;
};
