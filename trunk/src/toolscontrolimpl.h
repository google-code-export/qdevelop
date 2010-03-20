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
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/

#ifndef TOOLSCONTROLIMPL_H
#define TOOLSCONTROLIMPL_H
//
#include "ui_toolsControl.h"
//
class ToolsControlImpl : public QDialog, public Ui::ToolsControl
{
Q_OBJECT
public:
	ToolsControlImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	bool toolsControl();
	QString qmakeName() { return qmake->text(); };
	QString makeName() { return make->text(); };
	QString gdbName() { return gdb->text(); };
	QString ctagsName() { return ctags->text(); };
	QString linguistName() { return linguist->text(); };
	QString lupdateName() { return lupdate->text(); };
	QString lreleaseName() { return lrelease->text(); };
	QString designerName() { return designer->text(); };
	QString assistantName() { return assistant->text(); };
	bool ctagsIsPresent()  { return m_ctagsIsPresent; };
	bool  checkEnvOnStartup() { return checkEnvironmentOnStartup->isChecked(); };
	QString qVersion();
private slots:
	void on_assistantLocation_clicked();
	void on_linguistLocation_clicked();
	void on_lupdateLocation_clicked();
	void on_lreleaseLocation_clicked();
	void on_designerLocation_clicked();
	void on_buttonBox_clicked(QAbstractButton * button );	
	void on_qmakeLocation_clicked();
	void on_makeLocation_clicked();
	void on_gdbLocation_clicked();
	void on_ctagsLocation_clicked();
	void on_test_clicked();
	void chooseLocation(QLineEdit *dest);
private:
	bool m_ctagsIsPresent;
	QString m_qVersion;
};
#endif










