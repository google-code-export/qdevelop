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
#include <QtGui>
#include <QCursor>

#include "cpphighlighter.h"

CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_functionFormat.setForeground(Qt::black);
    m_keywordFormat.setForeground(Qt::blue);
    m_preprocessorFormat.setForeground(Qt::blue);
    m_classFormat.setForeground(Qt::blue);
    m_singleLineCommentFormat.setForeground(Qt::red);
    m_multiLineCommentFormat.setForeground(Qt::red);
    m_quotationFormat.setForeground(Qt::darkGreen);
    //
    keywordPatterns 
	<< "bool" << "break" << "case" << "catch" << "char" << "class"
	<< "compl" << "const" << "const_cast" << "continue" << "default" << "delete"
	<< "do" << "double" << "dynamic_cast" << "else"
	<< "export" << "rxtern" << "false" << "float" << "for" << "friend"
	<< "goto" << "if" << "inline" << "int" << "long" 
	<< "namespace" << "new"<< "operator" << "or"
	<< "private" << "protected" << "public" << "register" << "reinterpret_cast"
	<< "return" << "short" << "signed" << "sizeof" << "static" << "static_cast"
	<< "struct" << "switch" << "template" << "this" << "throw" << "true"
	<< "try" << "typedef" << "typeid" << "typename" << "union" << "unsigned"
	<< "using" << "virtual" << "void" << "volatile"<< "while"
	<< "xor" << "xor_eq" 
	<< "foreach" << "qint8" << "qint16" << "qint32" << "qint64" << "qlonglong" << "qreal" << "quint8"
	<< "quint16" << "quint32" << "quint64" << "qulonglong" << "uchar" << "uint" << "ulong" << "ushort"
	<< "qAbs" << "qBound" << "qCritical" << "qDebug" << "qFatal" << "qInstallMsgHandler" << "qMacVersion"
	<< "qMax" << "qMin" << "qRound64" << "qRound" << "qVersion" << "qWarning" << "qPrintable"
	<< "SLOT" << "SIGNAL" << "signals" << "slots" << "connect" << "disconnect"
	<< "and" << "and_eq" << "asm" << "auto" << "bitand" << "bitor" << "mutable" << "not" << "not_eq" 
	<< "or_eq" << "or_eq"  << "wchar_t";
}
//
void CppHighlighter::highlightBlock(const QString &t)
{
	enum { Undefined=-1, Closed=0, Opened};
	QRegExp commentStartExpression = QRegExp("/\\*");
	QRegExp commentEndExpression = QRegExp("\\*/");
	QString text = t;
//qDebug()<<text;
	setCurrentBlockState(Closed);

	// Comments between /* and */ 
	int startIndex = 0, endIndex = -1;
	int commentLength = 0;
	do
	{
		if (previousBlockState() != Opened || endIndex != -1)
		{
			startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
		}
		endIndex = -1;
		if (startIndex >= 0) 
		{
			endIndex = text.indexOf(commentEndExpression, startIndex);
			if (endIndex == -1) 
			{
				setCurrentBlockState(Opened);
				commentLength = text.length() - startIndex;
			} 
			else 
			{
				setCurrentBlockState(Closed);
				commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
			}
			if( !inQuotations(startIndex, text) && !inQuotations(startIndex+commentLength, text) )
			{
				setFormat(startIndex, commentLength, m_multiLineCommentFormat);
				for(int i = startIndex; i<startIndex+commentLength; i++)
					text[i] = QChar(255);
			}
			else
				setCurrentBlockState(Closed);
		}
		else
		{
			endIndex = -1;
			setCurrentBlockState(Closed);
		}
	} while( endIndex != -1 );
	if( currentBlockState() == Opened )
		return;
		
	// Simple quotation
	int debutQuote = 0, finQuote;
	bool realBegin = false;
	do
	{
		do
		{

			debutQuote = text.indexOf("\"", debutQuote); 
			if( debutQuote > 0 && text.at(debutQuote-1) == '\\' && text.at(debutQuote-1) == '\'' 
				&& (debutQuote > 1 && text.at(debutQuote-2) != '\\') )
			{
				realBegin = false;
			}
			else
			{
				realBegin = true;
			}
			debutQuote++;
		} while(!realBegin);
		
		finQuote = -1;
		if( debutQuote )
		{
			finQuote = debutQuote;
			bool realEnd = false;
			do
			{
				finQuote = text.indexOf("\"", finQuote); 
				if( finQuote > 0 && text.at(finQuote-1) == '\\' 
					&& (finQuote > 1 && text.at(finQuote-2) != '\\') )
				{
					finQuote++;
					realEnd = false;
				}
				else 
				{
					if( finQuote != -1 )
						finQuote--;
					realEnd = true;
				}
			} while(!realEnd);
		}
		if( debutQuote!=-1 && finQuote!=-1 )
		{
			setFormat(debutQuote, (finQuote+1)-debutQuote, m_quotationFormat);
			for(int i=debutQuote; i<finQuote+1; i++)
				text[i] = QChar(255);
			debutQuote = finQuote+2;
		}
	} while( debutQuote!=-1 && finQuote!=-1 );
	
	// Single line comment
	QRegExp single("//[^\n]*");
	int indexSingle = text.indexOf(single);
	int length = single.matchedLength();
	if( indexSingle != -1 )
	{
		setFormat(indexSingle, length, m_singleLineCommentFormat);
		for(int i=indexSingle; i<indexSingle+length; i++)
			text[i] = QChar(255);
	}
	// #
	int indexSharp = text.indexOf("#");
	int indexEndSharp = text.lastIndexOf('"', indexSharp);
	if ( indexEndSharp == -1 )
		indexEndSharp = text.indexOf(QChar(255), indexSharp);
	if ( indexEndSharp == -1 )
		indexEndSharp = text.length();
	if( indexSharp != -1 )
	{
		setFormat(indexSharp, indexEndSharp-indexSharp, m_preprocessorFormat);
		for(int i=indexSharp; i<indexEndSharp; i++)
			text[i] = QChar(255);
	}
	// Words
	QRegExp expression( "\\b(\\w+)\\b" );
	int index = -1;
	do
	{
		index = text.indexOf(expression, index+1);
	} while( index >= 0 && text.at( index ) == QChar(255) );
	while (index >= 0) 
	{
		//if( (index < debutQuote && index > finQuote) || debutQuote==-1 || finQuote==-1 )
		//{
			length = expression.matchedLength();
			QString word = expression.cap();
			if( word.at(0) == 'Q' )
				setFormat(index, length, m_classFormat);
			else if( keywordPatterns.contains( word ) )
					setFormat(index, length, m_keywordFormat);
			else if( index+length+1<text.length() && text.mid(index+length).simplified().count() && text.mid(index+length).simplified().at(0) == '(' )
				setFormat(index, length, m_functionFormat);
		//}
		index = text.indexOf(expression, index + length);
	}
}
//
bool CppHighlighter::inQuotations(int position, QString text)
{
	int debutQuote = 0, finQuote;
	bool realBegin = false;
	do
	{
		do
		{

			debutQuote = text.indexOf("\"", debutQuote); 
			if( debutQuote > 0 && text.at(debutQuote-1) == '\\' && text.at(debutQuote-1) == '\'' )
			{
				realBegin = false;
			}
			else
			{
				realBegin = true;
			}
		} while(!realBegin);
		
		finQuote = -1;
		if( debutQuote != -1 )
		{
			finQuote = debutQuote+1;
			bool realEnd = false;
			do
			{
				finQuote = text.indexOf("\"", finQuote); 
				if( finQuote > 0 && text.at(finQuote-1) == '\\' )
				{
					finQuote++;
					realEnd = false;
				}
				else 
				{
					realEnd = true;
				}
			} while(!realEnd);
		}
		if( debutQuote!=-1 && finQuote!=-1 )
		{
			if( position > debutQuote && position < finQuote )
			{
				return true;
			}
			debutQuote = finQuote+1;
		}
	} while( debutQuote!=-1 && finQuote!=-1 );
	return false;
}
//
