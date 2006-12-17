#ifndef PROMPTREPLACEIMPL_H
#define PROMPTREPLACEIMPL_H
//
#include "ui_promptreplace.h"
//
class PromptReplaceImpl : public QDialog, public Ui::PromptReplace
{
Q_OBJECT
public:
	PromptReplaceImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	int choice() { return m_choice; };
private slots:
	void on_replace_clicked();
	void on_replaceAll_clicked();
	void on_findNext_clicked();
	void on_close_clicked();
private:
	int m_choice;
};
#endif
