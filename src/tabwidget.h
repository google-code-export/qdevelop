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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/
#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
//
class MainImpl;
class QToolButton;
//
class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    TabWidget(MainImpl *parent);
    virtual ~TabWidget();
    void setCloseButtonInTabs(bool b);
protected:
    //void mousePressEvent( QMouseEvent * event );
    bool eventFilter(QObject *obj, QEvent *event);
    void tabInserted ( int index );
private:
    bool swapTabs(int index1, int index2);
    int m_clickedItem;
    MainImpl *m_mainImpl;
    QToolButton *cross;
    QPoint mousePos;
    bool m_closeButtonInTabs;
private slots:
    void slotCloseTab();
    void slotCloseOtherTab();
    void slotCloseAllTab();
};

#endif

// kate: space-indent on; tab-indent off; tab-width 4; indent-width 4; mixedindent off; indent-mode cstyle; 
// kate: end-of-line: unix
