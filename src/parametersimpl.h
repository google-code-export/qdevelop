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

#ifndef PARAMETERSIMPL_H
#define PARAMETERSIMPL_H
//
#include "ui_parameters.h"
#include "projectmanager.h"
//
class ParametersImpl : public QDialog, public Ui::Param
{
Q_OBJECT
public:
	ParametersImpl( QWidget * parent );
	Parameters parameters();
	void setParameters(Parameters p);
private slots:
	void on_tableVariables_itemDoubleClicked(QTableWidgetItem* item);
	void on_edit_clicked();
	void on_sort_clicked();
	void on_defaults_clicked();
	void on_chooseDirectory_clicked();
	void on_add_clicked();
	void on_del_clicked();
private:
};
#endif





