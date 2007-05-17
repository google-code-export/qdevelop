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
#include <QHeaderView>
#include "shortcutsimpl.h"
#include "ui_main.h"
#include "mainimpl.h"
#include "misc.h"

//
ShortcutsImpl::ShortcutsImpl(QWidget * parent) 
	: QDialog(parent), m_mainImpl((MainImpl *)parent)
{
	setupUi(this); 
	initTable( m_mainImpl );
	connect(okButton, SIGNAL(clicked()), this, SLOT(slotAccept()) );
	connect(defaultButton, SIGNAL(clicked()), this, SLOT(slotDefault()) );
}

//
void ShortcutsImpl::initTable(MainImpl *main)
{
	QList<QObject*> childrens = main->children();
	QListIterator<QObject*> iterator(childrens);
	int row = 0;
	
	while( iterator.hasNext() )
	{
		QObject *object = iterator.next();
		QAction *action = qobject_cast<QAction*>(object);
		
		if (action)
		{
			QString text = action->text().remove("&");
			
			if ( !text.isEmpty() && !(action->data().toString().contains("Recent|")) )
			{
				QString shortcut = action->shortcut();
				QTableWidgetItem *newItem = new QTableWidgetItem(text);
				
				newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
				newItem->setData(Qt::UserRole, addressToVariant(object));
				newItem->setIcon(action->icon());
				table->setRowCount(row+1);
				table->setItem(row, 0, newItem);
				table->setItem(row++, 1, new QTableWidgetItem(shortcut));
			}
		}
		table->sortItems( 0 );
	}
	
	QHeaderView *header = table->horizontalHeader();
	header->resizeSection( 0, 230 );
	table->verticalHeader()->hide();
}

//
void ShortcutsImpl::slotAccept()
{
	for(int row=0; row<table->rowCount(); row++ )
	{
		QTableWidgetItem *item = table->item(row, 0);
		QAction *action = (QAction *)variantToAction( item->data(Qt::UserRole) );
		QString shortcut = table->item(row, 1)->text();
		action->setShortcut( shortcut );
	}
}

//
void ShortcutsImpl::slotDefault()
{
	QMainWindow *dial = new QMainWindow;
	Ui::Main ui;
	ui.setupUi(dial);
	
	QList<QObject*> childrens = dial->children();
	QListIterator<QObject*> iterator(childrens);
	
	while( iterator.hasNext() )
	{
		QObject *object = iterator.next();
		QAction *action = qobject_cast<QAction*>(object);
		
		if (action)
		{
			QString text = action->text().remove("&");
			QString shortcut = action->shortcut();
			QList<QTableWidgetItem *> listFind = table->findItems(text , Qt::MatchExactly);
			if( listFind.count() )
				table->item(table->row(listFind.first()), 1)->setText(shortcut);
		}
	}
	
	delete dial;
}
