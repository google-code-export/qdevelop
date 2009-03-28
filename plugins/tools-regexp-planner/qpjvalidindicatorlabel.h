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

#ifndef MY_QPJVIALIDINDICATORLABEL_H
#define MY_QPJVIALIDINDICATORLABEL_H

#define RED_TEXT_QPJVIALIDINDICATORLABEL "<font color=red>" + tr("Invalid") + "</font>"

#include <QLabel>

/********************** CLASS DECLARATION **************************/
class QpjValidIndicatorLabel : public QLabel
{
		Q_OBJECT
	public:
		QpjValidIndicatorLabel(
			const QString& text = RED_TEXT_QPJVIALIDINDICATORLABEL, 
			QWidget* parent = 0
		);
	public slots:
		void setView(const QString& pattern);
}; // END OF CLASS DECLARATION

#endif  // END OF ifndef...
