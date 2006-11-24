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

#include <QApplication>
#include <QPlastiqueStyle>
#include <QProcess>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QSplashScreen>
#include <QSettings>
#include <QDir>
#include <QProgressBar>
#include <QDebug>
#include "mainimpl.h"
//
QSplashScreen *splash = 0;
//
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	splash = new QSplashScreen(QPixmap(":/divers/images/SplashQDevelop.png"));
	splash->setFont( QFont("Helvetica", 10) );
	splash->show();
	QTranslator translator;
	QString translationFile = QLocale::languageToString( QLocale::system().language() );
	//
	QStringList toOpen;
	for(int i=0; i<QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).count(); i++)
	{
		QString s = QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).at(i);
		if( s == "-l" )
		{
			translationFile = QString(app.argv()[ 2 ]).split(" ",QString::SkipEmptyParts).at(i);
			
			//i++;
		}
		else
			toOpen << QString(app.argv()[ 1 ]).split(" ",QString::SkipEmptyParts).at(i);
	}
	translationFile = ":/translations/translations/QDevelop_"+translationFile+".qm";
	qApp->processEvents();
	//
	splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Interface translation"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	translator.load( translationFile );
	app.installTranslator( &translator );
	//
	MainImpl main;// = new MainImpl;
	splash->showMessage(QObject::tr("Environment control"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	main.slotToolsControl(false);
	
	main.loadINI();
	splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Files on editor"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	//
	foreach(QString s, toOpen)
	{
		if( s.right(4).toLower() == ".pro" )
		{
			main.openProject(s);
			break;
		}
		else
			main.openFile( QStringList( s ) );
	}
	//
	splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Plugins"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	main.loadPlugins();
	//
	splash->showMessage(QObject::tr("Main Window creation"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	main.setGeometry(50,50, 800, 550);
	main.showMaximized();
	splash->finish(&main);
	delete splash;
	splash = 0;
	//
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	return app.exec();
}
