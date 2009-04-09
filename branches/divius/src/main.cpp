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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/

#include <QApplication>
#include <QPlastiqueStyle>
#include <QProcess>
#include <QTranslator>
#include <QLocale>
#include <QSplashScreen>
#include <QSettings>
#include <QDir>
#include <QProgressBar>
#include <QDebug>
#include <QLibraryInfo>
#include <QFile>
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
	//
	// change the plugins path (add the installation directory)
	QStringList list_path ;
	QDir dir = QDir(qApp->applicationDirPath()+"/QtPlugins/");
	list_path << dir.absolutePath () << app.libraryPaths ();
	app.setLibraryPaths( list_path  );
	//
	QTranslator translatorQDevelop, translatorQt;
	QString language = QLocale::languageToString( QLocale::system().language() );
	//
	QStringList toOpen, args = QCoreApplication::arguments();
	if (!args.isEmpty()) args.removeFirst();
	bool nextArgIsLanguage = false;
	foreach (QString arg, args)
	{
		if (arg.simplified() == "-l")
		{
			nextArgIsLanguage = true;
		}
		else if (nextArgIsLanguage)
		{
			language = arg.simplified();
			nextArgIsLanguage = false;
		}
		else
		{
			toOpen << arg;
		}
	}
	qApp->processEvents();
	//
	splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Interface translation"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	#ifdef Q_OS_WIN32
	QString defaultTranslationsPath = "/../translations";
	#else
	QString defaultTranslationsPath = "/../lib/qdevelop/translations";
	#endif
	QDir translationsDir(QCoreApplication::applicationDirPath() + defaultTranslationsPath);
	// load & install QDevelop translation
	translatorQDevelop.load(translationsDir.absoluteFilePath("QDevelop_"+language+".qm"));
	if (translatorQDevelop.isEmpty())
	{
		// Cmake workaround
		// (checking file existance is necessary as it will try loading other files, including "qdevelop" which might actually exist since
		//  it is the name of the Linux binary)
		if (QFile::exists(QCoreApplication::applicationDirPath() + "/QDevelop_"+language+".qm"))
			translatorQDevelop.load(QCoreApplication::applicationDirPath() + "/QDevelop_"+language+".qm");
		if (translatorQDevelop.isEmpty())
		{
			// Qmake workaround
			translatorQDevelop.load(QCoreApplication::applicationDirPath() 
				+ "/../resources/translations/QDevelop_"+language+".qm");
		}
	}
	if (!translatorQDevelop.isEmpty())
		app.installTranslator( &translatorQDevelop );
	// search, load & install Qt translation
	translatorQt.load(translationsDir.absoluteFilePath("qt_"+language+".qm"));
	if (translatorQt.isEmpty())
		translatorQt.load( QLibraryInfo::location( QLibraryInfo::TranslationsPath) + "/qt_"+QLocale::system().name()+".qm" );
	if (!translatorQt.isEmpty())
		app.installTranslator( &translatorQt );
	//
	MainImpl main;
	main.setGeometry(50,50, 800, 550);
	//
	QString projectName = main.loadINI();
	//
	splash->showMessage(QObject::tr("Environment control"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	main.slotToolsControl(false);
	//
	splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Files on editor"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
	qApp->processEvents();
	//
	if( !projectName.isEmpty() )
	{
		main.openProject( projectName );
	}
	foreach(QString s, toOpen)
	{
		if( s.right(4).toLower() == ".pro" )
		{
			if( s.toLower() != projectName.toLower() )
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
	main.show();
	splash->finish(&main);
	delete splash;
	splash = 0;
	//
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	main.checkQtDatabase();
	return app.exec();
}

