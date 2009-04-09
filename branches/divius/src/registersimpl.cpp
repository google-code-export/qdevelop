#include "registersimpl.h"
//
#include <QHeaderView>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
RegistersImpl::RegistersImpl( QWidget * parent, Qt::WFlags f) 
	: QWidget(parent, f)
{
	setupUi(this);
	QHeaderView *header = tableWidget->horizontalHeader();
	header->resizeSection( 0, 50 );
	tableWidget->verticalHeader()->hide();
}
//


void RegistersImpl::registers(QString regs)
{
	regs = regs.section("Registers:", 1);
    while ( tableWidget->rowCount() )
        tableWidget->removeRow(0);
	foreach(QString line, regs.split("\n") )
	{
		QString name = line.section(" ", 0, 0);
		if( name == "(gdb)" || name.isEmpty() )
			continue;
		line = line.section(" ", 1).simplified();
		QString value1 = line.section(" ", 0, 0);
		line = line.section(" ", 1).simplified();
		QString value2 = line.section(" ", 0);
        QTableWidgetItem *newItem1 = new QTableWidgetItem(name);
        newItem1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        QTableWidgetItem *newItem2 = new QTableWidgetItem(value1);
        //newItem2->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        QTableWidgetItem *newItem3 = new QTableWidgetItem(value2);
        newItem3->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        int row = tableWidget->rowCount();
        tableWidget->setRowCount(row+1);
        tableWidget->setItem(row, 0, newItem1);
        tableWidget->setItem(row, 1, newItem2);
        tableWidget->setItem(row, 2, newItem3);
	}
}


void RegistersImpl::on_applyButton_clicked()
{
	QString command;
	for(int row=0; row<tableWidget->rowCount(); row++ )
	{
		QString name = tableWidget->item(row, 0)->text();
		QString value1 = tableWidget->item(row, 1)->text();
		command += "set $" + name + "=" + value1 + "\n";
	}
	emit debugCommand( command );
}
