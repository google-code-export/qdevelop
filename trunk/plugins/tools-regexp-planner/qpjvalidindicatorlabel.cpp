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
#include "qpjvalidindicatorlabel.h"

/********************** IMPLEMENTATION *****************************/
QpjValidIndicatorLabel::QpjValidIndicatorLabel(const QString& text, QWidget* parent) 
	: QLabel(text, parent) 
{
	if (text.isEmpty())
	{
		setText("<font color=red>" + tr("Invalid") + "</font>");
	}
}

void QpjValidIndicatorLabel::setView(const QString& pattern) 
{
	QRegExp regExp(pattern);
	if (regExp.isValid()  &&  pattern != "") {
		setText("<font color=green>" + tr("Valid") + "</font>");
	}
	else {
		setText("<font color=red>" + tr("Invalid") + "</font>");
	}
}
// END OF IMPLEMENTATION
