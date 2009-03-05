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
#include <QtGui>
#include <QMessageBox>
#include <QFile>
#include <QDialog>
#include "astyledialogimpl.h"
#include "astyleplugin.h"
//
int AStyle_plugin_main(int argc, char *argv[]);
//
QString AStylePlugin::menuName() const
{
	return "Artistic Style Formatter Plugin";
}
//
TextEditInterface::Action AStylePlugin::action() const
{
	return TextEditInterface::ReplaceAll;
}
//
QString AStylePlugin::text(QString text, QString selectedText, QTextCodec *codec) 
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/astyle-plugin.ini", QSettings::IniFormat);
#else
	QSettings settings("astyle-plugin");
#endif
	settings.beginGroup("Arguments");
	QStringList arguments = settings.value("arguments", "--style=ansi").toStringList();
	settings.endGroup();
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
		return QString();
	file.write( text.toLocal8Bit() );
	file.close();
	//
	int nbArgs = arguments.count();
	char **argv = new char*[nbArgs+2];
     //
	argv[0] = NULL;
	int n = 1;
	foreach(QString s, arguments )
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
	QString formattedContent = QString::fromLocal8Bit(file.readAll());
	file.close();
	QFile( f ).remove();
	QFile( f+".orig" ).remove();
	if( !formattedContent.isEmpty() )
		return formattedContent;
	else
		return QString();
	Q_UNUSED(codec);
	Q_UNUSED(selectedText);
}
//
bool AStylePlugin::hasConfigDialog() const
{
	return true;
}

void AStylePlugin::config()
{
	AStyleDialogImpl *dialog = new AStyleDialogImpl( 0 );
	dialog->exec();
	delete dialog;
}
//
Q_EXPORT_PLUGIN2(pnp_textplugin, AStylePlugin)
