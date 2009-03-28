/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2009  Dmitrij "Divius" Tantsur
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
#ifndef REPLUGIN_H
#define REPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QTranslator>

#include <pluginsinterfaces.h>

class RePlugin : public QObject, public SimplePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(SimplePluginInterface)

public:
    RePlugin() : translator(0) {}
    QString menuName() const;
    void start(QWidget * owner = 0);
    bool hasConfigDialog() const;
    void config();
private:
    mutable QTranslator * translator;
};

#endif
