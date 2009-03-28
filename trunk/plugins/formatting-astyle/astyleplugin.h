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
#ifndef ASTYLEPLUGIN_H
#define ASTYLEPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QTranslator>

#include <pluginsinterfaces.h>

class AStylePlugin : public QObject, public TextEditInterface
{
    Q_OBJECT
    Q_INTERFACES(TextEditInterface)

public:
    AStylePlugin() : translator(0) {}
    QString menuName() const;
    QString text(QString text, QString selectedText, QTextCodec *codec); 
    TextEditInterface::Action action() const;
    bool hasConfigDialog() const;
    void config();
private:
    mutable QTranslator * translator;
};

#endif
