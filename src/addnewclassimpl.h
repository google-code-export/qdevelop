#ifndef ADDNEWCLASSIMPL_H
#define ADDNEWCLASSIMPL_H
//
#include "ui_addnewclass.h"
//
class ProjectManager;
//
class AddNewClassImpl : public QDialog, public Ui::AddNewClass
{
Q_OBJECT
public:
	AddNewClassImpl(ProjectManager * parent);
private slots:
	void on_okButton_clicked();
	void on_className_textChanged(QString );
	void on_directoryButton_clicked();
public slots:
	void on_comboProjects_currentIndexChanged(int index);
private:
	ProjectManager *m_projectManager;
    void control();
    QString templateSourceImpl();
    QString templateHeaderImpl();
};
#endif











