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
* Program URL   : http://qtfr.org
*
*/
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
//
class MainImpl;
//
class TabWidget : public QTabWidget  
{
Q_OBJECT 
public:
	TabWidget(MainImpl *parent);
	virtual ~TabWidget();
protected:
	void mousePressEvent( QMouseEvent * event );
	void mouseMoveEvent ( QMouseEvent * event );
	void mouseReleaseEvent ( QMouseEvent * event );
    bool eventFilter(QObject *obj, QEvent *event);
private:
    bool swapTabs(int index1, int index2);
	int m_clickedItem;
    int m_hoverItem;
	MainImpl *m_mainImpl;
private slots:
	void slotCloseTab();
	void slotCloseOtherTab();
	void slotCloseAllTab();
};
#endif 
