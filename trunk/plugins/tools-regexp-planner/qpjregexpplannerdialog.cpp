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
#include "qpjregexpplannerdialog.h"
#include "qpjchecktextedit.h"
#include "qpjvalidindicatorlabel.h"

/********************** IMPLEMENTATION *****************************/
QpjRegExpPlannerDialog::QpjRegExpPlannerDialog(QWidget* parent) 
	: QDialog(parent)
{
	// making objects:
	regExpEditor = new QLineEdit();
	indicator    = new QpjValidIndicatorLabel();
	checker      = new QpjCheckTextEdit();
	labelEdit    = new QLabel(tr
		("Regular Expression Editor   e.g.  [A-Za-z][1-9][0-9]{0,2}")
	);
	labelEmpty   = new QLabel("");
	labelCheck   = new QLabel(tr("Regular Expression Checker"));
	labelColour  = new QLabel(tr("Meaning of Colours:"));
	labelGreen   = new QLabel("<font color=green>" + tr("Acceptable") + "</font>");
	labelBlue    = new QLabel("<font color=blue>" + tr("Intermediate") + "</font>");
	labelRed     = new QLabel("<font color=red>" + tr("Invalid") + "</font>");
	
	// by Divius [for QDevelop]
	QPushButton * buttonClose = new QPushButton(tr("Close"));
	connect(buttonClose, SIGNAL(clicked()), SLOT(close()));
	// ! by Divius

	// making tha layout:
	QHBoxLayout* h1Layout = new QHBoxLayout;
	h1Layout->addWidget(regExpEditor);
	h1Layout->addWidget(indicator);
	QVBoxLayout* v1Layout = new QVBoxLayout;
	v1Layout->addStretch();
	v1Layout->addWidget(labelColour);
	v1Layout->addWidget(labelGreen);
	v1Layout->addWidget(labelBlue);
	v1Layout->addWidget(labelRed);
	v1Layout->addStretch();
	v1Layout->addWidget(buttonClose);
	QHBoxLayout* h2Layout = new QHBoxLayout;
	h2Layout->addWidget(checker);
	h2Layout->addLayout(v1Layout);
	QVBoxLayout* theLayout = new QVBoxLayout;
	theLayout->addWidget(labelEdit);
	theLayout->addLayout(h1Layout);
	theLayout->addWidget(labelEmpty);
	theLayout->addWidget(labelCheck);
	theLayout->addLayout(h2Layout);
	setLayout(theLayout); // the main layout 
	// Removed by Divius [for QDevelop] setWindowIcon(QIcon("images/pj.png"));  // my icon
	setWindowTitle(tr("RegExp Planner")); // title of dialog
	
	// making the connections between objects:
	connect(
		regExpEditor, SIGNAL(textChanged(const QString&)),
		indicator,    SLOT  (setView    (const QString&))
	);
	connect(
		regExpEditor, SIGNAL(textChanged     (const QString&)),
		checker,      SLOT  (setPatternString(const QString&))
	);
}
// END OF IMPLEMENTATION
