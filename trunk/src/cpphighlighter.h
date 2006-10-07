/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qdevelop.org
*
*/
#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>

class QTextDocument;

class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
	CppHighlighter(QTextDocument *parent = 0);
	QTextCharFormat keywordFormat() { return m_keywordFormat;};
	void setKeywordFormat (QTextCharFormat f) { m_keywordFormat = f;};
	QTextCharFormat classFormat() { return m_classFormat;};
	void setClassFormat (QTextCharFormat f) { m_classFormat = f;};
	QTextCharFormat singleLineCommentFormat() { return m_singleLineCommentFormat;};
	void setSingleLineCommentFormat (QTextCharFormat f) { m_singleLineCommentFormat = f;};
	QTextCharFormat multiLineCommentFormat() { return m_multiLineCommentFormat;};
	void setMultiLineCommentFormat (QTextCharFormat f) { m_multiLineCommentFormat = f;};
	QTextCharFormat quotationFormat() { return m_quotationFormat;};
	void setQuotationFormat (QTextCharFormat f) { m_quotationFormat = f;};
	QTextCharFormat functionFormat() { return m_functionFormat;};
	void setFunctionFormat (QTextCharFormat f) { m_functionFormat = f;};
	QTextCharFormat preprocessorFormat() { return m_preprocessorFormat;};
	void setPreprocessorFormat (QTextCharFormat f) { m_preprocessorFormat = f;};

protected:
	void highlightBlock(const QString &t);

private:
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_classFormat;
    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
    QTextCharFormat m_quotationFormat;
    QTextCharFormat m_functionFormat;
    QTextCharFormat m_preprocessorFormat;
    QStringList keywordPatterns;
    //
	bool inQuotations(int position, QString text);
};

#endif
