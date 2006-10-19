#include "toolscontrolimpl.h"
#include "ui_warning.h"
//
#include <QSettings>
#include <QLibraryInfo>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDebug>

//
ToolsControlImpl::ToolsControlImpl( QWidget * parent, Qt::WFlags f) 
	: QDialog(parent, f)
{
	QString suffix;
#ifdef Q_OS_WIN32
	suffix = ".exe";
#endif
	setupUi(this);
	QSettings settings(QDir::homePath()+"/qdevelop.ini", QSettings::IniFormat);
	settings.beginGroup("Options");
	qmake->setText( settings.value("m_qmakeName", QLibraryInfo::location( QLibraryInfo::BinariesPath )+"/qmake"+suffix).toString() );
	make->setText( settings.value("m_makeName").toString() );
	gdb->setText( settings.value("m_gdbName").toString() );
	ctags->setText( settings.value("m_ctagsName").toString() );
	linguist->setText( settings.value("m_linguistName", QLibraryInfo::location( QLibraryInfo::BinariesPath )+"/linguist"+suffix).toString() );
	lupdate->setText( settings.value("m_lupdateName", QLibraryInfo::location( QLibraryInfo::BinariesPath )+"/lupdate"+suffix).toString() );
	lrelease->setText( settings.value("m_lreleaseName", QLibraryInfo::location( QLibraryInfo::BinariesPath )+"/lrelease"+suffix).toString() );
	designer->setText( settings.value("m_designerName", QLibraryInfo::location( QLibraryInfo::BinariesPath )+"/designer"+suffix).toString() );
	settings.endGroup();
}
//

void ToolsControlImpl::on_qmakeLocation_clicked()
{
	chooseLocation( qmake );
}
//
void ToolsControlImpl::on_makeLocation_clicked()
{
	chooseLocation( make );
}
//
void ToolsControlImpl::on_gdbLocation_clicked()
{
	chooseLocation( gdb );
}
//
void ToolsControlImpl::on_ctagsLocation_clicked()
{
	chooseLocation( ctags );
}
//
void ToolsControlImpl::chooseLocation(QLineEdit *dest)
{
	QString s = QFileDialog::getOpenFileName(
		this,
		tr("Please designe the program"),
		"/",
		"*" );
	if( !s.isEmpty() ) // Ok clicked
	{
		dest->setText( s );
	}
}
//
void ToolsControlImpl::on_test_clicked()
{
	toolsControl();
}
//
bool ToolsControlImpl::toolsControl()
{
	bool result = true;
	qmakeIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	makeIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	gdbIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	ctagsIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	linguistIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	lupdateIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	lreleaseIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	designerIcon->setPixmap( QPixmap(":/divers/images/good.png") );
	// Control external tools
	QString lu;
	QProcess *testqmake = new QProcess(this);
	testqmake->start(qmake->text(), QStringList("-v"));
	testqmake->waitForFinished(5000);
	lu = testqmake->readAll();
	if( lu.remove(":").left(15) != "QMake version 2" )
	{
		qmakeIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	delete testqmake;
	//
	//m_qtInstallHeaders = QLibraryInfo::location( QLibraryInfo::HeadersPath );
	// make control
	QProcess *testMake = new QProcess(this);
	testMake->start(make->text(), QStringList("-v"));
	testMake->waitForFinished(5000);
	lu = testMake->readAll();
	if( lu.left(8) != "GNU Make" )
	{
		makeIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	testMake->waitForFinished(5000);
	testMake->terminate();
	delete testMake;
	// gdb control
	QProcess *testGdb = new QProcess(this);
	testGdb->start(gdb->text()+" -v");
	testGdb->waitForFinished(5000);
	lu = testGdb->readAll();
	if( lu.left(7) != "GNU gdb" )
	{
		gdbIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	testGdb->waitForFinished(5000);
	testGdb->terminate();
	delete testGdb;
	// ctags control
	QProcess *testCtags = new QProcess(this);
	testCtags->start(ctags->text()+" --version");
	testCtags->waitForFinished(5000);
	lu = testCtags->readAll();
	m_ctagsIsPresent = true;
	if( lu.isEmpty() )
	{
		m_ctagsIsPresent = false;
		ctagsIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	testCtags->waitForFinished(5000);
	testCtags->terminate();
	delete testCtags;
	//
	// linguist control
	QString s = linguist->text();
	if( !QFile::exists( s ) )
	{
		linguistIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	// lupdate control
	s = lupdate->text();
	if( !QFile::exists( s ) )
	{
		lupdateIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	// lrelease control
	s = lrelease->text();
	if( !QFile::exists( s ) )
	{
		lreleaseIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
	// designer control
	s = designer->text();
	if( !QFile::exists( s ) )
	{
		designerIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
		result = false;
	}
		
	return result;
}
//

void ToolsControlImpl::on_okButton_clicked()
{
	QSettings settings(QDir::homePath()+"/qdevelop.ini", QSettings::IniFormat);
	settings.beginGroup("Options");
	settings.setValue("m_qmakeName", qmake->text());
	settings.setValue("m_makeName", make->text());
	settings.setValue("m_ctagsName", ctags->text());
	settings.setValue("m_gdbName", gdb->text());
	settings.setValue("m_linguistName", linguist->text());
	settings.setValue("m_lreleaseName", lrelease->text());
	settings.setValue("m_lupdateName", lupdate->text());
	settings.setValue("m_designerName", designer->text());
	settings.endGroup();
	accept();
}
//

void ToolsControlImpl::on_linguistLocation_clicked()
{
	chooseLocation( linguist );
}
//
void ToolsControlImpl::on_lupdateLocation_clicked()
{
	chooseLocation( lupdate );
}
//
void ToolsControlImpl::on_lreleaseLocation_clicked()
{
	chooseLocation( lrelease );
}
//
void ToolsControlImpl::on_designerLocation_clicked()
{
	chooseLocation( designer );
}
//
