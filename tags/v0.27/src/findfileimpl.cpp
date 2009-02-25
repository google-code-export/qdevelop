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

#include "findfileimpl.h"
#include <QFileDialog>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

//
FindFileImpl::FindFileImpl( QWidget * parent, QStringList directories, QListWidget *listResult, QListWidget *findLines)
        : QDialog(parent), m_parent(parent), m_listResult(listResult), m_listLines(findLines)
{
    setupUi(this);

    // BK - store last entered search on the top.
    textFind->setInsertPolicy(QComboBox::InsertAtTop);
    textReplace->setInsertPolicy(QComboBox::InsertAtTop);

    for (int i=0; i<directories.count(); i++)
        comboFindIn->addItem(directories.at(i)+"/");
        
    //initialize the variable used to remember where we are in the recursive search algorithm
    m_recursiveDepth=0;
}

//
void FindFileImpl::setDefaultWord( QString s) {
    defaultWord=s;
}

//
void FindFileImpl::showEvent(QShowEvent* _pEvent)
{
    // BK - put focus on textFind and select previous search word
    textFind->setFocus();
    QLineEdit* pLineEdit = textFind->lineEdit();
    
    //set the default word if existing
    if(!defaultWord.isEmpty()) {
        pLineEdit->setText(defaultWord);
		defaultWord.clear();
    }
    
    //select the displayed word
    pLineEdit->setSelection(0, pLineEdit->text().count());

    QWidget::showEvent(_pEvent);
}
//
void FindFileImpl::on_chooseDirectoryButton_clicked()
{
    QString s = QFileDialog::getExistingDirectory(
                    m_parent,
                    tr("Choose a directory"),
                    QDir::homePath(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    comboFindIn->addItem(s);
    comboFindIn->setCurrentIndex( comboFindIn->count()-1 );
}
//
void FindFileImpl::on_findButton_clicked()
{
    // BUG when button Find/Close is pressed, textFind doesn't remember last entered string.

    if ( !comboFindIn->count() || textFind->currentText().isEmpty() )
        return;
    if ( findButton->text() == tr("&Find") )
    {
        if ( textFind->findText( textFind->currentText() ) == -1 )
        {
            textFind->addItem( textFind->currentText() );
        }
        findButton->setText(tr("&Stop"));
        closeButton->setEnabled( false );
        m_stop = false;
        m_listResult->clear();
        m_listLines->clear();
    }
    else
    {
        findButton->setText(tr("&Find"));
        closeButton->setEnabled( true );
        m_stop = true;
    }
    find( comboFindIn->currentText() );
    if ( m_listResult->count() )
    {
        m_listResult->setItemSelected( m_listResult->item(0), true);
        m_listLines->addItems( m_listResult->item(0)->data(Qt::UserRole).toStringList() );
    }
    findButton->setText(tr("&Find"));
    closeButton->setEnabled( true );
}
//
void FindFileImpl::find( QString directory )
{
    //save the folder only if the search is just beginning
    if(m_recursiveDepth==0) {
        m_searchDirectory=directory;
    }
    m_recursiveDepth++; //search is one-level deeper
    
    QDir dir(directory);
    QString filterNames = comboFileTypes->currentText();
    filterNames.remove(" ");
    QFileInfoList list = dir.entryInfoList(filterNames.split(";"), QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        qApp->processEvents();
        if ( m_stop )
            return;
        if ( fileInfo.isFile() )
        {
            findInFile( fileInfo.absoluteFilePath() );
        }
        else if ( checkRecursive->isChecked() )// Directory
        {
            find( fileInfo.absoluteFilePath() );
        }
    }
    
    //search is one-level less deep
    m_recursiveDepth--;
}
//
void FindFileImpl::findInFile( QString filename )
{
    QFile file(filename);
    bool replace = checkReplace->isChecked();
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    QString tempFileName = QDir::tempPath() + QDir::separator() + 
    	"qdtemp-" + QDateTime::currentDateTime().toString("yyMMddmmss") +
    	QFileInfo(filename).fileName();
    
    QFile tempFile(tempFileName);
    QTextStream tempStream;
    if (replace)
    {
    	if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    	{
    		qDebug() << tr("Cannot open temporary file %1").arg(tempFileName);
    		replace = false;
   		}
   		else
   		{
   			tempStream.setDevice(&tempFile);
  		}
   	}
   	
    //
    QRegExp exp;
    if ( checkWholeWords->isChecked() )
        exp.setPattern( "\\b"+textFind->currentText()+"\\b");
    else
        exp.setPattern( textFind->currentText());
    exp.setCaseSensitivity((Qt::CaseSensitivity) checkCase->isChecked());

    //
    QTextStream in(&file);
    int numLine = 0;
    QStringList lines;
    while (!in.atEnd())
    {
        numLine++;
        QString line = in.readLine();
        
        if (replace)
        {
        	line = line.replace(exp, textReplace->currentText());
        	tempStream << line << endl;
       	}
        
        if ( line.contains(exp) )
        {
            lines << tr("Line")+" "+QString::number(numLine)+" : "+line;
        }
    }
    if (tempFile.isOpen()) tempFile.close();
    file.close();
    
    if (replace)
    {
    	QFile::remove(filename);
    	QFile::copy(tempFileName, filename);
    	QFile::remove(tempFileName);
   	}
   	
   	if ( lines.count() )
    {
        //display only the relative path
        m_listResult->addItem( tr("File")+" : "+filename.mid(m_searchDirectory.size()) );
        QListWidgetItem *item = m_listResult->item(m_listResult->count()-1);
        item->setData(Qt::UserRole, QVariant(lines) );
        
        //complete filename (used in slotDoubleClickFindLines)
        item->setData(Qt::UserRole+1, QVariant(filename) );
    }
}
//
