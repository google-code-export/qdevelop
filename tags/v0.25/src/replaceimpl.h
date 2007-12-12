/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2007  Jean-Luc Biord
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

#ifndef REPLACEIMPL_H
#define REPLACEIMPL_H
//
#include "ui_replace.h"
#include "editor.h"
//
class TextEdit;
//
class ReplaceImpl : public QDialog, public Ui::Replace
{
Q_OBJECT
public:
	ReplaceImpl( QWidget * parent, TextEdit *textEdit, ReplaceOptions options);
	ReplaceOptions replaceOptions() { return m_replaceOptions;};
private slots:
	void on_replace_clicked();
protected:
private:
	void saveReplaceOptions();
	TextEdit *m_textEdit;
	ReplaceOptions m_replaceOptions;
};
#endif


