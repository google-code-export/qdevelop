#include "addnewclassmethodimpl.h"
#include "mainimpl.h"
#include "treeclasses.h"
#include "tabwidget.h"
#include "editor.h"
#include <QMessageBox>
#include <QComboBox>
#include <QDebug>
//
AddNewClassMethodImpl::AddNewClassMethodImpl( MainImpl * parent, TreeClasses *treeClasses, QString implementation, QString declaration, QString classname)
        : QDialog(parent), m_mainImpl(parent), m_treeClasses(treeClasses), m_implementation(implementation), m_declaration(declaration), m_classname(classname)
{
    setupUi(this);
    setWindowTitle ( tr("Add New Method in class") + " " + m_classname );
}
//

void AddNewClassMethodImpl::on_okButton_clicked()
{
    QString l_methodName = methodName->text().simplified();
    if( l_methodName.isEmpty() )
    	return;
    QString l_parameters = parameters->text().simplified();
    if ( !l_parameters.startsWith( "(" ) )
        l_parameters = "("+l_parameters;
    if ( !l_parameters.endsWith( ")" ) )
        l_parameters = l_parameters+")";
    QString l_scope = scope->currentText();
    QString l_returnType = returnType->currentText();
    bool l_Inline = Inline->isChecked();
    bool l_Virtual = Virtual->isChecked();
    bool l_Static = Static->isChecked();
    bool l_Pure = Pure->isChecked();
    if ( m_implementation.section("|", 0, 0).isEmpty() && !l_Inline && !l_Pure && l_scope.simplified() != "signals" )
    {
        QMessageBox::warning(this,
                             "QDevelop", tr("An implementation file is required."),
                             tr("Cancel") );
        return;
    }
    // Find if method already exists in class (same name and same signature).
    QStringList listMethods = m_treeClasses->methods(m_declaration.section("|", 0, 0), m_classname);
    foreach(QString s, listMethods)
    {
        QString sign = l_returnType+" "+l_methodName+l_parameters;
        if ( s.contains(l_methodName) && m_treeClasses->signature(s) == m_treeClasses->signature(sign) )
        {
            QMessageBox::warning(this,
                                 "QDevelop", l_methodName+m_treeClasses->signature(sign)+" "+tr("already exists in class") + " "+m_classname+".",
                                 tr("Cancel") );
            return;
        }
    }
    // First, add in declaration file or editor
    // The file is perhaps already opened. Find filename in tabs.
    QStringList lines;
    Editor *editor = 0;
    for (int i=0; i<m_mainImpl->tabEditors()->count(); i++)
    {
        if ( m_mainImpl->givenEditor(i)->filename() == m_declaration.section("|", 0, 0))
        {
            editor = m_mainImpl->givenEditor(i);
        }
    }
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
    for (int ind = m_declaration.section("|", 1, 1).toInt(); lines.count()>0, ind<lines.count(); ind++)
    {
        QString s = lines.at( ind );
        if ( s.simplified().startsWith( l_scope.simplified()+":" ) )
        {
            indexScope = ind+1;
            break;
        }
        else if ( s.simplified().startsWith( "{" ) )
        {
            indexBracket = ind+1;
        }
        else if ( s.simplified().startsWith( "Q_OBJECT" ) )
        {
            indexQ_OBJECT = ind+1;
        }
        else if ( s.simplified().startsWith( "class" ) )
        {
            // The begin of another class, stop find
            break;
        }
    }
    int insertAfterLine = indexScope;
    QString insertedText;
    if ( indexScope == -1 && indexQ_OBJECT != -1 )
    {
        insertAfterLine = indexQ_OBJECT;
        insertedText = l_scope.simplified() + ":" + "\n";
    }
    else if ( indexScope == -1 && indexBracket != -1 )
    {
        insertAfterLine = indexBracket;
        insertedText = l_scope.simplified() + ":" + "\n";
    }
    if ( l_Static )
        insertedText += "\tstatic ";
    else if ( l_Virtual )
        insertedText += "\tvirtual ";
    else
        insertedText += "\t";
    insertedText += l_returnType + " " + l_methodName + l_parameters;
    if ( l_Pure )
        insertedText += " = 0";
    else if ( l_Inline )
        insertedText += " { }";
    insertedText += ";";
    if ( editor )
    {
        insertedText += "\n";
        editor->insertText(insertedText, insertAfterLine+1);
    }
    else
    {
        foreach(QString s, insertedText.split("\n") )
        {
        	lines.insert(insertAfterLine++, s);
       	}
        QFile file(m_declaration.section("|", 0, 0));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write( lines.join("\n").toLocal8Bit()  );
        file.close();
        m_mainImpl->slotUpdateClasses(m_declaration.section("|", 0, 0), lines.join("\n").toLocal8Bit());
    }
    if ( l_Pure || l_Inline || l_scope.simplified() == "signals" )
        accept();
    // Now, add in implementation file or editor
    if ( l_parameters.contains( "=" ) )
    {
        QString formattedParams;
        foreach(QString param, l_parameters.remove("(").remove(")").split(",") )
        {
            param = param.simplified();
            param = param.section("=", 0, 0);
            formattedParams += " " + param + ",";
        }
        formattedParams = formattedParams.simplified().left( formattedParams.simplified().lastIndexOf(",") );
        l_parameters ="(" + formattedParams + ")";
    }
    editor = 0;
    for (int i=0; i<m_mainImpl->tabEditors()->count(); i++)
    {
        if ( m_mainImpl->givenEditor(i)->filename() == m_implementation.section("|", 0, 0))
        {
            editor = m_mainImpl->givenEditor(i);
        }
    }
    insertedText = l_returnType + " " + m_classname + "::" + l_methodName + l_parameters;
    insertedText += "\n{\n\t// TODO\n}\n";
    if ( editor )
    {
        // Write content in opened editor
        lines = editor->toPlainText().split("\n");
        insertedText += "\n";
        editor->insertText(insertedText, -1);
    }
    else
    {
        // The file is not opened, write content in file on disk
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
    accept();
}

