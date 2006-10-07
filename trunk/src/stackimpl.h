#ifndef STACKIMPL_H
#define STACKIMPL_H
//
#include "ui_stack.h"
			#include <QDebug>
//
class MainImpl;
//
class StackImpl : public QDialog, public Ui::Stack
{
Q_OBJECT
public:
	StackImpl( MainImpl * parent = 0, Qt::WFlags f = 0 );
	void addLine( const QString line );
	void setDirectory( QString directory ) { m_directory = directory; };
	void infoSources(const QString s );
private slots:
	void on_list_currentItemChanged ( QListWidgetItem * item, QListWidgetItem * );
protected:
	void closeEvent( QCloseEvent * event );
private:
	QString m_directory;
	QStringList m_infoSources;
	MainImpl *m_mainImpl;
};
#endif

