#ifndef FINDFILEIMPL_H
#define FINDFILEIMPL_H
//
#include "ui_findfiles.h"
//
class QWidget;
class QListWidget;
//
class FindFileImpl : public QDialog, public Ui::FindFile
{
Q_OBJECT
public:
	FindFileImpl(QWidget * parent, QStringList directories, QListWidget *listResult, QListWidget *findLines);
private slots:
	void on_chooseDirectoryButton_clicked();
	void on_findButton_clicked();
	void find( QString directory );
protected:
	void showEvent(QShowEvent* _pEvent);
private:
	QWidget *m_parent;
	QListWidget *m_listResult, *m_listLines;
	void findInFile( QString filename );
	bool m_stop;
};
#endif
