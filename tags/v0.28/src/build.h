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

#ifndef BUILD_H
#define BUILD_H

#include <QThread>
#include <QDateTime>
#include <QStringList>
//
class QProcess;

class Build : public QThread
{
Q_OBJECT
public:
	Build(QObject * parent, QString qmakeName, QString makename, QString makeOptions, QString absoluteProjectName, bool qmake, 
		bool n, bool g, QString compileFile=0, QString forceMode = QString());
    void run();
    int nbErrors() { return m_errors; }
    int nbWarnings() { return m_warnings; }
private:
	bool m_qmake;
	QString m_projectDirectory;
	QString m_projectName;
	bool m_clean;
	bool m_build;
	bool m_isStopped;
	QString m_compileFile;
	QProcess *m_buildProcess;
	QString buildOnly( QString sourceFile );
	QString m_qmakeName;
	QString m_makeName;
	QString m_makeOptions;
	int m_errors;
	int m_warnings;
	QString m_forceMode;
signals:
	void message(QString, QString=0);
protected slots:
	void slotBuildMessages();
	void slotStopBuild();
public slots:
    void slotIncErrors();
    void slotIncWarnings();
};

#endif
