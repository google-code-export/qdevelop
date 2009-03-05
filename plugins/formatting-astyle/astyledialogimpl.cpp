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
#include <QSettings>
#include <QDir>
#include <QTextEdit>
#include "astyledialogimpl.h"
//
int AStyle_plugin_main(int argc, char *argv[]);
//
AStyleDialogImpl::AStyleDialogImpl( QWidget * parent, Qt::WFlags f) 
	: QDialog(parent, f)
{
	setupUi(this);
	m_textSample = textEdit->toPlainText();
	indentation->setDisabled( true );
	formatting->setDisabled( true );
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/astyle-plugin.ini", QSettings::IniFormat);
#else
	QSettings settings("astyle-plugin");
#endif
	settings.beginGroup("Dialog");
	ansi->setChecked( settings.value("ansi", false).toBool() );
	kr->setChecked( settings.value("kr", false).toBool() );
	Linux->setChecked( settings.value("linux", false).toBool() );
	gnu->setChecked( settings.value("gnu", false).toBool() );
	java->setChecked( settings.value("java", false).toBool() );
	custom->setChecked( settings.value("custom", false).toBool() );
	checkBox1->setChecked( settings.value("checkBox1", false).toBool() );
	checkBox2->setChecked( settings.value("checkBox2", false).toBool() );
	checkBox3->setChecked( settings.value("checkBox3", false).toBool() );
	checkBox4->setChecked( settings.value("checkBox4", false).toBool() );
	checkBox5->setChecked( settings.value("checkBox5", false).toBool() );
	checkBox6->setChecked( settings.value("checkBox6", false).toBool() );
	checkBox7->setChecked( settings.value("checkBox7", false).toBool() );
	checkBox8->setChecked( settings.value("checkBox8", false).toBool() );
	checkBox9->setChecked( settings.value("checkBox9", false).toBool() );
	checkBox10->setChecked( settings.value("checkBox10", false).toBool() );
	checkBox11->setChecked( settings.value("checkBox11", false).toBool() );
	checkBox12->setChecked( settings.value("checkBox12", false).toBool() );
	checkBox13->setChecked( settings.value("checkBox13", false).toBool() );
	checkBox14->setChecked( settings.value("checkBox14", false).toBool() );
	checkBox15->setChecked( settings.value("checkBox15", false).toBool() );
	checkBox16->setChecked( settings.value("checkBox16", false).toBool() );
	checkBox17->setChecked( settings.value("checkBox17", false).toBool() );
	checkBox18->setChecked( settings.value("checkBox18", false).toBool() );
	spacesSize->setValue( settings.value("spacesSize", false).toInt() );
	comboBox->setCurrentIndex( settings.value("comboBox", false).toInt() );
	settings.endGroup();
}
//

QStringList AStyleDialogImpl::args()
{
	QStringList s;
	if( ansi->isChecked() )
		s << "--style=ansi";
	else if( kr->isChecked() )
		s << "--style=kr";
	else if( Linux->isChecked() )
		s << "--style=linux";
	else if( gnu->isChecked() )
		s << "--style=gnu";
	else if( java->isChecked() )
		s << "--style=java";
	else
	{
		if( checkBox1->isChecked() )
			s << QString().sprintf("--indent=tab=%d", spacesSize->value());
		else
			s << QString().sprintf("--indent=spaces=%d", spacesSize->value());
		if( checkBox2->isChecked() )
			s << QString().sprintf("--force-indent=tab=%d", spacesSize->value());
		if( checkBox3->isChecked() )
			s << "--convert-tabs";
		if( checkBox4->isChecked() )
			s << "--fill-empty-lines";
		if( checkBox5->isChecked() )
			s << "--indent-classes";
		if( checkBox6->isChecked() )
			s << "--indent-switches";
		if( checkBox7->isChecked() )
			s << "--indent-cases";
		if( checkBox8->isChecked() )
			s << "--indent-brackets";
		if( checkBox9->isChecked() )
			s << "--indent-blocks";
		if( checkBox10->isChecked() )
			s << "--indent-namespaces";
		if( checkBox11->isChecked() )
			s << "--indent-labels";
		if( checkBox12->isChecked() )
			s << "--indent-preprocessor";
		if( comboBox->currentText() != "none" )
			s << "--brackets="+comboBox->currentText();
		// Formatting tab
		if( checkBox13->isChecked() )
			s << "--break-blocks=all";
		if( checkBox14->isChecked() )
			s << "--break-elseifs";
		if( checkBox15->isChecked() )
			s << "--pad=oper";
		if( checkBox16->isChecked() )
			s << "--pad=paren";
		if( checkBox17->isChecked() )
			s << "--one-line=keep-statements";
		if( checkBox18->isChecked() )
			s << "--one-line=keep-blocks";
	}
	return s;
}
//
void AStyleDialogImpl::on_okButton_clicked()
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/astyle-plugin.ini", QSettings::IniFormat);
#else
	QSettings settings("astyle-plugin");
