#ifndef ADDNEWCLASSMETHODIMPL_H
#define ADDNEWCLASSMETHODIMPL_H
//
#include "ui_addnewclassmethod.h"
//
class MainImpl;
class TreeClasses;
//
class AddNewClassMethodImpl : public QDialog, public Ui::AddNewClassMethod
{
Q_OBJECT
public:
	AddNewClassMethodImpl( MainImpl * parent, TreeClasses *treeClasses, QString implementation, QString declaration, QString classname);
private slots:
	void on_okButton_clicked();
private:
	MainImpl *m_mainImpl;
	TreeClasses *m_treeClasses;
	QString m_implementation;
	QString m_declaration;
	QString m_classname;
};
#endif





