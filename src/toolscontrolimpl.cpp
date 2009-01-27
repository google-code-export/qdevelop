/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2007  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/

#include "toolscontrolimpl.h"
#include "ui_warning.h"
#include "misc.h"
//
#include <QSettings>
#include <QLibraryInfo>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QDebug>
#define QD qDebug() << __FILE__ << __LINE__ << ":"

//
ToolsControlImpl::ToolsControlImpl( QWidget * parent, Qt::WFlags f)
        : QDialog(parent, f) {
    QString suffix;
#ifdef Q_OS_WIN32
    suffix = ".exe";
    QChar dirDelimiter = '\\';
#else
    if ( QFileInfo("/etc/debian_version").exists() )
        suffix = "-qt4";
    QChar dirDelimiter = '/';
#endif

    setupUi(this);
    QSettings settings( getQDevelopPath() + "qdevelop.ini" , QSettings::IniFormat);
    settings.beginGroup("Options");

    QString bindir = QLibraryInfo::location( QLibraryInfo::BinariesPath );
    bindir.append( dirDelimiter );

    qmake->setText( settings.value("m_qmakeName", bindir+"qmake"+suffix).toString() );
    lupdate->setText ( settings.value("m_lupdateName" , bindir+"lupdate" +suffix).toString() );
    lrelease->setText( settings.value("m_lreleaseName", bindir+"lrelease"+suffix).toString() );
#ifdef Q_OS_MACX
    linguist->setText( settings.value("m_linguistName", bindir+"linguist.app").toString() );
    designer->setText( settings.value("m_designerName", bindir+"Designer.app").toString() );
    assistant->setText( settings.value("m_assistantName", bindir+"Assistant.app").toString() );
#else
    linguist->setText( settings.value("m_linguistName", bindir+"linguist"+suffix).toString() );
    designer->setText( settings.value("m_designerName", bindir+"designer"+suffix).toString() );
    assistant->setText( settings.value("m_assistantName", bindir+"assistant"+suffix).toString() );
#endif

#ifdef Q_OS_WIN32
    make->setText( settings.value("m_makeName").toString() );
    gdb->setText( settings.value("m_gdbName").toString() );
    ctags->setText( settings.value("m_ctagsName").toString() );
#else
    make->setText( settings.value("m_makeName", "/usr/bin/make").toString() );
    gdb->setText( settings.value("m_gdbName", "/usr/bin/gdb").toString() );
    ctags->setText( settings.value("m_ctagsName", "/usr/bin/ctags").toString() );
#endif
    checkEnvironmentOnStartup->setChecked( settings.value("m_checkEnvironmentOnStartup", true).toBool() );
    settings.endGroup();
}
//

void ToolsControlImpl::on_qmakeLocation_clicked() {
    chooseLocation( qmake );
}
//
void ToolsControlImpl::on_makeLocation_clicked() {
    chooseLocation( make );
}
//
void ToolsControlImpl::on_gdbLocation_clicked() {
    chooseLocation( gdb );
}
//
void ToolsControlImpl::on_ctagsLocation_clicked() {
    chooseLocation( ctags );
}
//
void ToolsControlImpl::chooseLocation(QLineEdit *dest) {
    QString s = QFileDialog::getOpenFileName(
                    this,
                    tr("Please select the program"),
                    QDir::cleanPath(dest->text()),
                    "*" );
    if ( !s.isEmpty() ) // Ok clicked
    {
        dest->setText( QDir::toNativeSeparators ( s ) );
    }
}
//
void ToolsControlImpl::on_test_clicked() {
    toolsControl();
}
//
bool ToolsControlImpl::toolsControl() {
    bool result = true;
    qmakeIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    makeIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    gdbIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    ctagsIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    linguistIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    lupdateIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    lreleaseIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    designerIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    assistantIcon->setPixmap( QPixmap(":/divers/images/good.png") );
    // Control external tools
    QString lu;
    QProcess *testqmake = new QProcess(this);
    testqmake->start(qmake->text(), QStringList("-v"));
    testqmake->waitForFinished(5000);
    lu = testqmake->readAll();
    m_qVersion = lu.section("Using Qt version", 1, 1).section("in ", 0, 0).simplified();
    if ( lu.remove(":").left(15) != "QMake version 2" ) {
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
    if ( !lu.toLower().contains( "make" ) ) {
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
    if ( lu.left(7) != "GNU gdb" ) {
        gdbIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    testGdb->waitForFinished(5000);
    testGdb->terminate();
    delete testGdb;
    // ctags control
    // ctags control
    QProcess *testCtags = new QProcess(this);
    testCtags->start(ctags->text()+" --version");
    testCtags->waitForFinished(5000);
    lu = testCtags->readAll();
    m_ctagsIsPresent = true;
    if ( lu.isEmpty() ) {
        if (testCtags->error() == QProcess::UnknownError) {
            QMessageBox::warning(this, "QDevelop", tr("Ctags was detected in path %1, but this version is too old.").arg(ctags->text())
             , tr("Ok") );
        }
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
    if ( !QFile::exists( s ) ) {
        linguistIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    // lupdate control
    s = lupdate->text();
    if ( !QFile::exists( s ) ) {
        lupdateIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    // lrelease control
    s = lrelease->text();
    if ( !QFile::exists( s ) ) {
        lreleaseIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    // designer control
    s = designer->text();
    if ( !QFile::exists( s ) ) {
        designerIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    // assistant control
    s = assistant->text();
    if ( !QFile::exists( s ) ) {
        assistantIcon->setPixmap( QPixmap(":/divers/images/nogood.png") );
        result = false;
    }
    return result;
}
//

void ToolsControlImpl::on_buttonBox_clicked(QAbstractButton * button ) {
    // we only deal with "ok" and "cancel" clicks, all others are ignored
    if (buttonBox->button(QDialogButtonBox::Cancel) ==  button)
    {
	reject();
	return; 
    }
    else if (buttonBox->button(QDialogButtonBox::Ok) !=  button)
	return;
	
    QSettings settings( getQDevelopPath() + "qdevelop.ini" , QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("m_qmakeName", qmake->text());
    settings.setValue("m_makeName", make->text());
    settings.setValue("m_ctagsName", ctags->text());
    settings.setValue("m_gdbName", gdb->text());
    settings.setValue("m_linguistName", linguist->text());
    settings.setValue("m_lreleaseName", lrelease->text());
    settings.setValue("m_lupdateName", lupdate->text());
    settings.setValue("m_designerName", designer->text());
    settings.setValue("m_assistantName", assistant->text());
    settings.setValue("m_checkEnvironmentOnStartup", checkEnvironmentOnStartup->isChecked());
    settings.endGroup();
    accept();
}
//

void ToolsControlImpl::on_linguistLocation_clicked() {
    chooseLocation( linguist );
}
//
void ToolsControlImpl::on_lupdateLocation_clicked() {
    chooseLocation( lupdate );
}
//
void ToolsControlImpl::on_lreleaseLocation_clicked() {
    chooseLocation( lrelease );
}
//
void ToolsControlImpl::on_designerLocation_clicked() {
    chooseLocation( designer );
}
//

void ToolsControlImpl::on_assistantLocation_clicked() {
    chooseLocation( assistant );
}




QString ToolsControlImpl::qVersion()
{
	if( m_qVersion.isEmpty() )
                //return QString(qVersion()).left(5);
                return QString(QT_VERSION_STR).left(5);
	else
		return m_qVersion;
}

