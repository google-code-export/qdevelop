#ifndef PARAMETERSIMPL_H
#define PARAMETERSIMPL_H
//
#include "ui_parameters.h"
#include "projectmanager.h"
//
class ParametersImpl : public QDialog, public Ui::Param
{
Q_OBJECT
public:
	ParametersImpl( QWidget * parent );
	Parameters parameters();
	void setParameters(Parameters p);
private slots:
	void on_tableVariables_itemDoubleClicked(QTableWidgetItem* item);
	void on_edit_clicked();
	void on_sort_clicked();
	void on_defaults_clicked();
	void on_chooseDirectory_clicked();
	void on_add_clicked();
	void on_del_clicked();
private:
};
#endif




