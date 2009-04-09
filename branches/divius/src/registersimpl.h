#ifndef REGISTERSIMPL_H
#define REGISTERSIMPL_H
//
#include <QWidget>
#include "ui_registers.h"
//
class RegistersImpl : public QWidget, public Ui::Registers
{
Q_OBJECT
public:
	void registers(QString regs);
	RegistersImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
	void on_applyButton_clicked();
signals:
	void debugCommand(QString);
};
#endif





