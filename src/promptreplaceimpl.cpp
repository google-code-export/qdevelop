#include "promptreplaceimpl.h"
//
PromptReplaceImpl::PromptReplaceImpl( QWidget * parent, Qt::WFlags f) : QDialog(parent, f)
{
	setupUi(this);
	m_choice = 3;
}
//
void PromptReplaceImpl::on_replace_clicked()
{
	m_choice = 0;
	accept();
}
//
void PromptReplaceImpl::on_replaceAll_clicked()
{
	m_choice = 1;
	accept();
}
//
void PromptReplaceImpl::on_findNext_clicked()
{
	m_choice = 2;
	accept();
}
//
void PromptReplaceImpl::on_close_clicked()
{
	m_choice = 3;
	accept();
}
//
