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
#include "newprojectimpl.h"
#include "projectmanager.h"
#include "mainimpl.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
//
NewProjectImpl::NewProjectImpl(QWidget * parent, QString s)
        : QDialog(0), m_projectLocation(s)
{
    setupUi(this);
    m_mainImpl = (MainImpl *)parent;
    connect(locationButton, SIGNAL(clicked()), this, SLOT(slotChooseDirectory()) );
    connect(projectName, SIGNAL(textChanged(QString)), this, SLOT(slotLabel()) );
    connect(location, SIGNAL(textChanged(QString)), this, SLOT(slotLabel()) );
    location->setText( s );

    // TODO remove gcc warnings
    parent = NULL;
}
//
void NewProjectImpl::slotChooseDirectory()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the project directory"),
                    m_projectLocation,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel clicked
        return;
    }
    location->setText( s );
}
//
void NewProjectImpl::slotLabel()
{
    //label->setText( label->text().section(":", 0, 0) + ": "	+ location->text() + "/" + projectName->text().toLower().remove(".pro") );
    if ( projectName->text().isEmpty() || location->text().isEmpty() )
    {
        okButton->setEnabled( false );
        label->setText( label->text().section(":", 0, 0) + ": "	);
    }
    else
    {
        okButton->setEnabled( true );
        label->setText( label->text().section(":", 0, 0) + ": "	+ location->text() + "/" + QFileInfo(projectName->text()).baseName() );
    }
}

void NewProjectImpl::on_dialog_clicked(bool checked)
{
    if (!checked)
        return;

    uiFilename->setText( "dialog" );
    subclassFilename->setText( "dialogimpl" );
    uiObjectName->setText( "Dialog" );
    subclassObjectName->setText( "DialogImpl" );
}

void NewProjectImpl::on_mainwindow_clicked(bool checked)
{
    if (!checked)
        return;

    uiFilename->setText( "mainwindow" );
    subclassFilename->setText( "mainwindowimpl" );
    uiObjectName->setText( "MainWindow" );
    subclassObjectName->setText( "MainWindowImpl" );
}

