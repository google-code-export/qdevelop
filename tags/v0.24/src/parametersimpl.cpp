#include "parametersimpl.h"
//
#include <QProcess>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
//
#include <QDebug>
//
ParametersImpl::ParametersImpl(QWidget * parent) 
	: QDialog(parent)
{
	setupUi(this);
    tableVariables->verticalHeader()->hide();
    QHeaderView *header = tableVariables->horizontalHeader();
    header->resizeSection(1, 230);
    on_defaults_clicked();
}
//
Parameters ParametersImpl::parameters()
{
	Parameters p;
	p.workingDirectory = location->text();
	p.arguments = arguments->text();
	for(int row=0; row<tableVariables->rowCount(); row++)
		p.env << tableVariables->item(row, 0)->text()+"="+tableVariables->item(row, 1)->text();
	return p;
}
//
void ParametersImpl::setParameters(Parameters p)
{
	arguments->setText( p.arguments );
	newVariable->clear();
	location->setText( p.workingDirectory );
	for(int i=0; i<tableVariables->rowCount(); i++)
		tableVariables->removeRow(0);
	int row = 0;
	foreach(QString s, p.env)
	{
		tableVariables->setRowCount(row+1);
        QTableWidgetItem *itemCol0 = new QTableWidgetItem( s.section("=", 0, 0) );
		itemCol0->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        tableVariables->setItem(row, 0, itemCol0);
        tableVariables->setItem(row++, 1, new QTableWidgetItem( s.section("=", 1, 1)));
	}
}
//
void ParametersImpl::on_defaults_clicked()
{
	arguments->clear();	
	newVariable->clear();
	location->clear();
	for(int i=0; i<tableVariables->rowCount(); i++)
		tableVariables->removeRow(0);
	QStringList systemEnvironment  = QProcess::systemEnvironment();
	int row = 0;
	foreach(QString s, systemEnvironment)
	{
		tableVariables->setRowCount(row+1);
        QTableWidgetItem *itemCol0 = new QTableWidgetItem( s.section("=", 0, 0) );
		itemCol0->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        tableVariables->setItem(row, 0, itemCol0);
        tableVariables->setItem(row++, 1, new QTableWidgetItem( s.section("=", 1, 1)));
	}
}
//
void ParametersImpl::on_chooseDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project location"),
		location->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	location->setText( s );
}
//
void ParametersImpl::on_add_clicked()
{
	QString  variable = newVariable->text().section("=", 0, 0).simplified();
	QString  value = newVariable->text().section("=", 1, 1).simplified();
	if (variable.isEmpty() || value.isEmpty() )
		return;
	if( tableVariables->findItems(variable, Qt::MatchExactly).count() )
	{
		QMessageBox::warning(this, 
			"QDevelop", tr("The variable \"%1\" already exists.").arg(variable),
			tr("Cancel") );
		return;
	}
	tableVariables->setRowCount(tableVariables->rowCount()+1);
	QTableWidgetItem *itemCol0 = new QTableWidgetItem( variable );
	itemCol0->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	tableVariables->setItem(tableVariables->rowCount()-1, 0, itemCol0);
	tableVariables->setItem(tableVariables->rowCount()-1, 1, new QTableWidgetItem( value ) );
}
//
void ParametersImpl::on_del_clicked()
{
	int row = tableVariables->currentRow();
	if( row != 1 )
		tableVariables->removeRow( row );
}
//


void ParametersImpl::on_sort_clicked()
{
	tableVariables->sortItems(0);
}
//

void ParametersImpl::on_edit_clicked()
{
	int row = tableVariables->currentRow();
	if( row != 1 )
	{
		tableVariables->setCurrentItem( tableVariables->item(row, 1) );
		tableVariables->editItem( tableVariables->item(row, 1) );
	}
}
//

void ParametersImpl::on_tableVariables_itemDoubleClicked(QTableWidgetItem* item)
{
	on_edit_clicked();

	// TODO remove gcc warnings
	item = NULL;
}
//
