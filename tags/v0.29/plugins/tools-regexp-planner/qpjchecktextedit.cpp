/***************************************************************************
 *   Copyright (C) 2008 by Prém József (Hungary)                           *
 *   pelz@freemail.hu                                                      *
 *                                                                         *
 *   This program made with Qt4 sotware of TROLLTECH ASA.                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   Version of Program: V1.0                                              *
 ***************************************************************************/

// header files:
#include <QtGui>
#include "qpjchecktextedit.h"

/********************** IMPLEMENTATION *****************************/
QpjCheckTextEdit::QpjCheckTextEdit(QWidget* parent) 
	: QTextEdit(parent)
{
	connectToFormating(true);
	setText("");
	regExp.setPattern("");
}

void QpjCheckTextEdit::focusInEvent(QFocusEvent* event)
{
	QTextEdit::focusInEvent(event);
	formatText();
}

void QpjCheckTextEdit::connectToFormating(bool yes)
{
	if (yes) {
		connect(
			this->document(), SIGNAL(contentsChanged()),
			this,             SLOT  (formatText     ())
		);
	}
	else {
		disconnect(
			this->document(), SIGNAL(contentsChanged()),
			this,             SLOT  (formatText     ())
		);
	}
}

void QpjCheckTextEdit::formatText()
{
	connectToFormating(false);
	if (regExp.isValid()  &&  regExp.pattern() != "") {
		QRegExpValidator* validator = new QRegExpValidator(regExp, this);
		QString text = document()->toPlainText();
		int startSize = text.size();
		int n = 0;
		while (
			text.size() > 0  &&  
			validator->validate(text, n) == QValidator::Invalid)
		{
			text.resize(text.size() - 1);
		}
		QTextCursor highlightCursor(document());
		highlightCursor.movePosition(QTextCursor::End);
		QTextCharFormat colorFormat(highlightCursor.charFormat());
		if (startSize > text.size()) {
			colorFormat.setForeground(Qt::red);
			highlightCursor.movePosition(
				QTextCursor::PreviousCharacter, 
				QTextCursor::KeepAnchor,
				startSize - text.size()
			);
			highlightCursor.mergeCharFormat(colorFormat);
		}
		highlightCursor.setPosition(text.size());
		if (text.size() > 0) {
			switch(validator->validate(text, n)) {
				case QValidator::Acceptable:
					colorFormat.setForeground(Qt::darkGreen);
					break;
				case QValidator::Intermediate:
					colorFormat.setForeground(Qt::blue);
					break;
				default:
					break;
			}
			highlightCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
			highlightCursor.mergeCharFormat(colorFormat);
		}
		delete validator;
	}
	else setText("");
	if (!isActiveWindow()) show();
	connectToFormating(true);
}

void QpjCheckTextEdit::setPatternString(const QString& pattern)
{
	regExp.setPattern(pattern);
	if (regExp.isValid()) formatText();
}
// END OF IMPLEMENTATION

/******************* COMMENTS AND ANNOTATIONS ************************
1, I write newly the focusInEvent() function, because I wanted to fomat
the text when the TextEditor comes in focus. If the user do not write
a valid regular expression, I stop the editing in the checker field.
2, I used the connectToFormating() function for switching the connection
between the document and formatText() function. I had to do this, because
without this switching I can make a endless rekursion with the formatText().
3, formatText() checks the text, and write the characters with colours.
*/
