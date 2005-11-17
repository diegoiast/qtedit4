#ifndef __KATE_HIGHLIGHT_RULE__
#define __KATE_HIGHLIGHT_RULE__

#include <QDomNode>
#include <QString>

#include "kateqtglobal.h"

// highlight rules
enum KateHighlightRuleTypes
{
	DetectChar,
	Detect2Chars,
	AnyChar,
	StringDetect,
	RegExp,
	keyword,
	Int,
	Float,
	HICOct,
	HlCStringChar,
	HlCChar,
	RangeDetect,
	LineContinue,
	IncludeRules,
	DetectSpaces,
	DetectIdentifier
};

// TODO: contexts can have AnyChar or StringDetect childs
class kateHighlightRule
{
public:
	kateHighlightRule();
	kateHighlightRule( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
	int getTypeFromName( QString name );
private:
	QString name;
	int type;
	QStringMap attributes;
};

#endif // __KATE_HIGHLIGHT_RULE__
