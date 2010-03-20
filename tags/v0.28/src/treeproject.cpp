/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006 - 2010 Jean-Luc Biord
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
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/

#include "treeproject.h"
#include "projectmanager.h"

#include <QMenu>
#include <QMouseEvent>
#include <QItemDelegate>
#include <QPainter>
#include <QHeaderView>

#include <QDebug>
// Delegator

/**
 * @author: David Cuadrado \<krawek@gmail.com\>
 */
class Delegator: public QItemDelegate
{
	public:
		Delegator(QTreeWidget *tree, QWidget *parent );
		~Delegator();
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
	private:
		QTreeWidget *m_tree;
};

Delegator::Delegator(QTreeWidget *tree, QWidget *parent)
    : QItemDelegate(parent), m_tree(tree)
{
}

Delegator::~Delegator()
{
}


void Delegator::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const QAbstractItemModel *model = index.model();
	QString text = model->data(index, Qt::DisplayRole).toString();
	QRect r = option.rect;
    r.setX( 0 );
    
	if ( text.toLower().endsWith(".pro") )
    {
    	// Project
    	
    	QPalette palette = option.palette;
    	
    	if (!model->parent(index).isValid())
    	{
    		QColor color = palette.highlight().color();
    		color.setAlpha(70);
    		palette.setBrush(QPalette::Button, color);
    	}
    		
    	painter->save();
    	
        QStyleOptionButton buttonOption;

        buttonOption.state = option.state;
        buttonOption.state &= ~QStyle::State_HasFocus;
        
        buttonOption.rect = r;
        buttonOption.palette = palette;
        buttonOption.features = QStyleOptionButton::None;
        
		if(m_tree->selectedItems().contains(static_cast<TreeProject *>(m_tree)->itemFromIndex(index)))
		{
			buttonOption.state |= QStyle::State_Sunken;
		}
        
        m_tree->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, m_tree);
        
        painter->restore();
        
        // draw text
        
        QRect textRect = QRect(r.left() + 20, r.top(), r.width() - (option.fontMetrics.width(text) /2), r.height());
        QString textToDraw = elidedText(option.fontMetrics, textRect.width(), Qt::ElideMiddle, text.left(text.length()-4) );
        m_tree->style()->drawItemText(painter, textRect, Qt::AlignCenter, option.palette, m_tree->isEnabled(), textToDraw);
    }
    else 
    {
    	// Childs
    	QStyleOptionViewItem newOption = option;
    	
    	QModelIndex i = model->parent(model->parent(index));
    	if ( i.isValid() )
    	{
    		QString text = model->data(i, Qt::DisplayRole).toString();
    		if ( !text.toLower().endsWith(".pro") )
    		{
    			r.setX(0);
    		}
    		else
    		{
    			r.setX(20);
   			}
  		}
    	newOption.rect = r;
        QItemDelegate::paint(painter, newOption, index);
    }
}


QSize Delegator::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    QSize sz = QItemDelegate::sizeHint(opt, index) + QSize(2, 2);
    return sz;
}

//
TreeProject::TreeProject(QWidget * parent) : QTreeWidget(parent)
{
    //setDragEnabled(true);
    //setAcceptDrops(true);
    //setDropIndicatorShown(true);
	//setAcceptDrops ( true );
	krawekItemDelegate = 0;
	setup(); 
}

TreeProject::~TreeProject()
{
}

//
void TreeProject::drawBranches( QPainter * painter, const QRect & rect, const QModelIndex & index ) const 
{
	if( normalItemDelegate == itemDelegate() )
	{
		QTreeView::drawBranches(painter, rect, index );
	}
}

void TreeProject::setup()
{
	setColumnCount(1);
	normalItemDelegate = itemDelegate();
	slotShowAsNormal();
	krawekItemDelegate = 0;
}

