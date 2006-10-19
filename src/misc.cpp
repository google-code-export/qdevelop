#include "misc.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

//
QVariant addressToVariant(void *it ) 
{
#if QT_POINTER_SIZE == 4
	return QVariant( reinterpret_cast<uint>(it)); 
#else
	return QVariant( reinterpret_cast<qulonglong>(it)); 
#endif
}
//
QTreeWidgetItem* variantToItem( QVariant variant )
{
#if QT_POINTER_SIZE == 4
		return (QTreeWidgetItem*)variant.toUInt();
#else
	return (QTreeWidgetItem*)variant.toULongLong();
#endif
}
//
QAction* variantToAction( QVariant variant )
{
#if QT_POINTER_SIZE == 4
	return (QAction*)variant.toUInt();
#else
	return (QAction*)variant.toULongLong();
#endif
}
//
bool connectDB(QString const& dbName)
{
	QSqlDatabase database;
	
	if( QSqlDatabase::database().databaseName() != dbName )
	{
		database = QSqlDatabase::addDatabase("QSQLITE");
		database.setDatabaseName(dbName);
	}
	else
	{
		database = QSqlDatabase::database();
		if ( database.isOpen() )
			return true;
	}
	//
    if (!database.open()) {
        QMessageBox::critical(0, "QDevelop",
            QObject::tr("Unable to establish a database connection.")+"\n"+
                     QObject::tr("QDevelop needs SQLite support. Please read "
                     "the Qt SQL driver documentation for information how "
                     "to build it."), QMessageBox::Cancel,
                     QMessageBox::NoButton);
        return false;
    }
	else
	{
		QSqlQuery query;
		QString queryString = "create table classesbrowser ("
		    "text string,"
		    "tooltip string,"
		    "icon string,"
		    "key string,"
		    "parents string,"
		    "name string,"
		    "implementation int,"
		    "declaration string,"
		    "ex_cmd string,"
		    "language string,"
		    "classname string,"
		    "structname string,"
		    "enumname string,"
		    "access string,"
		    "signature string,"
		    "kind string"
		    ")";
		
		query.exec(queryString);
		queryString = "create table editors ("
		    "filename string,"
		    "scrollbar int,"
		    "numline int"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table bookmarks ("
		    "filename string,"
		    "numline int"
		    ")";
		query.exec(queryString);
		//
		queryString = "create table breakpoints ("
		    "filename string,"
		    "numline int"
		    ")";
		query.exec(queryString);
		//
		queryString = "select * from config",
		//
		queryString = "create table config ("
		    "currentEditor int"
		    ")";
		query.exec(queryString);
		//
qDebug()<<"TODO INSERT INTO config";
    }
    return true;
}
//
