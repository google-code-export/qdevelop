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

#ifndef __OPENFILEIMPL_H__
#define __OPENFILEIMPL_H__

#include "ui_openfile.h"
#include "mainimpl.h"
#include "projectmanager.h"

class OpenFileImpl : public QDialog, public Ui::OpenFile
{
	Q_OBJECT
	
public:
	OpenFileImpl(QWidget* _pParent, ProjectManager* _pProjectManager, MainImpl* _pMainImpl);
	bool eventFilter(QObject* _pObject, QEvent* _pEvent);
	
private:
	ProjectManager* m_pProjectManager;
	MainImpl* m_pMainImpl;
	
private slots:
	void slotTextChanged(QString text);
	void slotSelect();
};

#endif // __OPENFILEIMPL_H__
