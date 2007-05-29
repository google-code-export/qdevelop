#ifndef ADDNEWCLASSVARIABLEIMPL_H
#define ADDNEWCLASSVARIABLEIMPL_H
//
#include "ui_addnewclassvariable.h"
//
class MainImpl;
class TreeClasses;
class QTreeWidgetItem;
class Editor;
//
class AddNewClassVariableImpl : public QDialog, public Ui::AddNewClassVariable
{
Q_OBJECT
public:
	AddNewClassVariableImpl( MainImpl * parent, TreeClasses *treeClasses, QTreeWidgetItem *treeWidget, QString declaration, QString implementation, QString classname);
	bool addGet() { return get->isChecked(); }
	QString addGetName() { return getName->text(); }
	bool addGetInline() { return getInline->isChecked(); }
	bool addSet() { return set->isChecked(); }
	QString addSetName() { return setName->text(); }
	bool addSetInline() { return setInline->isChecked(); }
private slots:
	void on_variableName_textChanged(QString );
	void on_okButton_clicked();
private:
	MainImpl *m_mainImpl;
	TreeClasses *m_treeClasses;
	QTreeWidgetItem *m_treeWidget;
	QString m_declaration;
	QString m_implementation;
	QString m_classname;
	void insertInDeclaration(QString scope, QString insertedText);
	void insertInImplementation(QString insertedText);
};
#endif






