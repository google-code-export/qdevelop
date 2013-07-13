/*
* This file is part of QMagneto, an EPG (Electronic Program Guide)
* Copyright (C) 2008-2010  Jean-Luc Biord
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
* Program URL   : http://biord-software.org/qmagneto/
*
*/

#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include "getlastversion.h"
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

GetLastVersion::GetLastVersion(QWidget *parent, const QUrl &url)
: m_parent(parent)
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(slotFinished(QNetworkReply*)));
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
}

void GetLastVersion::slotFinished(QNetworkReply *reply)
{
#ifdef false
    if(!reply->error())
    {
		QString version = QString(reply->readAll()).simplified();
		if( QString(version) != QString(VERSION) )
		{
			 QMessageBox messageBox(m_parent);
			 messageBox.setWindowTitle("QMagneto");
			 messageBox.setIcon(QMessageBox::Information);
			 messageBox.setText(tr("A new version %1 is available on http://biord-software.org.").arg(version));
			 QAbstractButton *gotosite = (QAbstractButton *) messageBox.addButton(tr("Go to site"), QMessageBox::ActionRole);
			 QAbstractButton *close = (QAbstractButton *) messageBox.addButton(tr("Close"), QMessageBox::ActionRole);

			 messageBox.exec();
			 if (messageBox.clickedButton() == gotosite) 
			 {
			 	QDesktopServices::openUrl( QUrl("http://biord-software.org") );
			 }
 		}
    }
#endif
    reply->deleteLater();
}
