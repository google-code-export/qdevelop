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
#ifndef SHORTCUTSIMPL_H
#define SHORTCUTSIMPL_H

#include "ui_shortcuts.h"
//
class MainImpl;
//
class ShortcutsImpl : public QDialog, public Ui::Shortcuts
{
Q_OBJECT
public:
	ShortcutsImpl(QWidget * parent);
private:
	MainImpl *m_mainImpl;
	void initTable(MainImpl *main);
private slots:
	void slotAccept();
	void slotDefault();
signals:
};

#endif
