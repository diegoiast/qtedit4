#include "qsvdefaulthighlighter.h"

DefaultHighlighter::DefaultHighlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
	setMatchBracketList("()[]''\"\"");
}

void DefaultHighlighter::highlightBlock(const QString &text)
{
	QsvSyntaxHighlighterBase::highlightBlock(text);
}

void DefaultHighlighter::toggleBookmark(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->toggleBookmark();
}

void DefaultHighlighter::removeModification(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->m_isModified = false;
}

void DefaultHighlighter::setBlockModified(QTextBlock &block, bool on)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->m_isModified = on;
}

bool DefaultHighlighter::isBlockModified(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return false;
	return data->m_isModified;
}

bool DefaultHighlighter::isBlockBookmarked(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return false;
	return data->isBookmark();
}

Qate::BlockData::LineFlags DefaultHighlighter::getBlockFlags(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return Qate::BlockData::LineFlag::Empty;
	return data->m_flags;
}

void DefaultHighlighter::clearMatchData(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->matches.clear();
}

void DefaultHighlighter::addMatchData(QTextBlock &block, Qate::MatchData m)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->matches << m;
}

QList<Qate::MatchData> DefaultHighlighter::getMatches(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return QList<Qate::MatchData>();
	return data->matches;
}

QTextBlock DefaultHighlighter::getCurrentBlockProxy()
{
	return currentBlock();
}

Qate::BlockData *DefaultHighlighter::getBlockData(QTextBlock &block)
{
	QTextBlockUserData *userData  = block.userData();
	Qate::BlockData       *blockData = nullptr;

	if (userData == nullptr) {
		blockData =  new Qate::BlockData();
		block.setUserData(blockData);
	} else {
		blockData = dynamic_cast<Qate::BlockData*>(userData);
	}
	return blockData;
}
