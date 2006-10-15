/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
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
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qtfr.org
*
*/
#include "editor.h"
#include "mainimpl.h"
#include "lineedit.h"
#include "replaceimpl.h"
#include "tabwidget.h"

#include <QComboBox>
#include <QTextCursor>
#include <QTextBlock>
#include <QDialog>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPair>
#include <QToolButton>
#include <QTimer>
#include <QProcess>
#include "cpphighlighter.h"
#include <QTextDocumentFragment>
#include <QFileInfo>
#include <QDebug>


Editor::Editor(TabWidget * parent, MainImpl *mainimpl, InitCompletion *completion, QString name)
	: QWidget(parent), m_parent(parent), m_mainimpl(mainimpl), m_completion(completion), m_filename( name )
{
	int vposLayout = 0;
	m_comboClasses = m_comboMethods = 0;
	m_textEdit = new TextEdit(this, mainimpl, completion);
	m_backward = false;
	m_activeEditor = false;
	//
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);
	gridLayout->setMargin(0);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	if( suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc" || suffixe( m_filename ).toLower() == "h")
	{
	
		QHBoxLayout *hboxLayout = new QHBoxLayout();
		hboxLayout->setSpacing(6);
		hboxLayout->setMargin(6);
		hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
		//
		m_otherFileButton = 0;
		m_nameOtherFile = m_filename.mid(0, m_filename.lastIndexOf(".") );
		m_otherFileButton = new QToolButton(this);
		m_otherFileButton->setIcon(QIcon(":/treeview/images/h.png"));
		//
		m_otherFileButton->setToolTip( tr("Open")+" "+Editor::shortFilename(m_nameOtherFile)+".cpp" );
		connect(m_otherFileButton, SIGNAL(clicked()), this, SLOT(slotOtherFile()));
		hboxLayout->addWidget(m_otherFileButton);
		//
		if( suffixe( m_filename ).toLower() != "h" )
		{
			m_otherFileButton->setToolTip( tr("Open")+" "+Editor::shortFilename(m_nameOtherFile)+".h" );
			m_refreshButton = new QToolButton(this);
			m_refreshButton->setIcon(QIcon(":/toolbar/images/refresh.png"));
			m_refreshButton->setToolTip( tr("Refresh classes and methods lists") );
			connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotClassesMethodsList()) );
			hboxLayout->addWidget(m_refreshButton);
			//
			m_comboClasses = new QComboBox(this);
			m_comboClasses->setLineEdit( new LineEdit(m_comboClasses) );
			m_comboClasses->setEditable( true );
			m_comboClasses->setAutoCompletion( true );
			m_comboClasses->setObjectName(QString::fromUtf8("m_comboClasses"));
			connect(m_comboClasses, SIGNAL(activated(QString)), this, SLOT(slotComboClasses(QString)) );
			hboxLayout->addWidget(m_comboClasses);
			//
			m_comboMethods = new QComboBox(this);
			m_comboMethods->setMaximumSize( 500, m_comboMethods->height());
			m_comboMethods->setLineEdit( new LineEdit(m_comboClasses) );
			m_comboMethods->setEditable( true );
			m_comboMethods->setAutoCompletion( true );
			m_comboMethods->setMaxVisibleItems( 25 );
			m_comboMethods->setObjectName(QString::fromUtf8("comboMethodes"));
			m_comboMethods->lineEdit()->setAlignment(Qt::AlignLeft);
			connect(m_comboMethods, SIGNAL(activated(int)), this, SLOT(slotComboMethods(int)) );
			hboxLayout->addWidget(m_comboMethods);
		}
		//
		QSpacerItem *spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		hboxLayout->addItem(spacerItem);
		//
	    //
		if( suffixe( m_filename ).toLower() == "h" )
		{
			m_nameOtherFile += ".cpp";
			m_otherFileButton->setIcon(QIcon(":/treeview/images/cpp.png"));
		}
		else if( suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc")
		{
			m_nameOtherFile += ".h";
		}
		//
		gridLayout->addLayout(hboxLayout, vposLayout++, 0, 1, 1);
	}
	connect(m_textEdit, SIGNAL(editorModified(bool)), this, SLOT(slotModifiedEditor(bool)) );
	//
    m_findWidget = new QWidget;
	uiFind.setupUi(m_findWidget);
	connect(uiFind.toolClose, SIGNAL(clicked()), m_findWidget, SLOT(hide()) );
	connect(uiFind.editFind, SIGNAL(textChanged(QString)), this, SLOT(slotFindWidget_textChanged(QString)) );
	connect(uiFind.editFind, SIGNAL(returnPressed()), this, SLOT(slotFindNext()) );
	connect(uiFind.toolPrevious, SIGNAL(clicked()), this, SLOT(slotFindPrevious()) );
	connect(uiFind.toolNext, SIGNAL(clicked()), this, SLOT(slotFindNext()) );
	//
	autoHideTimer = new QTimer(this);
	autoHideTimer->setInterval(5000);
	autoHideTimer->setSingleShot(true);
	connect(autoHideTimer, SIGNAL(timeout()), m_findWidget, SLOT(hide()));
	//
	gridLayout->addWidget(m_textEdit, vposLayout++, 0, 1, 1);
	gridLayout->addWidget(m_findWidget, vposLayout++, 0, 1, 1);
	uiFind.labelWrapped->setVisible(false);
    m_findWidget->hide();
	//
	connect(&m_timerUpdateClasses, SIGNAL(timeout()), this, SLOT(slotTimerUpdateClasses()));
	connect(&m_timerCheckLastModified, SIGNAL(timeout()), this, SLOT(slotTimerCheckIfModifiedOutside()));
}
//
void Editor::setActiveEditor(bool b) 
{ 
	m_activeEditor = b;
	if( b && m_showTreeClasses )
	{
		m_timerUpdateClasses.start(m_intervalUpdatingClasses);
		if( m_completion )
			connect(m_completion, SIGNAL(completionList(TagList)), m_textEdit, SLOT(slotCompletionList(TagList)) );
	}
	else
	{
		slotTimerUpdateClasses();
		m_timerUpdateClasses.stop();
		if( m_completion )
			m_completion->disconnect( m_textEdit );
	}
}
//
void Editor::setShowTreeClasses(bool s) 
{ 
	m_showTreeClasses=s; 
	if( !m_showTreeClasses )
		m_timerUpdateClasses.stop();
}
//
void Editor::slotTimerUpdateClasses()
{
	QByteArray array( m_textEdit->toPlainText().toLocal8Bit() );
	if( !array.count() )
		return;
	char *ptr = array.data();
	quint16 check = qChecksum(ptr, array.length());
	if( check != m_checksum )
	{
		m_checksum = check;	
		emit updateClasses( filename(), m_textEdit->toPlainText());
	}
}
//
void Editor::slotTimerCheckIfModifiedOutside()
{
	QFile file(m_filename);
	if( m_lastModified != QFileInfo( file ).lastModified() )
	{
		m_timerCheckLastModified.stop();
		int rep = QMessageBox::question(this, "QDevelop", 
			tr("The file")+" \""+m_filename+"\"\n"+tr("was modified outside editor.")+"\n\n"+
			tr("What do you want to do?"), 
			tr("Overwrite"), tr("Reload File") );
		if( rep == 0 ) // Overwrite
		{
			m_textEdit->document()->setModified( true );
			save();
		}
		else if( rep == 1 ) // Reload
		{
			open(false);
		}
		m_timerCheckLastModified.start( 5000 );
	}
}
//
void Editor::updateNumLines(int currentLine, int numLines)
{
	QList<int> points = m_textEdit->breakpoints();
	foreach(unsigned int num, points )
	{
		//qDebug()<<"num:"<<num<<" currentLine :"<<currentLine<<"currentLine+numLines :"<<currentLine+numLines;
		if( numLines < 0 ) 
		{
			if( num >= currentLine+numLines )
			{
				m_textEdit->slotToggleBreakpoint(num);
				if( currentLine <= num)
					m_textEdit->slotToggleBreakpoint(num+numLines);
			}
		}
		else if( numLines > 0 && ( currentLine <= num) )
		{
			m_textEdit->slotToggleBreakpoint(num);
			m_textEdit->slotToggleBreakpoint(num+numLines);
		}
	}
	//
	points = m_textEdit->bookmarks();
	foreach(unsigned int num, points )
	{
		//qDebug()<<"num:"<<num<<" currentLine :"<<currentLine<<"currentLine+numLines :"<<currentLine+numLines;
		if( numLines < 0 ) 
		{
			if( num >= currentLine+numLines )
			{
				m_textEdit->slotToggleBookmark(num);
				if( currentLine <= num)
					m_textEdit->slotToggleBookmark(num+numLines);
			}
		}
		else if( numLines > 0 && ( currentLine <= num) )
		{
			m_textEdit->slotToggleBookmark(num);
			m_textEdit->slotToggleBookmark(num+numLines);
		}
	}
}
//
Editor::~Editor()
{
}
//
void Editor::replace()
{
	ReplaceImpl *dialog = new ReplaceImpl(this, m_textEdit, m_replaceOptions);
	dialog->exec();
	m_replaceOptions = dialog->replaceOptions();
	delete dialog;
}
//
bool Editor::open(bool silentMode)
{
	bool ret = m_textEdit->open(silentMode, m_filename, m_lastModified);
	if( ret && (suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc"))
		slotClassesMethodsList();
	QByteArray array( m_textEdit->toPlainText().toLocal8Bit() );
	if( array.count() )
	{
		char *ptr = array.data();
		quint16 check = qChecksum(ptr, array.length());
		m_checksum = check;
	}
	m_timerCheckLastModified.start( 5000 );
	return ret;
}
//
bool Editor::close()
{
	bool ret = m_textEdit->close( m_filename );
	return ret;
}
//
void Editor::setSyntaxHighlight(bool activate) 
{ 
	if( activate && !QString(":c:cpp:cc:h:").contains( ":"+m_filename.section(".", -1, -1).toLower()+":" ) )
		return;
	m_textEdit->setSyntaxHighlight(activate); 
}
//
bool Editor::save()
{
	return m_textEdit->save( m_filename, m_lastModified);
}
//
void Editor::gotoLine( int line, bool moveTop )
{
	m_textEdit->gotoLine( line, moveTop );
}
//
void Editor::slotFindPrevious()
{
	m_backward = true;
    slotFindWidget_textChanged(uiFind.editFind->text(), true);
}
//
void Editor::slotFindNext()
{
	m_backward = false;
    slotFindWidget_textChanged(uiFind.editFind->text(), true);
}
//
void Editor::find()
{
	autoHideTimer->stop();
    m_findWidget->show();
	uiFind.editFind->setFocus(Qt::ShortcutFocusReason);
	if( m_textEdit->textCursor().selectedText().length() )
		uiFind.editFind->setText( m_textEdit->textCursor().selectedText() );
	uiFind.editFind->selectAll();
	autoHideTimer->start();
}
//
void Editor::setFocus() 
{ 
    m_findWidget->hide();
    m_textEdit->setFocus(Qt::OtherFocusReason); 
}
//
void Editor::findContinue()
{
    slotFindWidget_textChanged(uiFind.editFind->text(), true);
}
//
void Editor::slotFindWidget_textChanged(QString text, bool fromButton)
{
	int options = 0;
    if( m_backward )
		options |= QTextDocument::FindBackward;
	if( uiFind.checkWholeWords->isChecked() )
		options |= QTextDocument::FindWholeWords;
	if( uiFind.checkCase->isChecked() )
		options |= QTextDocument::FindCaseSensitively;
    m_textEdit->slotFind(uiFind, text, (QTextDocument::FindFlags)options,fromButton);
	autoHideTimer->start();
}
//
QStringList Editor::classes()
{
	QStringList liste = m_classesMethodsList.keys();
	liste.sort();
	return liste;
}
//
QStringList Editor::methodes(QString classe)
{
	QStringList liste;
	foreach(QString line, m_classesMethodsList.value( classe ) )
		 liste += QStringList(line.section("::", 1, 1)+"::"+ line.section("::", 0, 0));
	liste.sort();
	QStringList liste2;
	foreach(QString ligne2, liste)
		 liste2 += QStringList(ligne2.section("::", 1, 1)+"::"+ ligne2.section("::", 0, 0));
	return liste2;
}
//
void Editor::slotClassesMethodsList()
{
	if( m_textEdit->document()->isModified() )
	{
		// Proposer sauvegarde
		int rep = QMessageBox::question(this, "QDevelop", 
			tr("Save")+" \""+m_filename+"\"", tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
		if( rep == 2 )
			return;
		if( rep == 0 )
		{
			m_textEdit->save(m_filename, m_lastModified);
		}
	}
	m_classesMethodsList.clear();
	if( m_mainimpl->ctagsIsPresent() )
	{
		testCtags = new QProcess();
        //connect(testCtags, SIGNAL(readyReadStandardOutput()), this, SLOT(slotParseCtags()) );
        connect(testCtags, SIGNAL(finished(int , QProcess::ExitStatus)), this, SLOT(slotParseCtags()) );
		testCtags->start("ctags", QStringList()<<"-f-" << "--fields=+S+K+n" << filename());
	}
}
//
void Editor::slotParseCtags()
{
	QString read = ((QProcess*)sender())->readAll();
	if( !read.isEmpty() )
	{
		int width = 0;
		foreach(QString s, read.split("\n") )
		{
			if( !s.contains("function") )
				continue;
			QString className = s.section("class:", -1, -1).section("\t", 0, 0);
			QString methodName = s.section("\t", 0, 0);
			QString signature = s.section("signature:", -1, -1);
			QString returnName = s.mid(s.indexOf("/^")+2);
			QString numLine = s.mid(s.indexOf("line:")+5).section("\t", 0, 0);
			returnName = returnName.left(returnName.indexOf(" $/;"));
			if( returnName.left( returnName.indexOf("::") ).indexOf("\t") != -1 )
				returnName = returnName.left( returnName.indexOf("::") ).section("\t", 0, 0);
			else if( returnName.left( returnName.indexOf("::") ).indexOf(" ") != -1 )
				returnName = returnName.left( returnName.indexOf("::") ).section(" ", 0, 0);
			else
				returnName = "";
			QString add = returnName+" " +className+"::"+methodName+signature+QChar(255)+numLine;
			QStringList methodes = m_classesMethodsList.value(className);
			methodes << add;
			if( !className.isEmpty() )
			{
				width = qMax(width, fontMetrics().width( className ) );
				m_classesMethodsList[className] = methodes;
			}
		}
		m_comboClasses->setGeometry(m_comboClasses->x(), m_comboClasses->y(), qMin(350, width+30), m_comboClasses->height());
	}
	((QProcess*)sender())->deleteLater();
	slotComboClasses();
}

//
bool Editor::inQuotations(int position, QString text)
{
	int debutQuote = 0, finQuote;
	bool realBegin = false;
	do
	{
		do
		{

			debutQuote = text.indexOf("\"", debutQuote); 
			if( debutQuote > 0 && (text.at(debutQuote-1) == '\\' || text.at(debutQuote-1) == '\'') )
			{
				debutQuote++;
				realBegin = false;
			}
			else
			{
				realBegin = true;
			}
		} while(!realBegin);
		finQuote = -1;
		if( debutQuote != -1 )
		{
			finQuote = debutQuote+1;
			bool realEnd = false;
			do
			{
				finQuote = text.indexOf("\"", finQuote); 
				if( finQuote > 0 && text.at(finQuote-1) == '\\' )
				{
					finQuote++;
					realEnd = false;
				}
				else 
				{
					realEnd = true;
				}
			} while(!realEnd);
		}
		if( debutQuote!=-1 && finQuote!=-1 )
		{
			if( position > debutQuote && position < finQuote )
			{
				return true;
			}
		}
		debutQuote = finQuote+1;
	} while( debutQuote!=-1 && finQuote!=-1 );
	// Idem with "'"
	debutQuote = 0, finQuote;
	realBegin = false;
	do
	{
		do
		{

			debutQuote = text.indexOf('\'', debutQuote); 
			if( debutQuote > 0 && (text.at(debutQuote-1) == '\\' || text.at(debutQuote-1) == '\"') )
			{
				debutQuote++;
				realBegin = false;
			}
			else
			{
				realBegin = true;
			}
		} while(!realBegin);
		
		finQuote = -1;
		if( debutQuote != -1 )
		{
			finQuote = debutQuote+1;
			bool realEnd = false;
			do
			{
				finQuote = text.indexOf('\'', finQuote); 
				if( finQuote > 0 && (text.at(finQuote-1) == '\\' || text.at(finQuote-1) == '\"') )
				{
					finQuote++;
					realEnd = false;
				}
				else 
				{
					realEnd = true;
				}
			} while(!realEnd);
		}
		if( debutQuote!=-1 && finQuote!=-1 )
		{
			if( position > debutQuote && position < finQuote )
			{
//qDebug()<<text<<debutQuote<<position<<finQuote;
				return true;
			}
		}
		debutQuote = finQuote+1;
	} while( debutQuote!=-1 && finQuote!=-1 );
	return false;
}
//
void Editor::toggleBreakpoint(bool activate, int line) 
{ 
	emit breakpoint(shortFilename(), QPair<bool,unsigned int>(activate, line));
}
//
void Editor::toggleBookmark(bool activate, int line) 
{ 
	QTextCursor save = m_textEdit->textCursor();
	int scroll = verticalScrollBar();
	gotoLine( line, false );
	m_textEdit->textCursor().movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
	m_textEdit->textCursor().movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	QString s = m_textEdit->textCursor().block().text().simplified();
	emit bookmark(filename(), s, QPair<bool,unsigned int>(activate, line));
	m_textEdit->setTextCursor( save );
	setVerticalScrollBar( scroll );
}
//
void Editor::slotToggleBreakpoint() 
{ 
	m_textEdit->slotToggleBreakpoint( m_textEdit->currentLineNumber() ); 
}
//
void Editor::emitListBreakpoints()
{
	foreach(unsigned int num, m_textEdit->breakpoints() )
		emit breakpoint(shortFilename(), QPair<bool,unsigned int>(true, num));
}
//
QString Editor::shortFilename()
{
	QString name = m_filename;
	int pos = m_filename.lastIndexOf( "/" );
	if( pos != -1 )
		name = name.mid(pos+1);
	return name;
}
//
QString Editor::directory()
{
	return m_filename.left( m_filename.length()-shortFilename().length()-1 );
}
QString Editor::shortFilename(QString nomLong)
{
	QString name = nomLong;
	int pos = nomLong.lastIndexOf( "/" );
	if( pos != -1 )
		name = name.mid(pos+1);
	return name;
}
//
QString Editor::suffixe(QString filename)
{
	return filename.section(".", -1, -1);
}
//
void Editor::setExecutedLine(int line) 
{ 
	m_textEdit->setExecutedLine(line); 
}
//
void Editor::slotModifiedEditor(bool modified)
{
	emit editorModified(this, modified);
}
//
//
void Editor::slotComboClasses(QString text)
{
	if( m_comboClasses == 0 )
		return;
	m_comboClasses->setHidden( !classes().count() );
	m_comboMethods->setHidden( !classes().count() );
	if( text.isEmpty() )
	{
		m_comboClasses->clear();
		m_comboClasses->addItems( classes() );
		m_comboClasses->setCurrentIndex(0);
		text = m_comboClasses->currentText();
	}
	m_comboMethods->clear();
	int width = 0;
	foreach(QString line, methodes(text) )
	{
		int numLine = QString(line.section(QChar(255), -1)).toInt();
		QString methode = line.left( line.indexOf(QChar(255)) );
		methode.remove('\r').remove('\n');
		QString typeRetour = " : " + methode.section(" ", 0, 0);
		if( typeRetour.contains("::") )
			typeRetour = "";
		methode = methode.section("::", 1, 1);
		int last = methode.lastIndexOf(":");
		if( last != 1 && !inQuotations(last, methode) )
			methode = methode.left( last );
		methode += typeRetour;
		QTextBlock block;
		int n=1;
		for (block = m_textEdit->document()->begin(); block.isValid() && n != numLine; block = block.next(), n++ )
			;
		m_comboMethods->addItem(methode, QVariant(block.text()));
		width = qMax(width, fontMetrics().width( methode ) );
	}
	m_comboMethods->setGeometry(m_comboClasses->x()+m_comboClasses->width()+6, m_comboMethods->y(), qMin(500, width+30), m_comboMethods->height());
	m_comboMethods->setCurrentIndex(0);
}
//
void Editor::slotComboMethods(int index)
{
	m_textEdit->gotoLine(1, false);
	QString s = m_comboMethods->itemData(index).toString();
	m_textEdit->find( s );
	gotoLine( m_textEdit->currentLineNumber(), true);
}
//
void Editor::setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g)
{

	m_textEdit->setSyntaxColors(a, b, c, d, e, f, g);
}
//
void Editor::slotOtherFile()
{
	m_mainimpl->openFile( QStringList(m_nameOtherFile) );
}
//
