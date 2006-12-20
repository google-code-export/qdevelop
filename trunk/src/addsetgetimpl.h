#ifndef ADDSETGETIMPL_H
#define ADDSETGETIMPL_H
//
#include "ui_addsetget.h"
//
class MainImpl;
class TreeClasses;
class QTreeWidgetItem;
//
class AddSetGetImpl : public QDialog, public Ui::AddSetGet
{
    Q_OBJECT
public:
    AddSetGetImpl(MainImpl * parent, TreeClasses *treeClasses, QString declaration, QString implementation, QString classname, QString type, QString variableName);
private slots:
    void on_okButton_clicked();
private:
    MainImpl *m_mainImpl;
    TreeClasses *m_treeClasses;
    QString m_declaration;
    QString m_implementation;
    QString m_classname;
    QString m_type;
    QString m_variableName;
	void insertInDeclaration(QString scope, QString insertedText);
	void insertInImplementation(QString insertedText);
};
#endif





