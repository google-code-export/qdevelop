#include "addsetgetimpl.h"
#include "mainimpl.h"
#include "treeclasses.h"
#include "tabwidget.h"
#include "editor.h"
#include <QMessageBox>
#include <QComboBox>
#include <QTreeWidgetItem>
#include <QDebug>
//
AddSetGetImpl::AddSetGetImpl(MainImpl * parent, TreeClasses *treeClasses, QString declaration,  QString implementation, QString classname, QString type, QString variableName)
        : QDialog(parent), m_mainImpl(parent), m_treeClasses(treeClasses), m_declaration(declaration),  m_implementation(implementation), m_classname(classname), m_type(type), m_variableName(variableName)
{
    setupUi(this);
    groupBox->setTitle( groupBox->title() + " " + m_variableName );
    setName->setText( "set"+m_variableName.toUpper().left(1)+m_variableName.mid(1) );
    getName->setText( "get"+m_variableName.toUpper().left(1)+m_variableName.mid(1) );
}
//

void AddSetGetImpl::on_okButton_clicked()
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

    // Add get/set methods
    QString insertedText;
    if ( get->isChecked() )
        {
            if ( getInline->isChecked() || m_implementation.isEmpty()  )
            {
                insertedText = "\t" + m_type + " " + getName->text() + "() { return " + m_variableName + "; }";
                insertInDeclaration("public", insertedText);
            }
            else
            {
                insertedText = "\t" + m_type + " " + getName->text() + "();";
                insertInDeclaration("public", insertedText);
                insertedText = m_type + " " + m_classname+"::" + getName->text() + "()\n{\n\treturn " + m_variableName + ";\n}\n";
                insertInImplementation(insertedText);
            }
        }
    if ( set->isChecked() )
        {
            if ( setInline->isChecked() || m_implementation.isEmpty() )
            {
                insertedText = "\tvoid " + setName->text() + "( "+m_type+" value) { " + m_variableName + " = value; }";
                insertInDeclaration("public", insertedText);
            }
            else
            {
                insertedText = "\tvoid " + setName->text() + "( "+m_type+" value);";
                insertInDeclaration("public", insertedText);
                insertedText = "void " + m_classname+"::"+setName->text() + "("+m_type+" value)\n{\n\t" + m_variableName + " = value;\n}\n";
                insertInImplementation(insertedText);
            }
        }
    accept();
}
//
void AddSetGetImpl::insertInDeclaration(QString scope, QString insertedText)
{
    QStringList lines;
    Editor *editor = 0;
    for (int i=0; i<m_mainImpl->tabEditors()->count(); i++)
    {
        if ( m_mainImpl->givenEditor(i)->filename() == m_declaration.section("|", 0, 0))
        {
            editor = m_mainImpl->givenEditor(i);
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
    for (int ind = m_declaration.section("|", 1, 1).toInt(); lines.count()>0, ind<lines.count(); ind++)
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
void AddSetGetImpl::insertInImplementation(QString insertedText)
{
    QStringList lines;
    Editor *editor = 0;
    for (int i=0; i<m_mainImpl->tabEditors()->count(); i++)
    {
        if ( m_mainImpl->givenEditor(i)->filename() == m_implementation.section("|", 0, 0))
        {
            editor = m_mainImpl->givenEditor(i);
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

