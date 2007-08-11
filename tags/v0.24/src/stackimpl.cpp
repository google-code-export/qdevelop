#include "stackimpl.h"
#include "mainimpl.h"
//
#include <QDir>
//
StackImpl::StackImpl( MainImpl * parent, Qt::WFlags f) 
	: QDialog(parent, f), m_mainImpl(parent)
{
	setupUi(this);
}
//
void StackImpl::closeEvent( QCloseEvent * event )
{
	// TODO remove gcc warnings
	event = NULL;
}
//
void StackImpl::addLine( const QString line )
{
	QString s = line;
	s = s.mid( s.indexOf(" ") );
	if( s.contains(" in ") )
		s = line.section(" in ", -1);
	s = s.simplified();
	list->addItem( s );
}
//
void StackImpl::on_list_currentItemChanged ( QListWidgetItem * item, QListWidgetItem * )
{
	if( !item )
		return;
	QString s = item->text().section(" at ", -1);
	QString filename = s.section(":", 0, 0);
	int numLine = s.section(":", -1, -1).toInt();
	QStringList filter = m_infoSources.filter( filename );
	QStringList files;
	foreach(QString f, filter)
	{
		if( !QFile::exists( f ) )
			f = m_directory + "/" + f;
		files << f;
	}
	if( filter.count() )
		m_mainImpl->openFile( files, numLine);
}
//
void StackImpl::infoSources(const QString s )
{
	if( s.indexOf("InfoSources:Source files") == -1)
	{
		foreach(QString text, s.split(",") )
		{
			m_infoSources << text.remove(QString("InfoSources:")).simplified();
			
		}
	}
}
