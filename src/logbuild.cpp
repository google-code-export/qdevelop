#include "logbuild.h"
#include "mainimpl.h"
#include <QDir>
#include <QDebug>
//
LogBuild::LogBuild( QWidget * parent )
        : QTextEdit(parent)
{}
//
void LogBuild::mouseDoubleClickEvent( QMouseEvent * event )
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
                m_mainImpl->incErrors();
            }
            else if ( message.toLower().contains( "warning") || message.toLower().contains( tr("warning").toLower() ) )
            {
                setTextColor( Qt::blue );
                m_mainImpl->incWarnings();
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
    ensureCursorVisible();
}
//