#endif
	settings.beginGroup("Arguments");
	settings.setValue("arguments", args());
	settings.endGroup();
	settings.beginGroup("Dialog");
	settings.setValue("ansi", ansi->isChecked());
	settings.setValue("kr", kr->isChecked());
	settings.setValue("linux", Linux->isChecked());
	settings.setValue("gnu", gnu->isChecked());
	settings.setValue("java", java->isChecked());
	settings.setValue("custom", custom->isChecked());
	settings.setValue("checkBox1", checkBox1->isChecked());
	settings.setValue("checkBox2", checkBox2->isChecked());
	settings.setValue("checkBox3", checkBox3->isChecked());
	settings.setValue("checkBox4", checkBox4->isChecked());
	settings.setValue("checkBox5", checkBox5->isChecked());
	settings.setValue("checkBox6", checkBox6->isChecked());
	settings.setValue("checkBox7", checkBox7->isChecked());
	settings.setValue("checkBox8", checkBox8->isChecked());
	settings.setValue("checkBox9", checkBox9->isChecked());
	settings.setValue("checkBox10", checkBox10->isChecked());
	settings.setValue("checkBox11", checkBox11->isChecked());
	settings.setValue("checkBox12", checkBox12->isChecked());
	settings.setValue("checkBox13", checkBox13->isChecked());
	settings.setValue("checkBox14", checkBox14->isChecked());
	settings.setValue("checkBox15", checkBox15->isChecked());
	settings.setValue("checkBox16", checkBox16->isChecked());
	settings.setValue("checkBox17", checkBox17->isChecked());
	settings.setValue("checkBox18", checkBox18->isChecked());
	
	settings.setValue("spacesSize", spacesSize->value());
	settings.setValue("comboBox", comboBox->currentIndex());
	settings.endGroup();
}
//

void AStyleDialogImpl::on_gnu_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setDisabled( checked );
	formatting->setDisabled( checked );
	sample();
}
//
void AStyleDialogImpl::on_ansi_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setDisabled( checked );
	formatting->setDisabled( checked );
	sample();
}
//
void AStyleDialogImpl::on_linux_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setDisabled( checked );
	formatting->setDisabled( checked );
	sample();
}
//
void AStyleDialogImpl::on_java_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setDisabled( checked );
	formatting->setDisabled( checked );
	sample();
}
//
void AStyleDialogImpl::on_kr_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setDisabled( checked );
	formatting->setDisabled( checked );
	sample();
}
//
void AStyleDialogImpl::on_custom_toggled(bool checked)
{
	if( !checked )
		return;
	indentation->setEnabled( checked );
	formatting->setEnabled( checked );
	sample();
}
//
#include <QMessageBox>
void AStyleDialogImpl::sample()
{
	textEdit->setPlainText(m_textSample);
	// Find temp filename
	QString f;
	int numTempFile = 0;
	do {
		numTempFile++;
		f = QDir::tempPath()+"/astyle-plugin_"+QString::number(numTempFile)+".cpp";
	} while( QFile::exists( f ) );
	// Open temp file for writing
	QFile file( f );
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text ) ) 
	{
		textEdit->setPlainText( "problem" );
		return;
	}
	file.write( textEdit->toPlainText().toLocal8Bit() );
	file.close();
	//
	int nbArgs = args().count();
	char **argv = new char*[nbArgs+2];
     //
	argv[0] = NULL;
	int n = 1;
	foreach(QString s, args() )
	{
		argv[n] = new char[s.length()+1];
		strcpy(argv[n], s.toLocal8Bit().data());
		n++;
	}
	//
	QByteArray array( f.toLocal8Bit() );
	argv[n] = new char[f.length()+1];
	strcpy(argv[n], array.data());
	// Call astyle formatter
	AStyle_plugin_main(n+1, argv);
	for(int i=1; i<n; i++)
	{
		delete argv[i];
	}
	delete argv;
	// Read results
	file.open( QIODevice::ReadOnly | QIODevice::Text );
	QString formattedContent = file.readAll();
	file.close();
	QFile( f ).remove();
	QFile( f+".orig" ).remove();
	if( !formattedContent.isEmpty() )
		textEdit->setPlainText( formattedContent );
	else
		textEdit->setPlainText( "problem" );
     textArgs->setText( "astyle "+args().join(" ") );
}
//
void AStyleDialogImpl::on_comboBox_currentIndexChanged(QString )
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox13_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox14_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox16_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox17_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox15_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox18_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox12_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox11_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox10_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox9_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox8_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox3_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox2_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_spacesSize_valueChanged(int )
{
	sample();
}
//
void AStyleDialogImpl::on_qt_spinbox_lineedit_textChanged(QString )
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox5_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox7_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox1_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox4_toggled(bool checked)
{
	sample();
}
//
void AStyleDialogImpl::on_checkBox6_toggled(bool checked)
{
	sample();
}
//

void AStyleDialogImpl::on_aboutButton_clicked()
{
	QMessageBox::about(0, "Artistic Style Formatter Plugin", 
		"              AStyle plugin for QDevelop\n"
		"           Copyright (c) 2006 Jean-Luc Biord\n\n"
		"                   jlbiord@qtfr.org\n\n"
		"Part of this plugin is Artistic Style - an indentation and\n"
		" reformatting tool for C, C++, C# and Java source files.\n"
		"             http://astyle.sourceforge.net\n\n"
		);
}
//
