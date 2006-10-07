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
  tabBar()->installEventFilter(this);
  m_clickedItem = -1;
}
//
TabWidget::~TabWidget()
{
}
//
bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{

  if (obj==tabBar())
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= 0x040200
      if( mouseEvent->button() == Qt::LeftButton )
        qApp->setOverrideCursor( Qt::OpenHandCursor );
#endif
      m_clickedItem = -1;
      for(int i=0; i<tabBar()->count(); i++)
      {
        if( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
        {
          m_clickedItem = i;
          break;
        }
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
      qApp->restoreOverrideCursor();
    }
    else if (event->type() == QEvent::MouseMove )
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

      for(int i=0; i<tabBar()->count(); i++)
      {
        if( tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
        {
          if( swapTabs(i, m_clickedItem) )
          {
            setCurrentWidget(widget(i));
            update();
            int x;
            if( !tabBar()->tabRect(i).contains( mouseEvent->pos() ) )
            {
              if( tabBar()->tabRect(m_clickedItem).x() < tabBar()->tabRect(i).x() )
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
void TabWidget::mouseMoveEvent( QMouseEvent * event )
{
  //qDebug() << m_clickedItem << event->pos();
  QTabWidget::mouseMoveEvent( event );
}
//
void TabWidget::mouseReleaseEvent( QMouseEvent * event )
{
  //qDebug() << m_clickedItem << event->pos();
  for(int i=0; i<tabBar()->count(); i++)
  {
    //qDebug() << i << tabBar()->tabRect(i);
  }
  QTabWidget::mouseReleaseEvent( event );
}
//
void TabWidget::mousePressEvent( QMouseEvent * event )
{
	if( m_clickedItem == -1 )
		return;
  if( event->button() == Qt::RightButton )
  {
    QMenu *menu = new QMenu(this);
    connect(menu->addAction(QIcon(":/toolbar/images/cross.png"), tr("Close Tab")), SIGNAL(triggered()), this, SLOT(slotCloseTab()) );
    connect(menu->addAction(QIcon(":/toolbar/images/fileclose.png"), tr("Close Other Tabs")), SIGNAL(triggered()), this, SLOT(slotCloseOtherTab()) );
    connect(menu->addAction(QIcon(":/toolbar/images/fileclose.png"), tr("Close All Tabs")), SIGNAL(triggered()), this, SLOT(slotCloseAllTab()) );
    menu->exec(event->globalPos());
    delete menu;
  }
  QTabWidget::mousePressEvent( event );
}
//
void TabWidget::slotCloseTab()
{
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
