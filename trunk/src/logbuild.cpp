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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/

#include "logbuild.h"
#include "mainimpl.h"
#include <QDir>
#include <QDebug>
//
LogBuild::LogBuild( QWidget * parent )
        : QTextEdit(parent)
{}
//
void LogBuild::mouseDoubleClickEvent( QMouseEvent * /*event*/ )
{
    // First highlight the line double-clicked
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    setTextCursor( cursor );
    //
    BlockLogBuild *blockUserData = (BlockLogBuild*)cursor.block().userData();
    // If blockUserData is null, the line doesn't contains "error" or "warning", we quit this function.
    if ( !blockUserData )
        return;
    QString projectDirectory = blockUserData->directory();
    QString text = cursor.block().text();
    if ( !text.contains("error") && !text.contains("warning")
            // Modify the two strings below "error" and "warning" to adapt in your language.
            && !text.contains( tr("error").toLower() ) && !text.contains( tr("warning").toLower() ) )
        return;
    QString filename = text.section(":", 0, 0).replace("\\", "/").replace("//", "/");
    int numLine = text.section(":", 1, 1).toInt();
    if ( numLine == 0 )
        return;
    QString absoluteName = QDir(projectDirectory+"/"+filename).absolutePath();
    m_mainImpl->openFile( QStringList( absoluteName ), numLine);
}
//
void LogBuild::slotMessagesBuild(QString list, QString directory)
{
    /*  If your language is not translated in QDevelop and if g++ display the errors and warnings in your language, 
    modify the two strings below "error" and "warning" to adapt in your language.*/
    foreach(QString message, list.split("\n"))
    {
        if ( !message.isEmpty() )
        {
            message.remove( "\r" );
            setTextColor( Qt::black );
            if ( (message.toLower().contains("error") || message.toLower().contains( tr("error").toLower() ))
            	&& !message.contains("------") )
            {
                setTextColor( Qt::red );
                m_mainImpl->resetProjectsDirectoriesList();
                m_mainImpl->resetDebugAfterBuild();
                emit incErrors();
            }
            else if ( message.toLower().contains( "warning") || message.toLower().contains( tr("warning").toLower() ) )
            {
                setTextColor( Qt::blue );
                emit incWarnings();
            }
            append( message );
            if ( !directory.isEmpty() )
            {
                QTextCursor cursor = textCursor();
                BlockLogBuild *blockUserData = new BlockLogBuild(directory);
                cursor.block().setUserData( blockUserData );
                setTextCursor( cursor );
            }
        }
    }
    // Move the cursor to the bottom. Ensure it is visible.
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    setTextCursor( cursor );
}
//