void NewProjectImpl::on_okButton_clicked()
{
    bool isSubProject = false;
    if ( !parentProjectName->text().isEmpty() )
        isSubProject = true;
    if ( !isSubProject && !m_mainImpl->slotCloseProject() )
    {
        reject();
    }
    m_filename = projectName->text();
    if ( !m_filename.toLower().contains( ".pro" ) )
        m_filename += ".pro";
    QString projectDirectory = location->text();
    projectDirectory += "/" + m_filename.left( m_filename.lastIndexOf(".") );
    QString l_srcDirectory = projectDirectory+"/"+srcDirectory->text();
    QString l_uiDirectory = projectDirectory+"/"+uiDirectory->text();
    QString l_buildDirectory = buildDirectory->text();
    QString l_binDirectory = binDirectory->text();
    QString l_uiFilename = uiFilename->text();
    QString l_uiObjectName = uiObjectName->text();
    QString l_subclassFilename = subclassFilename->text();
    QString l_subclassObjectName = subclassObjectName->text();
    m_absoluteProjectName = projectDirectory + "/" + m_filename ;
    QDir dir;
    if ( !dir.mkdir(projectDirectory) )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("The directory \"%1\" cannot be created").arg(projectDirectory),
                             tr("Cancel") );
        return;
    }
    QFile projectFile ( m_absoluteProjectName );
    if ( !projectFile.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("The project cannot be created"),
                             tr("Cancel") );
        return;
    }
    else
    {
        QByteArray projectFileContent;
        if ( !empty->isChecked() )
        {

            projectFileContent += "TEMPLATE = app\n";
            projectFileContent += "QT = gui ";
            projectFileContent += "core\n";
            projectFileContent += "CONFIG += qt ";
            QString version = "debug";
            if ( release->isChecked() )
                projectFileContent += "release ";
            else
                projectFileContent += "debug ";
            projectFileContent += "warn_on ";
            projectFileContent += "console\n";
        }
        if ( dialog->isChecked() || mainwindow->isChecked() )
        {
            if ( !l_srcDirectory.isEmpty() )
            {
                QDir().mkdir( l_srcDirectory );
            }
            if ( !l_uiDirectory.isEmpty() )
            {
                QDir().mkdir( l_uiDirectory );
            }
            if ( !l_binDirectory.isEmpty() )
            {
                projectFileContent += "DESTDIR = "+l_binDirectory+"\n";
            }
            if ( !l_buildDirectory.isEmpty() )
            {
                projectFileContent += "OBJECTS_DIR = "+l_buildDirectory+"\n";
                projectFileContent += "MOC_DIR = "+l_buildDirectory+"\n";
                projectFileContent += "UI_DIR = "+l_buildDirectory+"\n";
            }
            if ( dialog->isChecked() )
            {
                QFile file(":/templates/templates/dialog.ui");
                file.open(QIODevice::ReadOnly);
                QByteArray data = file.readAll();
                file.close();
                data.replace("<class>Dialog</class>", "<class>"+l_uiObjectName.toAscii()+"</class>");
                data.replace("name=\"Dialog\"", "name=\""+l_uiObjectName.toAscii()+"\"");
                QFile uiFile(l_uiDirectory + "/" + l_uiFilename.section(".ui", 0, 0) + ".ui");
                uiFile.open(QIODevice::WriteOnly);
                uiFile.write( data );
                uiFile.close();
                projectFileContent+= "FORMS = "+QDir(projectDirectory).relativeFilePath(l_uiDirectory + "/" + l_uiFilename.section(".ui", 0, 0) + ".ui") + "\n";
            }
            else if ( mainwindow->isChecked() )
            {
                QFile file(":/templates/templates/mainwindow.ui");
                file.open(QIODevice::ReadOnly);
                QByteArray data = file.readAll();
                file.close();
                data.replace("<class>MainWindow</class>", "<class>"+l_uiObjectName.toAscii()+"</class>");
                data.replace("name=\"MainWindow\"", "name=\""+l_uiObjectName.toAscii()+"\"");
                QFile uiFile(l_uiDirectory + "/" + l_uiFilename.section(".ui", 0, 0) + ".ui");
                uiFile.open(QIODevice::WriteOnly);
                uiFile.write( data );
                uiFile.close();
                projectFileContent+= "FORMS = "+QDir(projectDirectory).relativeFilePath(l_uiDirectory + "/" + l_uiFilename.section(".ui", 0, 0) + ".ui") + "\n";
            }
            // Create subclassing header
            QFile file(":/templates/templates/impl.h.template");
            file.open(QIODevice::ReadOnly);
            QByteArray data = file.readAll();
            file.close();
            data.replace("$IMPL_H", QString( l_subclassFilename.section(".h", 0, 0).toUpper()+"_H" ).toAscii());
            data.replace("$UIHEADERNAME", QString( "\"ui_"+l_uiFilename.section(".ui", 0, 0)+".h\"").toAscii());
            data.replace("$CLASSNAME", QString( l_subclassObjectName ).toAscii());
            if ( dialog->isChecked() )
                data.replace("$PARENTNAME", QString( "QDialog" ).toAscii());
            else
                data.replace("$PARENTNAME", QString( "QMainWindow" ).toAscii());
            data.replace("$OBJECTNAME", QString( l_uiObjectName ).toAscii());
            QFile headerFile(l_srcDirectory + "/" + l_subclassFilename + ".h");
            headerFile.open(QIODevice::WriteOnly);
            headerFile.write( data );
            headerFile.close();
            projectFileContent += "HEADERS = "+ QDir(projectDirectory).relativeFilePath(l_srcDirectory + "/" + l_subclassFilename + ".h") + "\n";
            // Create subclassing sources
            QFile file2(":/templates/templates/impl.cpp.template");
            file2.open(QIODevice::ReadOnly);
            data = file2.readAll();
            file2.close();
            QFile sourceFile(l_srcDirectory + "/" + l_subclassFilename + ".cpp");
            data.replace("$HEADERNAME", QString( "\""+l_subclassFilename+".h\"" ).toAscii());
            data.replace("$CLASSNAME", QString( l_subclassObjectName ).toAscii());
            if ( dialog->isChecked() )
                data.replace("$PARENTNAME", QString( "QDialog" ).toAscii());
            else
                data.replace("$PARENTNAME", QString( "QMainWindow" ).toAscii());
            sourceFile.open(QIODevice::WriteOnly);
            sourceFile.write( data );
            sourceFile.close();
            projectFileContent += "SOURCES = "+ QDir(projectDirectory).relativeFilePath(l_srcDirectory + "/" + l_subclassFilename + ".cpp")+" ";
            // Create main.cpp
            QFile file3(":/templates/templates/main.cpp.template");
            file3.open(QIODevice::ReadOnly);
            data = file3.readAll();
            file3.close();
            QFile mainFile(l_srcDirectory + "/" + "main.cpp");
            data.replace("$HEADERNAME", QString( "\""+l_subclassFilename+".h\"" ).toAscii());
            data.replace("$CLASSNAME", QString( l_subclassObjectName ).toAscii());
            mainFile.open(QIODevice::WriteOnly);
            mainFile.write( data );
            mainFile.close();
            projectFileContent += QDir(projectDirectory).relativeFilePath(l_srcDirectory + "/" + "main.cpp") + "\n";
        }
        //
        projectFile.write( projectFileContent );
        projectFile.close();
        accept();
    }
}
