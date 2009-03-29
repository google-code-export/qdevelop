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

#ifndef MY_QPJREGEXPPLANNERDIALOG_H
#define MY_QPJREGEXPPLANNERDIALOG_H

// header files:
#include <QDialog>

// predeclarations:
class QpjCheckTextEdit;
class QpjValidIndicatorLabel;
class QLineEdit;
class QLabel;

/********************** CLASS DECLARATION **************************/
class QpjRegExpPlannerDialog : public QDialog
{
		Q_OBJECT
	public:
		QpjRegExpPlannerDialog(QWidget* parent = 0);
	private:
		QLineEdit* regExpEditor;
		QpjValidIndicatorLabel* indicator;
		QpjCheckTextEdit* checker;
		QLabel* labelEdit;
		QLabel* labelEmpty;
		QLabel* labelCheck;
		QLabel* labelColour;
		QLabel* labelGreen;
		QLabel* labelBlue;
		QLabel* labelRed;
	private slots:
		void aboutBox();
}; // END OF CLASS DECLARATION

#endif  // END OF ifndef...

/******************* COMMENTS AND ANNOTATIONS ************************
1, If you use the predeclarations, you do not have to load the 
header files. So the compiling is little bit faster.
*/
