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

#include "addnewclassvariableimpl.h"
#include "mainimpl.h"
#include "treeclasses.h"
#include "tabwidget.h"
#include "editor.h"
#include <QMessageBox>
#include <QComboBox>
#include <QTreeWidgetItem>
//
AddNewClassVariableImpl::AddNewClassVariableImpl( MainImpl * parent, TreeClasses *treeClasses, QTreeWidgetItem *treeWidget, QString declaration, QString implementation, QString classname)
        : QDialog(parent), m_mainImpl(parent), m_treeClasses(treeClasses), m_treeWidget(treeWidget), m_declaration(declaration), m_implementation(implementation), m_classname(classname)
{
    setupUi(this);
    scope->addItems(QStringList() << "public" << "protected" << "private");
    type->addItems(QStringList() << "bool" << "char" << "short"
    	<< "long" << "int" << "signed"
    	<< "unsigned " << "float" << "double"
    	<< "QString" << "void"
    );
}
//

void AddNewClassVariableImpl::on_okButton_clicked()
{
    if ( get->isChecked() && getName->text().isEmpty() )
        {
            QMessageBox::warning(this,
                                 "QDevelop", "The get method name is empty",
                                 tr("Cancel") );
            return;
        }
    if ( set->isChecked() && setName->text().isEmpty() )
        {
            QMessageBox::warning(this,
                                 "QDevelop", "The set method name is empty",
                                 tr("Cancel") );
            return;
        }

    QString l_variableName = variableName->text();
    QString l_type = type->currentText();
    QString l_scope = scope->currentText();
    for (int i=0; i<m_treeWidget->childCount(); i++)
    {
        ParsedItem parsedItem = m_treeWidget->child( i )->data(0, Qt::UserRole).value<ParsedItem>();
        if ( parsedItem.name == l_variableName )
        {
            QMessageBox::warning(this,
                                 "QDevelop", tr("%1 already exists in class %2.").arg(l_variableName, m_classname),
                                 tr("Cancel") );
            return;
        }
    }
    // Add in declaration file or editor
    // The file is perhaps already opened. Find filename in tabs.
    QString insertedText = "\t" + l_type + " " + l_variableName + ";";
    insertInDeclaration(l_scope, insertedText);
    //insertedText += "\t" + l_type + " " + l_variableName + ";";
    // Add get/set methods
    if ( get->isChecked() )
        {
            if ( getInline->isChecked() || m_implementation.isEmpty() )
            {
                insertedText = "\t" + l_type + " " + getName->text() + "() { return " + l_variableName + "; }";
                insertInDeclaration("public", insertedText);
            }
            else
            {
                insertedText = "\t" + l_type + " " + getName->text() + "();";
                insertInDeclaration("public", insertedText);
                insertedText = l_type + " " + m_classname+"::" + getName->text() + "()\n{\n\treturn " + l_variableName + ";\n}\n";
                insertInImplementation(insertedText);
           	}
        }
    if ( set->isChecked() )
        {
            if ( setInline->isChecked() || m_implementation.isEmpty() )
            {
                insertedText = "\tvoid " + setName->text() + "("+l_type+" value) { " + l_variableName + " = value; }";
                insertInDeclaration("public", insertedText);
            }
            else
            {
                insertedText = "\tvoid " + setName->text() + "("+l_type+" value);";
                insertInDeclaration("public", insertedText);
                insertedText = "void " + m_classname+"::"+setName->text() + "("+l_type+" value)\n{\n\t" + l_variableName + " = value;\n}\n";
                insertInImplementation(insertedText);
            }
        }
    accept();
}

//
void AddNewClassVariableImpl::insertInDeclaration(QString scope, QString insertedText)
{
    QStringList lines;
    Editor *editor = 0;
    foreach(Editor *ed, m_mainImpl->allEditors() )
    {
        if ( ed->filename() == m_declaration.section("|", 0, 0))
        {
            editor = ed;
        }
    }
    //
    if ( editor )
    {
        // Get content of opened editor
        lines = editor->toPlainText().split("\n");
    }
    else
    {
        // The file is not opened, get content from file
        QFile file(m_declaration.section("|", 0, 0));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        lines = QString(file.readAll()).split("\n");
        file.close();
    }
    int indexScope = -1;
    int indexBracket = -1;
    int indexQ_OBJECT = -1;
    for (int ind = m_declaration.section("|", 1, 1).toInt(); lines.count()>0 && ind<lines.count(); ind++)
    {
        QString s = lines.at( ind );
        if ( s.remove(" ").startsWith( scope.simplified()+":" ) )
        {
            indexScope = ind+1;
            break;
        }
        else if ( s.remove(" ").startsWith( "{" ) )
        {
            indexBracket = ind+1;
        }
        else if ( s.remove(" ").startsWith( "Q_OBJECT" ) )
        {
            indexQ_OBJECT = ind+1;
        }
        else if ( s.remove(" ").startsWith( "class" ) )
        {
            // The begin of another class, stop find
            break;
        }
    }
    int afterLine = indexScope;
    if ( indexScope == -1 && indexQ_OBJECT != -1 )
    {
        afterLine = indexQ_OBJECT;
        insertedText = scope.simplified() + ":" + "\n" + insertedText;
    }
    else if ( indexScope == -1 && indexBracket != -1 )
    {
        afterLine = indexBracket;
        insertedText = scope.simplified() + ":" + "\n" + insertedText;
    }
    if ( editor )
    {
        insertedText += "\n";
        editor->insertText(insertedText, afterLine+1);
    }
    else
    {
        foreach(QString s, insertedText.split("\n") )
        {
            lines.insert(afterLine++, s);
        }
        QFile file(m_declaration.section("|", 0, 0));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write( lines.join("\n").toLocal8Bit()  );
        file.close();
        m_mainImpl->slotUpdateClasses(m_declaration.section("|", 0, 0), lines.join("\n").toLocal8Bit());
    }
}
//
void AddNewClassVariableImpl::insertInImplementation(QString insertedText)
{
    QStringList lines;
    Editor *editor = 0;
    foreach(Editor *ed, m_mainImpl->allEditors() )
    {
        if ( ed->filename() == m_implementation.section("|", 0, 0))
        {
            editor = ed;
        }
    }
    //
    if ( editor )
    {
        // Get content of opened editor
        insertedText += "\n";
        editor->insertText(insertedText, -1);
    }
    else
    {
        // The file is not opened, get content from file
        QFile file(m_implementation.section("|", 0, 0));
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        lines = QString(file.readAll()).split("\n");
        file.close();
        foreach(QString s, insertedText.split("\n") )
        {
        	lines.append(s);
       	}
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write( lines.join("\n").toLocal8Bit()  );
        file.close();
        m_mainImpl->slotUpdateClasses(m_implementation.section("|", 0, 0), lines.join("\n").toLocal8Bit());
    }
}

void AddNewClassVariableImpl::on_variableName_textChanged(QString )
{
    setName->setText( "set"+variableName->text().toUpper().left(1)+variableName->text().mid(1) );
    getName->setText( "get"+variableName->text().toUpper().left(1)+variableName->text().mid(1) );
}


