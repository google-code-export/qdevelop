#ifndef REPLACEIMPL_H
#define REPLACEIMPL_H
//
#include "ui_replace.h"
#include "editor.h"
//
class TextEdit;
//
class ReplaceImpl : public QDialog, public Ui::Replace
{
Q_OBJECT
public:
	ReplaceImpl( QWidget * parent, TextEdit *textEdit, ReplaceOptions options);
	ReplaceOptions replaceOptions() { return m_replaceOptions;};
private slots:
	void on_replace_clicked();
protected:
private:
	void saveReplaceOptions();
	TextEdit *m_textEdit;
	ReplaceOptions m_replaceOptions;
};
#endif

