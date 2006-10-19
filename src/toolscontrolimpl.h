#ifndef TOOLSCONTROLIMPL_H
#define TOOLSCONTROLIMPL_H
//
#include "ui_toolsControl.h"
//
class ToolsControlImpl : public QDialog, public Ui::ToolsControl
{
Q_OBJECT
public:
	ToolsControlImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	bool toolsControl();
	QString qmakeName() { return qmake->text(); };
	QString makeName() { return make->text(); };
	QString gdbName() { return gdb->text(); };
	QString ctagsName() { return ctags->text(); };
	QString linguistName() { return linguist->text(); };
	QString lupdateName() { return lupdate->text(); };
	QString lreleaseName() { return lrelease->text(); };
	QString designerName() { return designer->text(); };
	bool ctagsIsPresent()  { return m_ctagsIsPresent; };
	bool checkEnvironment()  { return m_checkEnvironment; };
private slots:
	void on_linguistLocation_clicked();
	void on_lupdateLocation_clicked();
	void on_lreleaseLocation_clicked();
	void on_designerLocation_clicked();
	void on_okButton_clicked();
	void on_qmakeLocation_clicked();
	void on_makeLocation_clicked();
	void on_gdbLocation_clicked();
	void on_ctagsLocation_clicked();
	void on_test_clicked();
	void chooseLocation(QLineEdit *dest);
private:
	bool m_ctagsIsPresent;
	bool m_checkEnvironment;
};
#endif