void TreeProject::mousePressEvent( QMouseEvent * event )
{
	if( !m_projectManager )
		return;
	m_itemClicked = itemAt( event->pos() );
	if( !m_itemClicked )
		return;
	emit itemClicked(m_itemClicked, 0);
	if( event->button() == Qt::RightButton )
	{
		setCurrentItem( m_itemClicked );
		if( !m_itemClicked )
			return;
		QString key = m_projectManager->toKey( m_itemClicked->data(0, Qt::UserRole) );
		setCurrentItem( m_itemClicked );
		QMenu *menu = new QMenu(this);
		if( key == "PROJECT" || key == "SCOPE" )
		{
			if (key == "PROJECT" )
			{
				QAction *sousProjet = menu->addAction(QIcon(), tr("Add sub-project..."));
				connect(sousProjet, SIGNAL(triggered()), this, SLOT(slotAddSubProject()) );
				for(int i=0; i<m_itemClicked->childCount(); i++)
					if( QString("SCOPE:FORMS:HEADERS:RESOURCES:SOURCES:TRANSLATIONS").contains( m_projectManager->toKey( m_itemClicked->child( i )->data(0, Qt::UserRole) ) ) )
						sousProjet->setEnabled( false );
					
			}
			connect(menu->addAction(QIcon(), tr("Add New Item...")), SIGNAL(triggered()), this, SLOT(slotAddNewItem()) );
			connect(menu->addAction(QIcon(), tr("Add Scope...")), SIGNAL(triggered()), this, SLOT(slotAddScope()) );
			connect(menu->addAction(QIcon(), tr("Add New Class...")), SIGNAL(triggered()), this, SLOT(slotAddNewClass()) );
			connect(menu->addAction(QIcon(), tr("Add Existing Files...")), SIGNAL(triggered()), this, SLOT(slotAddExistingFiles()) );
		}
		else if( key == "DATA" )
		{
			if( m_projectManager->toKey( m_itemClicked->parent()->data(0, Qt::UserRole) ) == "TRANSLATIONS" )
			{
				connect(menu->addAction(QIcon(":/toolbar/images/edit.png"), tr("Open in Linguist")), SIGNAL(triggered()), this, SLOT(slotOpen()) );
				connect(menu->addAction(QIcon(":/toolbar/images/refresh.png"), tr("Refresh translation files")+" (.ts)"), SIGNAL(triggered()), this, SLOT(slotlupdate()) );
				connect(menu->addAction(QIcon(""), tr("Build release translation files")+" (.qm)"), SIGNAL(triggered()), this, SLOT(slotlrelease()) );
			}
			if( m_projectManager->toKey( m_itemClicked->parent()->data(0, Qt::UserRole) ) == "FORMS" )
			{
				connect(menu->addAction(QIcon(":/toolbar/images/edit.png"), tr("Open in Designer")), SIGNAL(triggered()), this, SLOT(slotOpen()) );
				connect(menu->addAction(QIcon(), tr("Dialog Subclassing...")), SIGNAL(triggered()), this, SLOT(slotSubclassing()) );
				connect(menu->addAction(QIcon(), tr("Dialog Preview")), SIGNAL(triggered()), this, SLOT(slotPreviewForm()) );
			}
			else
				connect(menu->addAction(QIcon(":/toolbar/images/edit.png"), tr("Open")), SIGNAL(triggered()), this, SLOT(slotOpen()) );
			connect(menu->addAction(QIcon(), tr("Rename...")), SIGNAL(triggered()), this, SLOT(slotRenameItem()) );
		}
		else if( QString("FORMS:HEADERS:RESOURCES:SOURCES:TRANSLATIONS").contains(key) )
		{
			connect(menu->addAction(QIcon(), tr("Add New Item...")), SIGNAL(triggered()), this, SLOT(slotAddNewItem()) );
		}
		// TODO: Divius: Temporary disabled for 0.27.1. may brake complex project files
		//connect(menu->addAction(QIcon(), tr("Sort")), SIGNAL(triggered()), this, SLOT(slotSort()) );
		connect(menu->addAction(QIcon(":/toolbar/images/editdelete.png"), tr("Remove from project")), SIGNAL(triggered()), this, SLOT(slotDeleteItem()) );
		menu->addSeparator();
		QTreeWidgetItem *tmp = m_itemClicked;
		while( tmp && m_projectManager->toKey( tmp->data(0, Qt::UserRole) ) != "PROJECT" )
			tmp = tmp->parent();
		if( tmp )
			connect(menu->addAction(QIcon(), tr("Properties of %1...").arg(tmp->text(0)) ), SIGNAL(triggered()), this, SLOT(slotProjectPropertie()) );
		//
		menu->addSeparator();
		QMenu *showAs = menu->addMenu(tr("Show As"));
		connect(showAs->addAction(QIcon(), tr("Normal")), SIGNAL(triggered()), this, SLOT(slotShowAsNormal()) );
		connect(showAs->addAction(QIcon(), tr("Krawek")), SIGNAL(triggered()), this, SLOT(slotShowAsKrawek()) );
		menu->exec(event->globalPos());
		delete menu;
    } else if( event->button() == Qt::MidButton )
    {
        QString key = m_projectManager->toKey( m_itemClicked->data(0, Qt::UserRole) );
        if ( (key == "DATA" ) &&
            ( m_projectManager->toKey( m_itemClicked->parent()->data(0, Qt::UserRole) ) == "FORMS" ) )
        {
            // show dialog
            previewForm(m_itemClicked);
        }
	}
	QTreeWidget::mousePressEvent(event);
}
//
void TreeProject::resizeEvent ( QResizeEvent * event )
{
	QTreeWidget::resizeEvent( event );
}
//
void TreeProject::keyPressEvent( QKeyEvent * event )
{
    if ( event->key( ) == Qt::Key_Return )
    {
        slotOpen( );
        event->setAccepted( true );
    }
    else
    {
        QTreeWidget::keyPressEvent( event );
        m_itemClicked = currentItem( );
    }
}
//
void TreeProject::slotShowAsNormal()
{
	setItemDelegate( normalItemDelegate );
	header()->hide();
	setRootIsDecorated(true);
	setAlternatingRowColors(false);
	if( krawekItemDelegate )
	{
		delete krawekItemDelegate;
		krawekItemDelegate = 0;
	}
}
//
void TreeProject::slotShowAsKrawek()
{
	if( !krawekItemDelegate )
		krawekItemDelegate = new Delegator(this, this);
	header()->hide();
	setRootIsDecorated(false);
	setItemDelegate( krawekItemDelegate );
	setAlternatingRowColors(true);
}
//
void TreeProject::slotAddNewItem()
{
	QString kind;
	QString key = m_projectManager->toKey( m_itemClicked->data(0, Qt::UserRole) );
	if( QString("FORMS:HEADERS:RESOURCES:SOURCES:TRANSLATIONS").contains(key) )
		kind = key;
	emit addNewItem(m_itemClicked, kind);
}
//
void TreeProject::slotPreviewForm()
{
	emit previewForm(m_itemClicked);
}
//
void TreeProject::slotSubclassing()
{
	emit subclassing(m_itemClicked);
}
//
void TreeProject::slotlupdate()
{
	emit lupdate(m_itemClicked);
}
//
void TreeProject::slotlrelease()
{
	emit lrelease(m_itemClicked);
}
//
void TreeProject::slotAddExistingFiles()
{
	emit addExistingsFiles(m_itemClicked);
}
//
void TreeProject::slotAddScope()
{
	emit addScope(m_itemClicked);
}
//
void TreeProject::slotAddSubProject()
{
	emit addSubProject(m_itemClicked);
}
//
void TreeProject::slotProjectPropertie()
{
	emit projectPropertie(m_itemClicked);
}
//
void TreeProject::slotOpen()
{
	emit open(m_itemClicked, 0);
}
//
void TreeProject::slotDeleteItem()
{
	emit deleteItem(m_itemClicked);
}
//
void TreeProject::slotRenameItem()
{
	emit renameItem(m_itemClicked);
}
//
void TreeProject::slotAddNewClass()
{
	emit addNewClass(m_itemClicked);
}
//
void TreeProject::slotSort()
{
	emit sort();
}
//
void TreeProject::dragEnterEvent(QDragEnterEvent *)
{
    //if (event->mimeData()->hasFormat("text/plain"))
        //event->acceptProposedAction();
}
//

