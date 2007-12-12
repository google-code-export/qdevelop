/*
* Open File in Project dialog implementation
* Copyright (C) 2007  Branimir Karadzic
*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/

#include <QDir>
#include <QKeyEvent>
#include <QTreeWidget>
#include "openfileimpl.h"

OpenFileImpl::OpenFileImpl(QWidget* _pParent, ProjectManager* _pProjectManager, MainImpl* _pMainImpl)
        : QDialog(_pParent)
        , m_pProjectManager(_pProjectManager)
        , m_pMainImpl(_pMainImpl)
{
    setupUi(this);

    connect(FilterEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)) );
    connect(FilterEdit, SIGNAL(returnPressed()), this, SLOT(slotSelect()) );
    connect(FileList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotSelect()) );
    FilterEdit->installEventFilter(this);

    QList<QTreeWidgetItem*> projectsList;
    m_pProjectManager->childsList(0, "PROJECT", projectsList);

    // TODO - FileList should be regenerated based on FilterEdit string.
    for (int nbProjects = 0; nbProjects < projectsList.count(); nbProjects++)
    {
        QList<QTreeWidgetItem*> dirSourcesList;
        m_pProjectManager->childsList(projectsList.at(nbProjects), "SOURCES", dirSourcesList);
        m_pProjectManager->childsList(projectsList.at(nbProjects), "HEADERS", dirSourcesList);
        m_pProjectManager->childsList(projectsList.at(nbProjects), "RESOURCES", dirSourcesList);
        for (int nbSrc=0; nbSrc < dirSourcesList.count(); nbSrc++)
        {
            QList<QTreeWidgetItem *> filesList;
            m_pProjectManager->childsList(dirSourcesList.at(nbSrc), "DATA", filesList);
            for (int nbFic=0; nbFic < filesList.count(); nbFic++)
            {
                QString name = filesList.at(nbFic)->text(0);

                FileList->insertItem(0, name);
            }
        }

        FileList->sortItems(Qt::AscendingOrder);
    }

    FilterEdit->setFocus();
}

bool OpenFileImpl::eventFilter(QObject* _pObject, QEvent* _pEvent)
{
    if ( _pObject == FilterEdit
            &&   _pEvent->type() == QEvent::KeyPress )
    {
        switch (static_cast<QKeyEvent*>(_pEvent)->key())
        {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
            QApplication::sendEvent(FileList, _pEvent);
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(_pObject, _pEvent);
}

void OpenFileImpl::slotTextChanged(QString text)
{
    QList<QListWidgetItem*> matches = FileList->findItems(text, Qt::MatchContains);
    if ( 0 < matches.count() )
    {
        FileList->setCurrentItem(matches.at(0));
    }
    else
    {
        FileList->setItemSelected( FileList->currentItem(), false );
    }
}

void OpenFileImpl::slotSelect()
{
    QList<QListWidgetItem*> selected = FileList->selectedItems();

    if ( 0 < selected.count() )
    {
        QListWidgetItem* pItem = selected.at(0);

        QList<QTreeWidgetItem *> projectsList;
        m_pProjectManager->childsList(0, "PROJECT", projectsList);

        QString filename = pItem->text();
        QString projectName = m_pProjectManager->projectFilename(projectsList.at(0));
        QString projectDirectory = m_pProjectManager->projectDirectory(projectsList.at(0));
        QString absoluteName = QDir(projectDirectory+"/"+filename).absolutePath();
        QStringList locationsList;
        locationsList << absoluteName;
        foreach(QString dir, m_pProjectManager->dependpath(projectsList.at(0)) )
        {
            locationsList << QDir(projectDirectory + "/" +dir + "/" + filename).absolutePath();
        }
        m_pMainImpl->openFile( locationsList );
    }
    accept();
}
