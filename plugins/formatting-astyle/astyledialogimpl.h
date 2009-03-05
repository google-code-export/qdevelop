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
* Program URL   : http://qdevelop.org
*
*/
#ifndef ASTYLEDIALOGIMPL_H
#define ASTYLEDIALOGIMPL_H
//
#include "ui_astyle.h"
#include <pluginsinterfaces.h>
//
class AStyleDialogImpl : public QDialog, public Ui::AStyleDialog
{
    Q_OBJECT
public:
    AStyleDialogImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
    void on_aboutButton_clicked();
    void on_comboBox_currentIndexChanged(QString );
    void on_checkBox13_toggled(bool checked);
    void on_checkBox14_toggled(bool checked);
    void on_checkBox16_toggled(bool checked);
    void on_checkBox17_toggled(bool checked);
    void on_checkBox15_toggled(bool checked);
    void on_checkBox18_toggled(bool checked);
    void on_checkBox12_toggled(bool checked);
    void on_checkBox11_toggled(bool checked);
    void on_checkBox10_toggled(bool checked);
    void on_checkBox9_toggled(bool checked);
    void on_checkBox8_toggled(bool checked);
    void on_checkBox3_toggled(bool checked);
    void on_checkBox2_toggled(bool checked);
    void on_spacesSize_valueChanged(int );
    void on_qt_spinbox_lineedit_textChanged(QString );
    void on_checkBox5_toggled(bool checked);
    void on_checkBox7_toggled(bool checked);
    void on_checkBox1_toggled(bool checked);
    void on_checkBox4_toggled(bool checked);
    void on_checkBox6_toggled(bool checked);
    void on_gnu_toggled(bool checked);
    void on_ansi_toggled(bool checked);
    void on_linux_toggled(bool checked);
    void on_java_toggled(bool checked);
    void on_kr_toggled(bool checked);
    void on_okButton_clicked();
    void on_custom_toggled(bool checked);
private:
    void sample();
    QStringList args();
    QString m_textSample;
};
#endif
