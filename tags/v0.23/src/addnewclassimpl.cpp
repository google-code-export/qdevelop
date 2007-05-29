#include "addnewclassimpl.h"
#include "projectmanager.h"
#include "misc.h"
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
//
AddNewClassImpl::AddNewClassImpl(ProjectManager * parent)
        : QDialog(0), m_projectManager(parent)
{
    setupUi(this);
}
//


void AddNewClassImpl::on_className_textChanged(QString )
{
    QString s = className->text().toLower();
    header->setText( s + ".h" );
    implementation->setText( s + ".cpp" );
    control();
}

void AddNewClassImpl::on_directoryButton_clicked()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the file location"),
                    location->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        // Cancel choosen
        return;
    }
    location->setText( s );
    control();
}
//
void AddNewClassImpl::control()
{
    bool enable = true;
    if ( className->text().isEmpty() )
        enable = false;
    if ( implementation->text().isEmpty() )
        enable = false;
    if ( header->text().isEmpty() )
        enable = false;
    okButton->setEnabled( enable );
}
//


void AddNewClassImpl::on_okButton_clicked()
{
    QByteArray templateData;
    QVariant variant = comboProjects->itemData( comboProjects->currentIndex() );
    QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
    QString projectDir = m_projectManager->projectDirectory( item );
    //
    QString impl = implementation->text();
    // filter out bad chars
    int j = impl.length();
    for ( int i=0; i < j; i++ )
    {
        if (!impl[i].isLetterOrNumber() && impl[i]!='.' )
            impl[i] = '_';
    }
    QString absoluteNameImpl = QDir( location->text() ).absoluteFilePath( impl ) ;
    QString relativeNameImpl = QDir( projectDir ).relativeFilePath( absoluteNameImpl );
    //
    QString head = header->text();
    // filter out bad chars
    j = head.length();
    for ( int i=0; i < j; i++ )
    {
        if (!head[i].isLetterOrNumber() && head[i]!='.' )
            head[i] = '_';
    }
    QString absoluteNameHeader = QDir( location->text() ).absoluteFilePath( head ) ;
    QString relativeNameHeader = QDir( projectDir ).relativeFilePath( absoluteNameHeader );
    //qDebug() << projectDir<<absoluteNameImpl <<relativeNameImpl <<absoluteNameHeader<<relativeNameHeader;
    //
    QFile file ( absoluteNameHeader );
    if ( file.exists() )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("The file")+" \""+absoluteNameHeader+"\"\n "+tr("already exist."),
                             tr("Cancel") );
        return;
    }
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("Unable to create file."),
                             tr("Cancel") );
        return;
    }
    file.write( templateHeaderImpl().toLatin1() );
    file.close();
    //
    QFile file2 ( absoluteNameImpl );
    if ( file2.exists() )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("The file")+" \""+absoluteNameImpl+"\"\n "+tr("already exist."),
                             tr("Cancel") );
        return;
    }
    if ( !file2.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("Unable to create file."),
                             tr("Cancel") );
        return;
    }
    file2.write( templateSourceImpl().toLatin1() );
    file2.close();
    m_projectManager->insertFile(item, relativeNameHeader);
    m_projectManager->insertFile(item, relativeNameImpl);
    m_projectManager->setQmake( m_projectManager->projectFilename( item ) );
    accept();
}
//
QString AddNewClassImpl::templateHeaderImpl()
{
    QString filename = header->text();
    QString classImpl = className->text();
    QFile file(":/templates/templates/newclassimpl.h");
    QString data;

    file.open(QIODevice::ReadOnly);
    data = file.readAll();
    file.close();

    data.replace("$IMPL_H", filename.toUpper().section(".H", 0, 0) +"_H");
    if ( checkboxInherits->isChecked() )
    {
        data.replace("$ANCESTORFILENAME", "#include <"+ ancestorFilename->text() +">");
        data.replace("$SCOPE", ": "+scope->currentText());
        data.replace("$PARENTNAME", ancestor->text());
        if ( !ancestor->text().length() || ancestor->text().left(1) != "Q" )
            data.replace("Q_OBJECT", "");
    }
    else
    {
        data.replace("$ANCESTORFILENAME", "");
        data.replace("$SCOPE", "");
        data.replace("$PARENTNAME", "");
        data.replace("Q_OBJECT", "");
    }
    if ( virtualDestructor->isChecked() )
        data.replace("$VIRTUAL", "virtual ~"+className->text()+"();");
    else
        data.replace("$VIRTUAL", "");
    data.replace("$ARGUMENTS", arguments->text());
    data.replace("$CLASSNAME", className->text());
    return data;
}
//
QString AddNewClassImpl::templateSourceImpl()
{
    QString filename = implementation->text();
    QString classImpl = className->text();
    QFile file(":/templates/templates/newclassimpl.cpp");
    QString data;

    file.open(QIODevice::ReadOnly);
    data = file.readAll();
    file.close();

    if ( checkboxInherits->isChecked() )
    {
        data.replace("$ANCESTOR", "\n\t: " + ancestor->text() + "()" );
    }
    else
    {
        data.replace("$ANCESTOR", "");
    }
    if ( virtualDestructor->isChecked() )
        data.replace("$VIRTUAL", className->text()+"::~"+className->text()+"()\n{\n}\n//");
    else
        data.replace("$VIRTUAL", "");
    data.replace("$HEADERNAME", "#include \""+header->text()+"\"");
    data.replace("$ARGUMENTS", arguments->text());
    data.replace("$CLASSNAME", className->text());
    return data;
}



void AddNewClassImpl::on_comboProjects_currentIndexChanged(int index)
{
    QVariant variant = comboProjects->itemData( index );
    QTreeWidgetItem *item = (QTreeWidgetItem*)(variantToItem( variant ) );
    QString projectDirectory = m_projectManager->srcDirectory( item );
    location->setText( projectDirectory );
}



