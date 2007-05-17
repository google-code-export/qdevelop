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

#include <QMouseEvent>
#include <QTabBar>
#include <QMenu>
#include <QDebug>

#include "tabwidget.h"
#include "mainimpl.h"
//
TabWidget::TabWidget(MainImpl *parent)
        : QTabWidget(parent), m_mainImpl( parent )
{
    setMouseTracking( true );
    cross = new QToolButton(this);
    cross->hide();
    cross->setIcon( QIcon(":/toolbar/images/cross.png") );
    connect(cross, SIGNAL(clicked()), this, SLOT(slotCloseTab()) );
    cross->setGeometry(0,0,15,15);
    tabBar()->installEventFilter(this);
    m_clickedItem = -1;
    m_closeButtonInTabs = true;
}
//
TabWidget::~TabWidget()
{}
//
bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==tabBar())
    {
        if (event->type() == QEvent::Leave)
        {
            QPoint point = cross->mapToGlobal( QPoint(0, 0) );
            QRect rect(point.x(), point.y(), cross->width(), cross->height() );
            if ( !rect.contains( QCursor::pos() ) )
                cross->hide();
        }
        else if (event->type() == QEvent::HoverMove && m_closeButtonInTabs )
        {
            QHoverEvent *mouseEvent = static_cast<QHoverEvent *>(event);
            mousePos = mouseEvent->pos();
            for (int i=0; i<tabBar()->count(); i++)
            {
                if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
                {
                    m_clickedItem= i;
                    break;
                }
            }
            cross->setGeometry(tabBar()->tabRect(m_clickedItem).x()+tabBar()->tabRect(m_clickedItem).width()-cross->width()-5,
                               //tabBar()->height()-(tabBar()->tabRect(m_clickedItem).height()/2),
                               5,
                               cross->width(),
                               cross->height());
            cross->show();
        }
        else if (event->type() == QEvent::MouseButtonRelease )
        {
            qApp->restoreOverrideCursor();
        }
        else if (event->type() == QEvent::MouseButtonPress )
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            for (int i=0; i<tabBar()->count(); i++)
            {
                if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
                {
                    m_clickedItem = i;
                    break;
                }
            }
            if ( mouseEvent->button() == Qt::LeftButton )
                qApp->setOverrideCursor( Qt::OpenHandCursor );
            if ( mouseEvent->button() == Qt::RightButton )
            {
                QMenu *menu = new QMenu(this);
                connect(menu->addAction(QIcon(":/toolbar/images/cross.png"), tr("Close Tab")), SIGNAL(triggered()), this, SLOT(slotCloseTab()) );
                connect(menu->addAction(QIcon(":/toolbar/images/fileclose.png"), tr("Close Other Tabs")), SIGNAL(triggered()), this, SLOT(slotCloseOtherTab()) );
                connect(menu->addAction(QIcon(":/toolbar/images/fileclose.png"), tr("Close All Tabs")), SIGNAL(triggered()), this, SLOT(slotCloseAllTab()) );
                menu->exec(mouseEvent->globalPos());
                delete menu;
            }
        }
        else if (event->type() == QEvent::MouseMove )
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            for (int i=0; i<tabBar()->count(); i++)
            {
                if ( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
                {
                    if ( swapTabs(i, m_clickedItem) )
                    {
                        setCurrentWidget(widget(i));
                        update();
                        int x;
                        if ( !tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
                        {
                            if ( tabBar()->tabRect(m_clickedItem).x() < tabBar()->tabRect(i).x() )
                                x = tabBar()->tabRect(i).x();
                            else
                                x = tabBar()->tabRect(i).x()+(tabBar()->tabRect(i).width()-(qAbs(tabBar()->tabRect(i).width()-tabBar()->tabRect(m_clickedItem).width())));
                            QPoint point =  QPoint(
                                                x,
                                                mouseEvent->pos().y()
                                            );
                            point =  widget(i)->mapToGlobal( point );
                            m_clickedItem = i;
                            QCursor::setPos ( point.x(), QCursor::pos().y() );
                        }
                        m_clickedItem = i;
                        break;
                    }
                }
            }
        }
    }
    return QTabWidget::eventFilter( obj, event);
}
//
void TabWidget::setCloseButtonInTabs(bool b) 
{ 
	m_closeButtonInTabs = b; 
	if( !m_closeButtonInTabs )
		cross->hide();
}
//
bool TabWidget::swapTabs(int index1, int index2)
{
    if (index1==index2)
        return false;
    int t1 = qMin(index1,index2);
    int t2 = qMax(index1,index2);

    index1=t1;
    index2=t2;

    QString name1 = tabBar()->tabText(index1);
    QString name2 = tabBar()->tabText(index2);

    QWidget *editor1 = widget(index1);
    QWidget *editor2 = widget(index2);

    removeTab(index2);
    removeTab(index1);

    insertTab(index1,editor2,name2);
    insertTab(index2,editor1,name1);
    return true;
}
//
void TabWidget::slotCloseTab()
{
    cross->hide();
    m_mainImpl->closeTab( m_clickedItem );
}
//
void TabWidget::slotCloseAllTab()
{
    m_mainImpl->slotCloseAllFiles(  );
}
//
void TabWidget::slotCloseOtherTab()
{
    m_mainImpl->closeOtherTab( m_clickedItem );
}
//
