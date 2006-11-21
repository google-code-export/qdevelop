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
* Program URL   : http://qdevelop.org
*
*/
#include "mainimpl.h"
#include "editor.h"
#include "build.h"
#include "debug.h"
#include "ui_about.h"
#include "ui_warning.h"
#include "parametersimpl.h"
#include "findfileimpl.h"
#include "shortcutsimpl.h"
#include "projectmanager.h"
#include "assistant.h"
#include "optionsimpl.h"
#include "newprojectimpl.h"
#include "cpphighlighter.h"
#include "tabwidget.h"
#include "stackimpl.h"
#include "toolscontrolimpl.h"
#include "InitCompletion.h"
#include "pluginsinterfaces.h"
//
#include <QFileDialog>
#include <QGridLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QFile> 
#include <QDir> 
#include <QProcess>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox> 
#include <QToolButton> 
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QLineEdit>
#include <QMenu>
#include <QActionGroup>
#include <QTimer>
#include <QString>
#include <QScrollArea>
#include <QLibraryInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QPluginLoader>
//

#define PROJECT_NAME "QDevelop"
#define VERSION "0.21-svn"

MainImpl::MainImpl(QWidget * parent) 
	: QMainWindow(parent)
{
	setupUi(this); 
	setStatusBar( false );
	m_saveBeforeBuild = true;
	m_restoreOnStart = true;
	m_projectManager = 0;
	m_debug = 0;
	//m_timer = 0;
	m_debugAfterBuild = ExecuteNone;
	m_buildAfterDebug = false;
	m_checkEnvironment = true;
	m_checkEnvironmentOnStartup = true;
	m_endLine = Default;
	m_tabSpaces = false;
	m_autoCompletion = true;
	m_autobrackets = true;
	m_match = true;
	m_backgroundColor = Qt::white;
	m_promptBeforeQuit = false;
	m_currentLineColor = QColor(215,252,255);
	m_matchingColor = Qt::red;
	m_findInFiles = 0;
	m_stack = 0;
	m_intervalUpdatingClasses = 5;
	m_showTreeClasses = true;
	m_completion = 0;
	m_projectsDirectory = QDir::homePath();
	m_closeButtonInTabs = false;
	crossButton = 0;
	//
	m_formatPreprocessorText.setForeground(QColor(0,128,0));
	m_formatQtText.setForeground(Qt::blue);
	m_formatSingleComments.setForeground(Qt::red);
	m_formatMultilineComments.setForeground(Qt::red);
	m_formatQuotationText.setForeground(Qt::darkGreen);	
	m_formatMethods.setForeground(Qt::black);
	m_formatKeywords.setForeground(Qt::blue);

	tableLocalVariables->verticalHeader()->hide();
	tableOtherVariables->verticalHeader()->hide();

#ifdef WIN32
	m_font = QFont("Courier New", 10);
#else
	m_font = QFont("Monospace", 10);
#endif
	m_tabStopWidth = 4;
	m_lineNumbers = m_selectionBorder = m_autoIndent = m_cppHighlighter = true;
	//
	m_tabEditors = new TabWidget( this );
	connect(m_tabEditors, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentTabChanged(int)) );
	//
	//
	setCentralWidget( m_tabEditors );
	//
	m_assistant = new Assistant();
	//
	treeFiles->setColumnCount(1);
	treeFiles->setHeaderLabels(QStringList(""));
	//
	treeClasses->setColumnCount(1);
	treeClasses->setHeaderLabels(QStringList(""));
	treeClasses->setMainImpl( this );
	//
	menuView->addSeparator();
	menuView->addAction(dockExplorer->toggleViewAction());
	menuView->addAction(dockOutputs->toggleViewAction());
	menuView->addSeparator();
	//
	menuToolbar->addAction(toolBarFiles->toggleViewAction());		
	menuToolbar->addAction(toolBarEdit->toggleViewAction());	
	menuToolbar->addAction(toolBarDebug->toggleViewAction());	
	menuToolbar->addAction(toolBarBuild->toggleViewAction());	
	//
	for (int i = 0; i < maxRecentsFiles; ++i) 
	{
		actionsRecentsFiles[i] = new QAction(this);
		menuLastsFiles->addAction(actionsRecentsFiles[i]);
		actionsRecentsFiles[i]->setVisible(false);
		connect(actionsRecentsFiles[i], SIGNAL(triggered()),this, SLOT(slotOpenRecentFile()));
	}
	updateActionsRecentsFiles();
	//
	for (int i = 0; i < maxRecentsProjects; ++i) 
	{
		actionsProjetsRecents[i] = new QAction(this);
		menuLastsProjects->addAction(actionsProjetsRecents[i]);
		actionsProjetsRecents[i]->setVisible(false);
		connect(actionsProjetsRecents[i], SIGNAL(triggered()),this, SLOT(slotOpenRecentProject()));
	}
	updateActionsRecentsProjects();
	createConnections();
	setMouseTracking( true );
	//
	dockExplorer->setFloating( false );
	dockOutputs->setFloating( false );
	//
	dockExplorer->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, dockExplorer);
	//
	m_stack = new StackImpl( this );
	m_stack->hide();
	//
	treeClasses->setCtagsName( m_ctagsName );
	loadPlugins();
}
//
MainImpl::~MainImpl() 
{
	if( m_completion )
	{
		m_completion->terminate();
		m_completion->wait();
		delete m_completion;
		m_completion = 0;
	}
}
//
void MainImpl::configureCompletion()
{
	if( m_completion )
		delete m_completion;
    m_completion = new InitCompletion (this);
    QString QTDIR;
    QString compilerInclude = "/usr/include";
    QStringList env = QProcess::systemEnvironment();
	foreach(QString str, env)
	{
#ifdef WIN32
		if( str.left(4).toUpper() == "PATH" )
		{
            foreach(QString entry, str.section("=", 1, 1).simplified().split(";") )
            {
                if( entry.toLower().contains("mingw") )
                {
                    compilerInclude = entry.toLower().section("bin", 0, 0) + "include";
                    break;
                }
            }
		}
#endif
	}
	QString projectDirectory;
	if( treeFiles->topLevelItem(0) )
		projectDirectory = m_projectManager->projectDirectory(treeFiles->topLevelItem(0));
    QStringList includes;
    includes << QLibraryInfo::location( QLibraryInfo::HeadersPath ) << projectDirectory 
    //includes << m_qtInstallHeaders << projectDirectory 
#ifdef WIN32
    << compilerInclude;
#else
    << "/usr/include";
#endif
    m_completion->setTempFilePath( QDir::tempPath() );
    m_completion->setCtagsCmdPath( ctagsName() );
    m_completion->addIncludes( includes );
    m_completion->initParse("", true, false);
    m_completion->start();
}
//
void MainImpl::setCrossButton(bool activate)
{
	if(crossButton && !activate)
	{
		crossButton->hide();
	}
	else if( activate )
	{
		if( !crossButton )
		{
			crossButton = new QToolButton(m_tabEditors);
			crossButton->setIcon( QIcon(":/toolbar/images/cross.png") );
			connect(crossButton, SIGNAL(clicked()), this, SLOT(slotCloseCurrentTab()) );
			crossButton->setGeometry(0,0,32,32);
			m_tabEditors->setCornerWidget(crossButton);
		}
		crossButton->show();
	}
	m_tabEditors->setCloseButtonInTabs( !activate );
}
//
void MainImpl::slotOtherFile()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->slotOtherFile();
}
//
void MainImpl::createConnections()
{
	connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()) );
	connect(actionNewProject, SIGNAL(triggered()), this, SLOT(slotNewProject()) );
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(slotOpen()) );
	connect(actionOpenProject, SIGNAL(triggered()), this, SLOT(slotOpenProject()) );
	connect(actionSaveFile, SIGNAL(triggered()), this, SLOT(slotSaveFile()) );
	connect(actionSaveAll, SIGNAL(triggered()), this, SLOT(slotSaveAll()) );
	connect(actionSaveFileAs, SIGNAL(triggered()), this, SLOT(slotSaveFileAs()) );
	connect(actionCompile, SIGNAL(triggered()), this, SLOT(slotCompile()) );
	connect(actionBuild, SIGNAL(triggered()), this, SLOT(slotBuild()) );
	connect(actionRebuild, SIGNAL(triggered()), this, SLOT(slotRebuild()) );
	connect(actionStopBuild, SIGNAL(triggered()), this, SLOT(slotStopBuild()) );
	connect(actionClean, SIGNAL(triggered()), this, SLOT(slotClean()) );
	connect(actionCut, SIGNAL(triggered()), this, SLOT(slotCut()) );
	connect(actionCopy, SIGNAL(triggered()), this, SLOT(slotCopy()) );
	connect(actionPaste, SIGNAL(triggered()), this, SLOT(slotPaste()) );
	connect(actionUndo, SIGNAL(triggered()), this, SLOT(slotUndo()) );
	connect(actionIndent, SIGNAL(triggered()), this, SLOT(slotIndent()) );
	connect(actionUnindent, SIGNAL(triggered()), this, SLOT(slotUnindent()) );
	connect(actionSelectAll, SIGNAL(triggered()), this, SLOT(slotSelectAll()) );
	connect(actionRedo, SIGNAL(triggered()), this, SLOT(slotRedo()) );
	connect(actionFind, SIGNAL(triggered()), this, SLOT(slotFind()) );	
	connect(actionReplace, SIGNAL(triggered()), this, SLOT(slotReplace()) );
	connect(actionGotoLine, SIGNAL(triggered()), this, SLOT(slotGotoLine()) );
	connect(actionFindContinue, SIGNAL(triggered()), this, SLOT(slotFindContinue()) );
	connect(actionFindPrevious, SIGNAL(triggered()), this, SLOT(slotFindPrevious()) );
	connect(actionCloseAllFiles, SIGNAL(triggered()), this, SLOT(slotCloseAllFiles()) );
	connect(actionCloseProject, SIGNAL(triggered()), this, SLOT(slotCloseProject()) );
	connect(actionOptions, SIGNAL(triggered()), this, SLOT(slotOptions()) );
	connect(actionConfigureShortcuts, SIGNAL(triggered()), this, SLOT(slotShortcuts()) );
	connect(actionCompleteCode, SIGNAL(triggered()), this, SLOT(slotCompleteCode()) );
	connect(actionFindInFiles, SIGNAL(triggered()), this, SLOT(slotFindInFiles()) );
	connect(actionNewFile, SIGNAL(triggered()), this, SLOT(slotNewFile()) );
	connect(actionToggleComment, SIGNAL(triggered()), this, SLOT(slotToggleComment()) );
	connect(actionComment, SIGNAL(triggered()), this, SLOT(slotComment()) );
	connect(actionUncomment, SIGNAL(triggered()), this, SLOT(slotUncomment()) );
	connect(actionParameters, SIGNAL(triggered()), this, SLOT(slotParameters()) );
	connect(actionGotoMatchingBracket, SIGNAL(triggered()), this, SLOT(slotGotoMatchingBracket()) );
	connect(actionBacktraces, SIGNAL(triggered()), this, SLOT(slotBacktraces()) );
	connect(addDebugVariable, SIGNAL(clicked()), this, SLOT(slotAddDebugVariable()) );
	connect(removeDebugVariable, SIGNAL(clicked()), this, SLOT(slotRemoveDebugVariable()) );
	//
	connect(actionExternalTools, SIGNAL(triggered()), this, SLOT(slotToolsControl()) );
	connect(actionCloseCurrentEditor, SIGNAL(triggered()), this, SLOT(slotCloseCurrentTab()) );
	actionCloseCurrentEditor->setShortcutContext( Qt::ApplicationShortcut );
	connect(actionPreviousTab, SIGNAL(triggered()), this, SLOT(slotPreviousTab()) );
	actionPreviousTab->setShortcutContext( Qt::ApplicationShortcut );
	connect(actionNextTab, SIGNAL(triggered()), this, SLOT(slotNextTab()) );
	actionNextTab->setShortcutContext( Qt::ApplicationShortcut );
	connect(actionsetFocusToEditor, SIGNAL(triggered()), this, SLOT(slotSetFocusToEditor()) );
	actionsetFocusToEditor->setShortcutContext( Qt::WindowShortcut );
	connect(actionToggleBreakpoint, SIGNAL(triggered()), this, SLOT(slotToggleBreakpoint()) );
	connect(actionDebug, SIGNAL(triggered()), this, SLOT(slotDebug()) );
	connect(actionExecuteWithoutDebug, SIGNAL(triggered()), this, SLOT(slotExecuteWithoutDebug()) );
	connect(actionStopDebug, SIGNAL(triggered()), this, SLOT(slotContinueDebug()) );
	connect(actionStepInto, SIGNAL(triggered()), this, SLOT(slotStepInto()) );
	connect(actionStepOver, SIGNAL(triggered()), this, SLOT(slotStepOver()) );
	connect(actionStepOut, SIGNAL(triggered()), this, SLOT(slotStepOut()) );
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()) );
	connect(treeFiles, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(slotDoubleClickTreeFiles(QTreeWidgetItem *, int)) );
	connect(treeFiles, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slotClickTreeFiles(QTreeWidgetItem *, int)) );
	connect(logBuild, SIGNAL(itemDoubleClicked ( QListWidgetItem *)), this, SLOT(slotDoubleClickLogBuild( QListWidgetItem *)) );
	connect(findFiles, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slotFindFilesActivated( QListWidgetItem *, QListWidgetItem *)) );
	connect(findLines, SIGNAL(itemDoubleClicked ( QListWidgetItem *)), this, SLOT(slotDoubleClickFindLines( QListWidgetItem *)) );
	connect(actionHelpQtWord, SIGNAL(triggered()), this, SLOT(slotHelpQtWord()) );
	connect(actionSwitchHeaderSources, SIGNAL(triggered()), this, SLOT(slotOtherFile()) );
	connect(actionToggleBookmark, SIGNAL(triggered()), this, SLOT(slotToggleBookmark()) );
	connect(actionNextBookmark, SIGNAL(triggered()), this, SLOT(slotNextBookmark()) );
	connect(actionPreviousBookmark, SIGNAL(triggered()), this, SLOT(slotPreviousBookmark()) );
	connect(actionClearAllBookmarks, SIGNAL(triggered()), this, SLOT(slotClearAllBookmarks()) );
	//
	m_projectGroup = new QActionGroup( this );	
	m_projectGroup->addAction( actionCloseProject );
	m_projectGroup->addAction( actionSaveProject );
	m_projectGroup->addAction( actionAddNewItem );
	m_projectGroup->addAction( actionAddScope );
	m_projectGroup->addAction( actionAddExistingFiles );
	m_projectGroup->addAction( actionProjectPropertie );
	m_projectGroup->addAction( actionBuild );
	m_projectGroup->addAction( actionRebuild );
	m_projectGroup->addAction( actionClean );
	m_projectGroup->addAction( actionStopBuild );
	m_projectGroup->addAction( actionCompile );
	m_projectGroup->addAction( actionDebug );
	m_projectGroup->addAction( actionStopDebug );
	m_projectGroup->addAction( actionExecuteWithoutDebug );
	m_projectGroup->addAction( actionStepOut );
	m_projectGroup->addAction( actionStepOver );
	m_projectGroup->addAction( actionStepInto );
	m_projectGroup->addAction( actionResetExecutablesList );
	m_projectGroup->setEnabled( false );
}
//
void MainImpl::slotShortcuts()
{
	ShortcutsImpl *dialog = new ShortcutsImpl(this);
	dialog->exec();
	delete dialog;
}
//
void MainImpl::slotNewFile()
{
	QString s = QFileDialog::getSaveFileName(
		this,
		tr("Choose the file to create"),
		"",
		tr("Files")+" (*.cpp *.h *.txt *.* *)" );
	if( s.isEmpty() )
	{
		// Cancel is clicked
		return;
	}
	QFile file(s);
	if( file.exists() )
	{
		QMessageBox::warning(0, 
			"QDevelop", tr("The file")+" \""+s+"\"\n "+tr("already exist on directory."),
			tr("Cancel") );
		return;
	}
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::about(0, "QDevelop",tr("Unable to create")+" "+s);
   		return;
	}
	file.close();
	openFile( QStringList( s ) );
}
//
void MainImpl::slotSetFocusToEditor()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->setFocus();
}
//
void MainImpl::slotToggleBreakpoint()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->toggleBreakpoint();
}
//
void MainImpl::slotPreviousBookmark()
{
	QList<QAction *> actionsList = menuBookmarks->actions();
	int pos = actionsList.indexOf( actionActiveBookmark );
	int posFirstBookmark = actionsList.indexOf( actionClearAllBookmarks ) + 2;
	if( posFirstBookmark > actionsList.count() )
		posFirstBookmark = -1;
	int posLastBookmark = actionsList.count()-1;
	if( pos != -1 && posFirstBookmark < pos )
	{
		QAction *newAction = actionsList.at( pos - 1 );
		slotActivateBookmark( newAction );
	}
	else if( posFirstBookmark != -1 )
	{
		QAction *newAction = actionsList.at( posLastBookmark );
		slotActivateBookmark( newAction );
	}
}
//
void MainImpl::slotNextBookmark()
{
	QList<QAction *> actionsList = menuBookmarks->actions();
	int count = actionsList.count();
	int pos = actionsList.indexOf( actionActiveBookmark );
	if( pos != 1 && pos+1 < count )
	{
		QAction *newAction = actionsList.at( pos + 1 );
		slotActivateBookmark( newAction );
	}
	else
	{
		int posFirstBookmark = actionsList.indexOf( actionClearAllBookmarks ) + 2;
		if( posFirstBookmark < count )
		{
			QAction *newAction = actionsList.at( posFirstBookmark );
			slotActivateBookmark( newAction );
		}
	}
}
//
void MainImpl::slotToggleBookmark()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->toggleBookmark();
}
//
void MainImpl::slotToggleComment()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->comment( TextEdit::Toggle );
}
//
void MainImpl::slotComment()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->comment( TextEdit::Comment );
}
//
void MainImpl::slotGotoMatchingBracket()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->gotoMatchingBracket();
}
//
void MainImpl::slotUncomment()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i ));
	if( editor  )
		editor->comment( TextEdit::Uncomment );
}
//
//
void MainImpl::slotPreviousTab()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i-1 ));
	if( editor  )
		m_tabEditors->setCurrentWidget( editor );
}
//
void MainImpl::slotNextTab()
{
	int i = m_tabEditors->currentIndex();
	Editor *editor = ((Editor*)m_tabEditors->widget( i+1 ));
	if( editor  )
		m_tabEditors->setCurrentWidget( editor );
}
//
void MainImpl::slotParameters()
{
	if(!m_projectManager)
		return; 
	ParametersImpl *parametersimpl = new ParametersImpl(this);
	parametersimpl->setParameters( m_projectManager->parameters() );
	if( parametersimpl->exec() == QDialog::Accepted )
		m_projectManager->setParameters( parametersimpl->parameters() );
	delete parametersimpl;
}
//
void MainImpl::slotOptions()
{
	OptionsImpl *options = new OptionsImpl(this, m_font, m_lineNumbers, m_selectionBorder, 
	m_autoIndent, m_cppHighlighter, m_tabStopWidth, m_saveBeforeBuild, m_restoreOnStart,
	m_formatPreprocessorText, m_formatQtText, m_formatSingleComments, 
	m_formatMultilineComments, m_formatQuotationText, m_formatMethods, 
	m_formatKeywords, m_autoMaskDocks, m_endLine, m_tabSpaces, m_autoCompletion, 
	m_backgroundColor, m_promptBeforeQuit, m_currentLineColor, m_autobrackets, 
	m_showTreeClasses, m_intervalUpdatingClasses, m_projectsDirectory, m_match, m_matchingColor,
	m_closeButtonInTabs);
	
	if( options->exec() == QDialog::Accepted )
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
		m_showTreeClasses = options->showTreeClasses->isChecked();
		m_intervalUpdatingClasses = options->interval->value();
		m_font = QFont( options->font().family(), options->font().pointSize() );
		m_tabStopWidth = options->tabStopWidth->value();
		m_cppHighlighter = options->highlight->isChecked();
		m_lineNumbers = options->numbers->isChecked();
		m_autoIndent = options->indent->isChecked();
		m_selectionBorder = options->selectionBorder->isChecked();
		m_saveBeforeBuild = options->saveAll->isChecked();
		m_restoreOnStart = options->restore->isChecked();
		m_endLine = (EndLine)options->endLine->currentIndex();
		m_tabSpaces = options->tabSpaces->isChecked();
		m_autoCompletion = options->completion->isChecked();
		m_autobrackets = options->brackets->isChecked();
		m_match = options->match->isChecked();
		m_promptBeforeQuit = options->promptBeforeQuit->isChecked();
		m_projectsDirectory = options->projectsDirectory->text();
		m_closeButtonInTabs = options->closeButton->isChecked();
		setCrossButton( !m_closeButtonInTabs );
		//
		m_formatPreprocessorText = options->syntaxe()->preprocessorFormat();
		m_formatQtText = options->syntaxe()->classFormat();
		m_formatSingleComments = options->syntaxe()->singleLineCommentFormat();
		m_formatMultilineComments = options->syntaxe()->multiLineCommentFormat();
		m_formatQuotationText = options->syntaxe()->quotationFormat();
		m_formatMethods = options->syntaxe()->functionFormat();
		m_formatKeywords = options->syntaxe()->keywordFormat();
		m_backgroundColor = options->backgroundColor();
		if( options->groupHighlightCurrentLine->isChecked() )
			m_currentLineColor = options->currentLineColor();
		else
			m_currentLineColor = QColor();
		if( options->match->isChecked() )
			m_matchingColor = options->matchingColor();
		else
			m_matchingColor = QColor();
		//
		for(int i=0; i<m_tabEditors->count(); i++)
		{
			((Editor *)m_tabEditors->widget( i ))->setShowTreeClasses( m_showTreeClasses );
			((Editor *)m_tabEditors->widget( i ))->setIntervalUpdatingTreeClasses( m_intervalUpdatingClasses );
			((Editor *)m_tabEditors->widget( i ))->setFont( m_font );
			((Editor *)m_tabEditors->widget( i ))->setTabStopWidth( m_tabStopWidth );
			((Editor *)m_tabEditors->widget( i ))->setSyntaxHighlight( m_cppHighlighter );
			((Editor *)m_tabEditors->widget( i ))->setLineNumbers( m_lineNumbers );
			((Editor *)m_tabEditors->widget( i ))->setAutoIndent( m_autoIndent );	
			((Editor *)m_tabEditors->widget( i ))->setMatch( m_match );	
			((Editor *)m_tabEditors->widget( i ))->setSelectionBorder( m_selectionBorder );
			((Editor *)m_tabEditors->widget( i ))->setAutoCompletion( m_autoCompletion );
			((Editor *)m_tabEditors->widget( i ))->setEndLine( m_endLine );
			((Editor *)m_tabEditors->widget( i ))->setTabSpaces( m_tabSpaces );
			((Editor *)m_tabEditors->widget( i ))->setBackgroundColor( m_backgroundColor );
			((Editor *)m_tabEditors->widget( i ))->setCurrentLineColor( m_currentLineColor );
			((Editor *)m_tabEditors->widget( i ))->setMatchingColor( m_matchingColor );
			((Editor *)m_tabEditors->widget( i ))->setAutobrackets( m_autobrackets );
			((Editor *)m_tabEditors->widget( i ))->setSyntaxColors
				(
					m_formatPreprocessorText,
					m_formatQtText,
					m_formatSingleComments,
					m_formatMultilineComments,
					m_formatQuotationText,
					m_formatMethods,
					m_formatKeywords
				);
		}
		QApplication::restoreOverrideCursor();
	}
	delete options;
}

//
void MainImpl::saveINI()
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif

	settings.beginGroup("Options");
	settings.setValue("m_showTreeClasses", m_showTreeClasses);
	settings.setValue("m_intervalUpdatingClasses", m_intervalUpdatingClasses);
	settings.setValue("m_font", m_font.toString());
	settings.setValue("m_tabStopWidth", m_tabStopWidth);
	settings.setValue("m_cppHighlighter", m_cppHighlighter);
	settings.setValue("m_lineNumbers", m_lineNumbers);
	settings.setValue("m_autoIndent", m_autoIndent);
	settings.setValue("m_selectionBorder", m_selectionBorder);
	settings.setValue("m_saveBeforeBuild", m_saveBeforeBuild);
	settings.setValue("m_restoreOnStart", m_restoreOnStart);
	settings.setValue("m_promptBeforeQuit", m_promptBeforeQuit);
	settings.setValue("m_autoCompletion", m_autoCompletion);
	settings.setValue("m_autobrackets", m_autobrackets);
	settings.setValue("m_closeButtonInTabs", m_closeButtonInTabs);
	settings.setValue("m_match", m_match);
	settings.setValue("m_checkEnvironment", m_checkEnvironment);
	settings.setValue("m_checkEnvironmentOnStartup", m_checkEnvironmentOnStartup);
	settings.setValue("m_endLine", m_endLine);
	settings.setValue("m_tabSpaces", m_tabSpaces);
	settings.setValue("m_backgroundColor", m_backgroundColor.name());	
	settings.setValue("m_currentLineColor", m_currentLineColor.name());
	settings.setValue("m_matchingColor", m_matchingColor.name());
	settings.setValue("m_projectsDirectory", m_projectsDirectory);
	//
	settings.setValue("m_formatPreprocessorText", m_formatPreprocessorText.foreground().color().name());
	settings.setValue("m_formatQtText", m_formatQtText.foreground().color().name());
	settings.setValue("m_formatSingleComments", m_formatSingleComments.foreground().color().name());
	settings.setValue("m_formatMultilineComments", m_formatMultilineComments.foreground().color().name());
	settings.setValue("m_formatQuotationText", m_formatQuotationText.foreground().color().name());
	settings.setValue("m_formatMethods", m_formatMethods.foreground().color().name());
	settings.setValue("m_formatKeywords", m_formatKeywords.foreground().color().name());
	settings.endGroup();
	
	// Save shortcuts
	settings.beginGroup("Shortcuts");
	QList<QObject*> childrens;
	childrens = children();
	QListIterator<QObject*> iterator(childrens);
	while( iterator.hasNext() )
	{
		QObject *object = iterator.next();
		QAction *action = qobject_cast<QAction*>(object);
		
		if ( (action) && (!(action->data().toString().contains( "Recent|" ))))
		{
			QString text = action->objectName();		
			if (!text.isEmpty())
			{
				QString shortcut = action->shortcut();
				settings.setValue( text, shortcut );
			}
		}
	}
	settings.endGroup();
	
	//
	if( !m_projectManager )
		return;
	
	if( m_restoreOnStart )
	{
		settings.beginGroup("Project");
		QString projectDirectory = m_projectManager->absoluteNameProjectFile(treeFiles->topLevelItem(0));
		settings.setValue("absoluteNameProjectFile", projectDirectory);
		settings.endGroup();
	}
}
//
void MainImpl::slotNewProject()
{
	NewProjectImpl *window = new NewProjectImpl(this, m_projectsDirectory);
	window->labelProjetParent->setHidden( true );
	window->parentProjectName->setHidden( true );
	if( window->exec() == QDialog::Accepted )
	{
		if( !slotCloseProject() )
		{
			delete window;
			return;
		}
		QString filename = window->projectName->text();
		if( !filename.toLower().contains( ".pro" ) )
			filename += ".pro";
		QString projectDirectory = window->location->text();
		projectDirectory += "/" + filename.left( filename.lastIndexOf(".") );
		QString srcDirectory = window->srcDirectory->text();
		QString uiDirectory = window->uiDirectory->text();
		QString buildDirectory = window->buildDirectory->text();
		QString binDirectory = window->binDirectory->text();
		QString uiFilename = window->uiFilename->text();
		QString uiObjectName = window->uiObjectName->text();
		QString subclassFilename = window->subclassFilename->text();
		QString subclassObjectName = window->subclassObjectName->text();
		QString absoluteProjectName = projectDirectory + "/" + filename ;
		QDir dir;
		if( !dir.mkdir(projectDirectory) )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("The directory cannot be created")+" \""+projectDirectory+"\"",
				tr("Cancel") );
			return;
		}
		QFile projectFile ( absoluteProjectName );
		if( !projectFile.open(QIODevice::WriteOnly | QIODevice::Text) )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("The project cannot be created"),
				tr("Cancel") );
			return;
		}
		else
		{
			QByteArray s;
			if( !window->empty->isChecked() )
			{
				
				s += "TEMPLATE = app\n";
				s += "QT = gui \\\n";
				s += "core\n";
				s += "CONFIG += qt \\\n";
				QString version = "debug";
				if( window->release->isChecked() )
					s += "release \\\n";
				else
					s += "debug \\\n";
				s += "warn_on \\\n";
				s += "console\n";
			}
			if( window->dialog->isChecked() || window->mainwindow->isChecked() )
			{
				if( !srcDirectory.isEmpty() )
				{
					QDir().mkdir( projectDirectory + "/" + srcDirectory );
				}
				if( !uiDirectory.isEmpty() )
				{
					QDir().mkdir( projectDirectory + "/" + uiDirectory );
				}
				if( !binDirectory.isEmpty() )
				{
					s += "DESTDIR = "+binDirectory+"\n";
				}
				if( !buildDirectory.isEmpty() )
				{
					s += "OBJECTS_DIR = "+buildDirectory+"\n";
					s += "MOC_DIR = "+buildDirectory+"\n";
					s += "UI_DIR = "+buildDirectory+"\n";
				}
				if( window->dialog->isChecked() )
				{
					QFile file(":/templates/templates/dialog.ui");
					file.open(QIODevice::ReadOnly);
					QByteArray data = file.readAll();
					file.close();
					data.replace("<class>Dialog</class>", "<class>"+uiObjectName.toAscii()+"</class>");
					data.replace("name=\"Dialog\"", "name=\""+uiObjectName.toAscii()+"\"");
					QFile uiFile(projectDirectory + "/" + uiDirectory + "/" + uiFilename.section(".ui", 0, 0) + ".ui");
					uiFile.open(QIODevice::WriteOnly);
					uiFile.write( data );
					uiFile.close();
					s+= "FORMS = "+uiDirectory + "/" + uiFilename.section(".ui", 0, 0) + ".ui" + "\n";
				}
				else if( window->mainwindow->isChecked() )
				{
					QFile file(":/templates/templates/mainwindow.ui");
					file.open(QIODevice::ReadOnly);
					QByteArray data = file.readAll();
					file.close();
					data.replace("<class>MainWindow</class>", "<class>"+uiObjectName.toAscii()+"</class>");
					data.replace("name=\"MainWindow\"", "name=\""+uiObjectName.toAscii()+"\"");
					QFile uiFile(projectDirectory + "/" + uiDirectory + "/" + uiFilename.section(".ui", 0, 0) + ".ui");
					uiFile.open(QIODevice::WriteOnly);
					uiFile.write( data );
					uiFile.close();
					s+= "FORMS = "+uiDirectory + "/" + uiFilename.section(".ui", 0, 0) + ".ui" + "\n";
				}
				// Create subclassing header
				QFile file(":/templates/templates/impl.h");
				file.open(QIODevice::ReadOnly);
				QByteArray data = file.readAll();
				file.close();
				data.replace("$IMPL_H", QString( subclassFilename.section(".h", 0, 0).toUpper()+"_H" ).toAscii());
				data.replace("$UIHEADERNAME", QString( "\"ui_"+uiFilename.section(".ui", 0, 0)+".h\"").toAscii());
				data.replace("$CLASSNAME", QString( subclassObjectName ).toAscii());
				if( window->dialog->isChecked() )
					data.replace("$PARENTNAME", QString( "QDialog" ).toAscii());
				else
					data.replace("$PARENTNAME", QString( "QMainWindow" ).toAscii());
				data.replace("$OBJECTNAME", QString( uiObjectName ).toAscii());
				QFile headerFile(projectDirectory + "/" + srcDirectory + "/" + subclassFilename + ".h");
				headerFile.open(QIODevice::WriteOnly);
				headerFile.write( data );
				headerFile.close();
				s += "HEADERS = "+ srcDirectory + "/" + subclassFilename + ".h" + "\n";
				
				// Create subclassing sources
				QFile file2(":/templates/templates/impl.cpp");
				file2.open(QIODevice::ReadOnly);
				data = file2.readAll();
				file2.close();
				QFile sourceFile(projectDirectory + "/" + srcDirectory + "/" + subclassFilename + ".cpp");
				data.replace("$HEADERNAME", QString( "\""+subclassFilename+".h\"" ).toAscii());
				data.replace("$CLASSNAME", QString( subclassObjectName ).toAscii());
				if( window->dialog->isChecked() )
					data.replace("$PARENTNAME", QString( "QDialog" ).toAscii());
				else
					data.replace("$PARENTNAME", QString( "QMainWindow" ).toAscii());
				sourceFile.open(QIODevice::WriteOnly);
				sourceFile.write( data );
				sourceFile.close();
				s += "SOURCES = "+ srcDirectory + "/" + subclassFilename + ".cpp \\" + "\n";
				
				// Create main.cpp
				QFile file3(":/templates/templates/main.cpp");
				file3.open(QIODevice::ReadOnly);
				data = file3.readAll();
				file3.close();
				QFile mainFile(projectDirectory + "/" + srcDirectory + "/" + "main.cpp");
				data.replace("$HEADERNAME", QString( "\""+subclassFilename+".h\"" ).toAscii());
				data.replace("$CLASSNAME", QString( subclassObjectName ).toAscii());
				mainFile.open(QIODevice::WriteOnly);
				mainFile.write( data );
				mainFile.close();
				s += "\t"+ srcDirectory + "/" + "main.cpp" + "\n";
			}
			//
			projectFile.write( s );
			projectFile.close();
		}
		delete window;
		openProject( absoluteProjectName );
	}
	else
		delete window;
}
//
void MainImpl::loadINI()
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif	

	settings.beginGroup("Options");
	QString s = settings.value("m_font", m_font.toString()).toString();
	m_font.fromString(s);
	m_tabStopWidth = settings.value("m_tabStopWidth", m_tabStopWidth).toInt();
	m_cppHighlighter = settings.value("m_cppHighlighter", m_cppHighlighter).toBool();
	m_lineNumbers = settings.value("m_lineNumbers", m_lineNumbers).toBool();
	m_autoIndent = settings.value("m_autoIndent", m_autoIndent).toBool();	
	m_autoCompletion = settings.value("m_autoCompletion", m_autoCompletion).toBool();
	m_autobrackets = settings.value("m_autobrackets", m_autobrackets).toBool();
	m_selectionBorder = settings.value("m_selectionBorder", m_selectionBorder).toBool();
	m_saveBeforeBuild = settings.value("m_saveBeforeBuild", m_saveBeforeBuild).toBool();
	m_restoreOnStart = settings.value("m_restoreOnStart", m_restoreOnStart).toBool();
	m_promptBeforeQuit = settings.value("m_promptBeforeQuit", m_promptBeforeQuit).toBool();
	m_checkEnvironment = settings.value("m_checkEnvironment", m_checkEnvironment).toBool();
	m_checkEnvironmentOnStartup = settings.value("m_checkEnvironmentOnStartup", m_checkEnvironmentOnStartup).toBool();
	m_autoMaskDocks = settings.value("m_autoMaskDocks", m_autoMaskDocks).toBool();
	m_endLine = (EndLine)settings.value("m_endLine", m_endLine).toInt();
	m_tabSpaces = settings.value("m_tabSpaces", m_tabSpaces).toBool();
	m_match = settings.value("m_match", m_match).toBool();
	m_backgroundColor = QColor(settings.value("m_backgroundColor", m_backgroundColor).toString());
	m_currentLineColor = QColor(settings.value("m_currentLineColor", m_currentLineColor).toString());
	m_matchingColor = QColor(settings.value("m_matchingColor", m_matchingColor).toString());
	m_projectsDirectory = settings.value("m_projectsDirectory", m_projectsDirectory).toString();
	m_showTreeClasses = settings.value("m_showTreeClasses", m_showTreeClasses).toBool();
	m_closeButtonInTabs = settings.value("m_closeButtonInTabs", m_closeButtonInTabs).toBool();
	setCrossButton( !m_closeButtonInTabs );
	m_intervalUpdatingClasses = settings.value("m_intervalUpdatingClasses", m_intervalUpdatingClasses).toInt();
	if( m_currentLineColor == Qt::black )
		m_currentLineColor = QColor();
	//
	m_formatPreprocessorText.setForeground( QColor(settings.value("m_formatPreprocessorText", m_formatPreprocessorText.foreground().color().name()).toString() ) );
	m_formatQtText.setForeground( QColor(settings.value("m_formatQtText", m_formatQtText.foreground().color().name()).toString() ) );
	m_formatSingleComments.setForeground( QColor(settings.value("m_formatSingleComments", m_formatSingleComments.foreground().color().name()).toString() ) );
	m_formatMultilineComments.setForeground( QColor(settings.value("m_formatMultilineComments", m_formatMultilineComments.foreground().color().name()).toString() ) );
	m_formatQuotationText.setForeground( QColor(settings.value("m_formatQuotationText", m_formatQuotationText.foreground().color().name()).toString() ) );
	m_formatMethods.setForeground( QColor(settings.value("m_formatMethods", m_formatMethods.foreground().color().name()).toString() ) );
	m_formatKeywords.setForeground( QColor(settings.value("m_formatKeywords", m_formatKeywords.foreground().color().name()).toString() ) );
	settings.endGroup();
	
	// Load shortcuts
	settings.beginGroup("Shortcuts");
	QList<QObject*> childrens;
	childrens = children();
	QListIterator<QObject*> iterator(childrens);
	while( iterator.hasNext() )
	{
		QObject *object = iterator.next();
		QAction *action = qobject_cast<QAction*>(object);
		
		if ( (action) && (!(action->data().toString().contains( "Recent|" ))) && (!action->objectName().isEmpty()) )
		{
			QString text = object->objectName();
			
			if( !text.isEmpty() )
			{
				QString shortcut = action->shortcut();
				shortcut = settings.value(text, shortcut).toString();
				action->setShortcut( shortcut );
			}
		}
	}
	settings.endGroup();
	
	if( m_restoreOnStart )
	{
		settings.beginGroup("Project");
		QString projectName = settings.value("absoluteNameProjectFile").toString();
		if( !projectName.isEmpty() )
		{
			if( !openProject( projectName ) )
			{
				return;
			}
		}
		settings.endGroup();
	}
}

void MainImpl::closeEvent( QCloseEvent * event )
{
	int response = 0;
	if( m_promptBeforeQuit )
	{
		response = QMessageBox::question(this, "QDevelop", 
			tr("Do you want to quit QDevelop ?"), 
			tr("Yes"), tr("No"), QString(), 0, 1);
	}
	if( response == 1 )
	{
		event->ignore();
		return;
	}
	saveINI();
	
	if( slotCloseProject(true) )
	{
		delete m_assistant;
		event->accept();
	}
	else
		event->ignore();
}

//
bool MainImpl::slotCloseAllFiles()
{
	bool ok = true;
	QList<Editor *> editorList;

	for(int i=0; i<m_tabEditors->count(); i++)
		editorList.append( ((Editor *)m_tabEditors->widget( i )) );
	foreach(Editor *editor, editorList )
	{
		if( !editor->close() )
			ok = false;
		else
			delete editor;
	}
	return ok;
}

//
static QString dir;

void MainImpl::slotOpen()
{
	if( dir.isEmpty() && m_projectManager )
        	dir = m_projectManager->projectDirectory( treeFiles->topLevelItem ( 0 ) );
	
	QString s = QFileDialog::getOpenFileName(
		this,
		tr("Choose a file to open"),
		dir,
		tr("Sources")+" (*.cpp *.h);;"+ 
		tr("Projects")+" (*.pro);;"+ 
		tr("Texts")+" (*.txt *.TXT);;"+ 
		tr("All Files")+" (* *.*)" 
		);
	if( s.isEmpty() )
	{
		// Cancel is clicked
		return;
	}
	if( s.right(4).toLower() == ".pro" )
	{
		openProject(s);
	}
	else
		openFile( QStringList( s ) );
	dir = QDir().absoluteFilePath( s );
}

void MainImpl::slotOpenProject()
{
	if( dir.isEmpty() && m_projectManager )
		dir = m_projectManager->projectDirectory( treeFiles->topLevelItem ( 0 ) );
	
	QString s = QFileDialog::getOpenFileName(
		this,
		tr("Choose a project to open"),
		dir,
		tr("Projects")+" (*.pro)"
	);
	if( s.isEmpty() )
	{
		// Cancel is clicked
		return;
	}
	
	openProject(s);
	dir = QDir().absoluteFilePath( s );
}


//
bool MainImpl::openProject(QString s)
{
	QFile file ( s );
	if( !file.exists() )
	{
		QMessageBox::warning(this, 
			"QDevelop", tr("The project")+ " " + s + " " + tr("doesn't exist."),
			tr("Cancel") );
		return false;
	}
	if( !slotCloseProject() )
		return false;
	configureCompletion();
	m_projectManager = new ProjectManager(this, treeFiles, treeClasses, s);
	treeFiles->setProjectManager( m_projectManager );
	treeClasses->setProjectManager( m_projectManager );
	connect(actionResetExecutablesList, SIGNAL(triggered()), m_projectManager, SLOT(slotResetExecutablesList()) );
	connect(actionSaveProject, SIGNAL(triggered()), m_projectManager, SLOT(slotSaveProject()) );
	connect(actionAddExistingFiles, SIGNAL(triggered()), m_projectManager, SLOT(slotAddExistingFiles()) );
	connect(actionAddNewItem, SIGNAL(triggered()), m_projectManager, SLOT(slotAddNewItem()) );
	connect(actionAddScope, SIGNAL(triggered()), m_projectManager, SLOT(slotAddScope()) );
	connect(actionProjectPropertie, SIGNAL(triggered()), m_projectManager, SLOT(slotProjectPropertie()) );
	setWindowTitle( s );
	setCurrentProject( s );
	m_projectGroup->setEnabled( true );
	slotClickTreeFiles( treeFiles->topLevelItem ( 0 ), 0);
    return true;
}
//
bool MainImpl::slotCloseProject(bool hide)
{
	if( m_projectManager )
		m_projectManager->saveProjectSettings();
	slotClearAllBookmarks();
	if( !slotCloseAllFiles() )
		return false;
	if( hide )
		this->hide();
	logBuild->clear();
	logDebug->clear();
	//
	delete m_completion;
	m_completion = 0;
	//
	if( m_projectManager && !m_projectManager->close() )
		return false;
	delete m_projectManager;
	m_projectManager = 0;
	setWindowTitle( "QDevelop" );
	m_projectGroup->setEnabled( false );
	return true;
}
//
void MainImpl::slotDoubleClickTreeFiles(QTreeWidgetItem *item, int)
{
	if( item->childCount() > 0 ) // Pas �itable
		return;
	QString filename = item->text(0);
	QString projectName = m_projectManager->projectFilename( item );
	QString projectDirectory = m_projectManager->projectDirectory(item);
	QString absoluteName = QDir(projectDirectory+"/"+filename).absolutePath();
	QStringList locationsList;
	locationsList << absoluteName;
	foreach(QString dir, m_projectManager->dependpath(item) )
		locationsList << QDir(projectDirectory + "/" +dir + "/" + filename).absolutePath();
	openFile( locationsList );
}
//
void MainImpl::slotClickTreeFiles(QTreeWidgetItem *item, int)
{
	QString projectName = m_projectManager->projectFilename( item );
	actionProjectPropertie->setText( tr("Properties of")+" "+projectName+"..." );
}
//
void MainImpl::slotSaveFile()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
	{
		editor->save();
	}
}
//
void MainImpl::slotSaveFileAs()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( !editor )
		return;
	QString s = QFileDialog::getSaveFileName(
		this,
		tr("Choose the file to create"),
		editor->filename(),
		tr("Files (*.cpp *.h *.txt *.* *)") );
	
	if( s.isEmpty() )
	{
		// Le bouton Annuler a ��cliqu�	
		return;
	}
	editor->setFilename( s );
	editor->save();
	m_tabEditors->setTabText(m_tabEditors->currentIndex(), editor->shortFilename() );
}
//
void MainImpl::slotHelpQtWord()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
	{
		QString word = editor->wordUnderCursor();
		QString className = editor->classNameUnderCursor();
		if (className.isEmpty() )
			className = word;
		if( !word.isEmpty() )
			m_assistant->showQtWord(className, word );
	}
}
//
void MainImpl::slotCloseCurrentTab()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor && !editor->close() )
		return;
	delete editor;
}
//
void MainImpl::closeTab(int numTab)
{
	Editor *editor = ((Editor*)m_tabEditors->widget( numTab ));
	if( editor && !editor->close() )
		return;
	delete editor;
}
//
void MainImpl::closeOtherTab(int numTab)
{
	Editor *noClose = ((Editor *)m_tabEditors->widget( numTab ));
	QList<Editor *> editorlist;
	for(int i=0; i<m_tabEditors->count(); i++)
		editorlist.append( ((Editor *)m_tabEditors->widget( i )) );
	foreach(Editor *editor, editorlist)
	{
		if( editor != noClose )
		{
			
			if( editor && !editor->close() )
				return;
			delete editor;
		}
	}
}
//
void MainImpl::slotClearAllBookmarks()
{
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		Editor *editor = ((Editor *)m_tabEditors->widget( i ));
		if( editor )
		{
			editor->clearAllBookmarks();
		}
	}
}
//
bool MainImpl::slotSaveAll()
{
	bool ok = m_projectManager->slotSaveProject();
	ok = true;
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		Editor *editor = ((Editor *)m_tabEditors->widget( i ));
		if( editor )
		{
			if( !editor->save() )
				ok = false;

		}
	}
	return ok;
}
//
Editor * MainImpl::openFile(QStringList locationsList, int numLine, bool silentMode, bool moveTop)
{
	//QApplication::setOverrideCursor(Qt::WaitCursor);
	QString s;
	foreach(s, locationsList)
	{
		if( QFile::exists( s ) )
		{
			break;
		}
	}
	if( Editor::shortFilename(s).section(".", -1, -1).toLower() == "ui" )
	{
		QProcess::startDetached (m_designerName, QStringList(s)); 
		QApplication::restoreOverrideCursor();
		return 0;
	}
	else if( Editor::shortFilename(s).section(".", -1, -1).toLower() == "ts" )
	{
		QProcess::startDetached (m_linguistName, QStringList(s)); 
		QApplication::restoreOverrideCursor();
		return 0;
	}
	// The file is perhaps already opened. Find filename in tabs.
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		if( ((Editor *)m_tabEditors->widget(i))->filename() == s)
		{
			m_tabEditors->setCurrentIndex( i );
			if( numLine != -1 )
			{
				((Editor *)m_tabEditors->widget(i))->setFocus( /*Qt::OtherFocusReason*/ );
				((Editor *)m_tabEditors->widget(i))->gotoLine( numLine, false );
				slotCurrentTabChanged( i );
			}
			QApplication::restoreOverrideCursor();
			return (Editor *)m_tabEditors->widget(i);
		}
	}
    //
    //
	// Not found in tabs, opens really.
	Editor *editor = new Editor(m_tabEditors, this, m_completion ,s);
	editor->setShowTreeClasses( m_showTreeClasses );
	editor->setIntervalUpdatingTreeClasses( m_intervalUpdatingClasses );
	editor->setFont( m_font );
	editor->setSyntaxHighlight( m_cppHighlighter );
	editor->setLineNumbers( m_lineNumbers );
	editor->setAutoIndent( m_autoIndent );	
	editor->setSelectionBorder( m_selectionBorder );
	editor->setEndLine( m_endLine );
	editor->setTabSpaces( m_tabSpaces );
	editor->setAutoCompletion( m_autoCompletion );
	editor->setAutobrackets( m_autobrackets );
	editor->setBackgroundColor( m_backgroundColor );
	editor->setMatchingColor( m_matchingColor );
	editor->setCurrentLineColor( m_currentLineColor );
	editor->setSyntaxColors
		(  
			m_formatPreprocessorText,
			m_formatQtText,
			m_formatSingleComments,
			m_formatMultilineComments,
			m_formatQuotationText,
			m_formatMethods,
			m_formatKeywords
		);
    
	if( !editor->open(silentMode) )
	{
		delete editor;
		QApplication::restoreOverrideCursor();
		return 0;
	}
	editor->setTabStopWidth( m_tabStopWidth );
	m_tabEditors->setCurrentIndex( m_tabEditors->addTab(editor, editor->shortFilename()+"   ") );
	editor->setFocus();
	if( numLine != -1 )
		editor->gotoLine(numLine, moveTop);
	connect(editor, SIGNAL(editorModified(Editor *, bool)), this, SLOT(slotModifiedEditor( Editor *, bool)) );
	connect(editor, SIGNAL(updateClasses(QString, QString)), this, SLOT(slotUpdateClasses(QString, QString)) );
	if( m_debug )
		connect(editor, SIGNAL(breakpoint(QString, QPair<bool,unsigned int>)), m_debug, SLOT(slotBreakpoint(QString, QPair<bool,unsigned int>)) );
	setCurrentFile(s);
	slotCurrentTabChanged( m_tabEditors->currentIndex() );

	QApplication::restoreOverrideCursor();
	return editor;
}
//

void MainImpl::toggleBookmark(Editor *editor, QString text, bool activate, QTextBlock block)
{
	Bookmark bookmark;
	bookmark.first = editor;
	bookmark.second = block;
	int line = editor->currentLineNumber( block );
	if( activate )
	{
		QString s = text;
		if( s.length() > 50 )
			s = s.left(50)+" ...";
		QAction *action = new QAction(s, menuBookmarks);
		connect(action, SIGNAL(triggered()), this, SLOT(slotActivateBookmark()));
		//
		QAction *before = 0;
		QList<QAction *> actionsList = menuBookmarks->actions();
		foreach(QAction *actionBefore, actionsList)
		{
			Bookmark bookmarkAction = actionBefore->data().value<Bookmark>();
			if( bookmarkAction.first == editor && editor->currentLineNumber( bookmarkAction.second ) > line )
			{
				before = actionBefore;
				break;
			}
		}
		//
		menuBookmarks->insertAction(before, action);
		QVariant v;
		v.setValue( bookmark );
		action->setData( v );
		actionActiveBookmark = action;
	}
	else
	{
		QList<QAction *> actionsList = menuBookmarks->actions();
		foreach(QAction *action, actionsList)
		{
			Bookmark bookmarkAction = action->data().value<Bookmark>();
			if( bookmarkAction.first == editor && bookmarkAction.second == block )
			{
				delete action;
				break;
			}
		}
	}
}
//
void MainImpl::slotActivateBookmark(QAction *action)
{
	if( action )
		actionActiveBookmark = action;
	else
		actionActiveBookmark = (QAction *)sender();
	Bookmark bookmark = actionActiveBookmark->data().value<Bookmark>();
	Editor *editor = 0;
	Editor *bookmarkEditor = bookmark.first;
	QTextBlock block = bookmark.second;
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		Editor *edit = ((Editor *)m_tabEditors->widget( i ));
		if( edit == bookmarkEditor )
		{
			editor = edit;
			break;
		}
	}
	if( editor )
	{
		QString filename = editor->filename();
		int line = editor->currentLineNumber( block );
		openFile(QStringList(filename), line);
	}
}
//
void MainImpl::slotUpdateClasses(QString filename, QString buffer)
{
	if( !m_projectManager )
		return;
	QString ext = "." + filename.section(".", -1, -1);
	QList<QTreeWidgetItem *> projectsList;
	m_projectManager->childsList(0, "PROJECT", projectsList);
	for(int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
	{
		QString projectName = projectsList.at(nbProjects)->text(0);
		QString projectDir = m_projectManager->findData(projectName, "projectDirectory");
		QStringList files;
		if( ext == ".cpp" )
			m_projectManager->sources( m_projectManager->itemProject(projectName), files );
		else
			m_projectManager->headers( m_projectManager->itemProject(projectName), files );
		foreach(QString s, files)
		{
			if( QDir::cleanPath(s) == QDir::cleanPath(filename) )
			{
				QStringList parents = m_projectManager->parents(projectsList.at(nbProjects));
				treeClasses->updateClasses(QDir::cleanPath(filename), buffer, parents, ext);
				if( m_completion )
				{
				    m_completion->initParse(buffer, true, false);
				    m_completion->start();
				}
			}
		}
	}
}
//
void MainImpl::slotModifiedEditor( Editor *editor, bool modified)
{
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		if( ((Editor *)m_tabEditors->widget( i )) == editor )
		{
			if( modified && m_tabEditors->tabText(i).left(1) != "*" )
				m_tabEditors->setTabText(i, "* "+m_tabEditors->tabText(i) );
			if( !modified && m_tabEditors->tabText(i).left(1) == "*" )
				m_tabEditors->setTabText(i, m_tabEditors->tabText(i).mid(2) );
			break;
		}
	}
}
//
void MainImpl::slotRebuild()
{
	slotBuild(true);
}
//
void MainImpl::slotClean()
{
	slotBuild(true, false);
}
//
void MainImpl::slotCompile()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor && Editor::suffixe( editor->filename() ).toLower() == "cpp" )
	{
		actionBuild->setEnabled( false );
		actionRebuild->setEnabled( false );
		actionCompile->setEnabled( false );
		toolBarDebug->setEnabled( false );
		logBuild->clear();
		dockOutputs->setVisible(true);
		if( m_saveBeforeBuild )
			slotSaveAll();
		tabOutputs->setCurrentIndex( 0 ); 
		m_projectsDirectoriesList << editor->directory();
		QString projectDirectory = m_projectManager->fileDirectory(editor->filename() );
		m_builder = new Build(this, m_qmakeName, m_makeName, projectDirectory, false, false, true, editor->filename());
	
		connect(m_builder, SIGNAL(finished()), this, SLOT(slotEndBuild()) );
		connect(m_builder, SIGNAL(finished()), m_builder, SLOT(deleteLater()) );
		connect(m_builder, SIGNAL(message(QString, QString)), this, SLOT(slotMessagesBuild(QString, QString)) );	
		m_builder->start();
	}
}
//
void MainImpl::slotBuild(bool clean, bool build)
{
	if(!m_projectManager)
	{
		return; 
	}
	if( m_qmakeName.isEmpty() || m_makeName.isEmpty() )
	{
		slotToolsControl();
		return;
	}
	if( actionDebug->text() == tr("Stop") && !slotDebug())
		return;
	m_buildAfterDebug = false;
	if( m_projectsDirectoriesList.count() == 0 )
	{
		actionBuild->setEnabled( false );
		actionRebuild->setEnabled( false );
		actionCompile->setEnabled( false );		
		actionExecuteWithoutDebug->setEnabled( false );
		toolBarDebug->setEnabled( false );
		logBuild->clear();
		dockOutputs->setVisible(true);
		if( m_saveBeforeBuild )
			slotSaveAll();
		tabOutputs->setCurrentIndex( 0 ); 
		//
		m_projectsDirectoriesList = m_projectManager->buildableProjectsDirectories();
		m_clean = clean;
		m_build = build;
	}
	QString repProjet = m_projectsDirectoriesList.first();
	QString projectName = m_projectManager->projectName( repProjet );

	QString makefilePath = repProjet + "/Makefile";
	bool qmakeNeeded = !QFile::exists(makefilePath);

	m_builder = new Build(this, m_qmakeName, m_makeName, repProjet, qmakeNeeded|m_clean, m_clean, m_build);

	connect(m_builder, SIGNAL(finished()), this, SLOT(slotEndBuild()) );
	connect(m_builder, SIGNAL(finished()), m_builder, SLOT(deleteLater()) );
	connect(m_builder, SIGNAL(message(QString, QString)), this, SLOT(slotMessagesBuild(QString, QString)) );	
	m_builder->start();
}
//
void MainImpl::slotStopBuild()
{
	m_projectsDirectoriesList = QStringList(QString());
	m_debugAfterBuild = ExecuteNone;
	emit stopBuild();
}
//
void MainImpl::slotEndBuild()
{
	m_projectsDirectoriesList.removeFirst();
	if( m_projectsDirectoriesList.count() )
		slotBuild();
	else
	{
		slotMessagesBuild( QString("\n---------------------- "+tr("Build normaly exited")+"  ----------------------\n"), "");
		actionBuild->setEnabled( true );
		actionRebuild->setEnabled( true );	
		actionCompile->setEnabled( true );	
		actionExecuteWithoutDebug->setEnabled( true );	
		toolBarDebug->setEnabled( true );
		if( m_debugAfterBuild )
			slotDebug( (int)m_debugAfterBuild-1 );
	}
}
//
void MainImpl::slotMessagesBuild(QString list, QString directory)
{
	QListWidgetItem *item = 0;
	foreach(QString message, list.split("\n"))
	{
		if( !message.isEmpty() )
		{
			message.remove( "\r" );
			logBuild->addItem( message );
			item = logBuild->item(logBuild->count()-1);
			if( message.toLower().contains("error:") || message.toLower().contains( tr("error:").toLower() ))
			{
				item->setTextColor( Qt::red );
				item->setData(Qt::UserRole, QVariant(directory) );
				m_projectsDirectoriesList = QStringList(QString());
				m_debugAfterBuild = ExecuteNone;
			}
			// Modify the two strings below "error:" and "warning:" to adapt in your language.
			else if( message.toLower().contains( "warning:") || message.toLower().contains( tr("warning:").toLower() ) )
			{
				item->setTextColor( Qt::blue );
				item->setData(Qt::UserRole, QVariant(directory) );
			}
		}
	}
	logBuild->setCurrentRow( logBuild->count()-1 );
	logBuild->setItemSelected( logBuild->currentItem(), false);
}
//
void MainImpl::slotDoubleClickLogBuild( QListWidgetItem *item )
{
	QString texte = item->text();
	if( !texte.contains("error:") && !texte.contains("warning:") 
		// Modify the two strings below "error:" and "warning:" to adapt in your language.
		&& !texte.contains( tr("error:").toLower() ) && !texte.contains( tr("warning:").toLower() ) )
		return;
	QString filename = texte.section(":", 0, 0).replace("\\", "/").replace("//", "/");
	int numLine = texte.section(":", 1, 1).toInt();
	if( numLine == 0 )
		return;
	QString projectDirectory = item->data(Qt::UserRole).toString();
	QString absoluteName = QDir(projectDirectory+"/"+filename).absolutePath();
	openFile( QStringList( absoluteName ), numLine);
}
//
void MainImpl::slotFindFilesActivated(QListWidgetItem *item, QListWidgetItem *)
{
	findLines->clear();
	if( !item )
		return;
	QStringList list = item->data(Qt::UserRole).toStringList();
	findLines->addItems( list );
}
//
void MainImpl::slotDoubleClickFindLines( QListWidgetItem *item)
{
	QString texte = item->text();
	int numLine = texte.section(":", 0, 0).section(" ", 1,1).toInt();
	if( numLine == 0 )
		return;
	QListWidgetItem *it = findFiles->currentItem();
	if( !it )
		it = findFiles->item(0);
	QString filename = it->text().mid( QString(tr("File")+" : ").length() );
	QApplication::setOverrideCursor(Qt::WaitCursor);
	openFile( QStringList( filename ), numLine);
    QApplication::restoreOverrideCursor();
}
//
void MainImpl::slotCut()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->cut();
}
//
void MainImpl::slotCurrentTabChanged(int index)
{
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		Editor *editor = ((Editor*)m_tabEditors->widget(i));
		if( editor )
			editor->setActiveEditor(i==index);
	}
}
//
void MainImpl::slotCopy()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->copy();
}
//
void MainImpl::slotCompleteCode()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->completeCode();
}
//
void MainImpl::slotPaste()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->paste();
}
//
void MainImpl::slotUndo()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->undo();
}
//
void MainImpl::slotIndent()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->slotIndent();
}
//
void MainImpl::slotUnindent()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->slotUnindent();
}
//
void MainImpl::slotRedo()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->redo();
}
//
void MainImpl::slotSelectAll()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->selectAll();
}
//
void MainImpl::slotFind()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->find();
}
//
void MainImpl::slotReplace()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->replace();
}
//
void MainImpl::slotGotoLine()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->dialogGotoLine();
}
//
void MainImpl::slotFindContinue()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if( editor )
		editor->findContinue();
}
//
void MainImpl::slotFindPrevious()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if ( editor )
		editor->findPrevious();
	}
//
void MainImpl::slotExecuteWithoutDebug()
{
	slotDebug(true);
}
//
bool MainImpl::modifiedEditors()
{
	bool modified = false; 
	for(int i=0; i<m_tabEditors->count(); i++)
	{
		Editor *editor = ((Editor *)m_tabEditors->widget( i ));
		if( editor )
		{
			if( editor->isModified() )
				modified = true;

		}
	}
	return modified;
}
//
bool MainImpl::slotDebug(bool executeOnly)
{
	if(!m_projectManager)
	{
		return false;
	}
	if( actionDebug->text() == tr("Start")
		&& ( m_projectManager->qmake()
		|| modifiedEditors() ) )
	{
		// Proposer sauvegarde
		int choice = QMessageBox::question(this, "QDevelop", 
			tr("The project has been modified, do you want to save your changes ?"), 
			tr("Yes"), tr("No"), tr("Cancel"),
			0, 2 );
		if( choice == 2 )
			return false;
		if( choice == 0 )
		{
			m_debugAfterBuild = (ExecuteVersion)(executeOnly+1);
			slotBuild();
			return true;
		}
	}
	QString exeName;
	if( actionDebug->text() == tr("Stop") )
	{
		int choice = QMessageBox::question(this, "QDevelop", 
			tr("Stop debugging ?"), 
			tr("Yes"), tr("No") );
		if( choice == 1 )
			return false;
	}
	else
		exeName = m_projectManager->executableName( executeOnly ? "release" : "debug");
	executeOnly = m_projectManager->isReleaseVersion();
	m_debugAfterBuild = ExecuteNone;
	if( exeName.isEmpty() && actionDebug->text() != tr("Stop"))
	{
		QMessageBox::critical(0, "QDevelop", 
			tr("The program don't exist,")+"\n"+
			tr("run Build."),tr("Ok") );
		return false;
	}
	emit resetExecutedLine();
	if( actionDebug->text() == tr("Stop") )
	{
		emit stopDebug();
		m_debug->wait();
		delete m_debug;
		m_debug = 0;
		slotEndDebug();
		return true;	
	}
	actionDebug->setText(tr("Stop"));
	actionDebug->setIcon( QIcon(":/toolbar/images/stop.png") );
	actionDebug->setShortcut( tr("Shift+F5") );
	actionStopDebug->setIcon( QIcon(":/toolbar/images/pause.png") );
	actionStopDebug->setEnabled( !executeOnly );
	logDebug->clear();
	dockOutputs->setVisible(true);
	tabOutputs->setCurrentIndex( 1 );
	Parameters parameters = m_projectManager->parameters();
	if( parameters.workingDirectory.isEmpty() )
		parameters.workingDirectory = m_projectManager->projectDirectoryOfExecutable();
	m_stack->setDirectory( m_projectManager->projectDirectoryOfExecutable() );
	m_debug = new Debug(this, m_gdbName, parameters, exeName, executeOnly);
	if( !executeOnly )
	{
		for(int i=0; i<m_tabEditors->count(); i++)
		{
			Editor *editor = ((Editor *)m_tabEditors->widget( i ));
			connect(editor, SIGNAL(breakpoint(QString, QPair<bool,unsigned int>)), m_debug, SLOT(slotBreakpoint(QString, QPair<bool,unsigned int>)) );
			editor->emitListBreakpoints();
		}
		QStringList list;
		for(int i=0; i < tableOtherVariables->rowCount(); i++)
			list << tableOtherVariables->item(i, 0)->text();
		emit otherVariables(list);
	}

	if( !executeOnly )
		connect(m_debug, SIGNAL(onPause()), this, SLOT(slotOnPause()) );
	connect(m_debug, SIGNAL(endDebug()), this, SLOT(slotEndDebug()) );
	connect(m_debug, SIGNAL(message(QString)), this, SLOT(slotMessagesDebug(QString)) );	
	connect(m_debug, SIGNAL(debugVariables( QList<Variable> )), this, SLOT(slotDebugVariables( QList<Variable> )) );	
	m_debug->start();
	return true;
}
//
void MainImpl::slotDebugVariables( QList<Variable> list)
{
	//qDebug()<<"slotDebugVariables"<<list.count();
	while( tableLocalVariables->rowCount() )
		tableLocalVariables->removeRow(0);
	foreach(Variable var, list )
	{
        QTableWidgetItem *newItem1 = new QTableWidgetItem(var.name);
		newItem1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        QTableWidgetItem *newItem2 = new QTableWidgetItem(var.type);
		newItem2->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        QTableWidgetItem *newItem3 = new QTableWidgetItem(var.address);
		newItem3->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        QTableWidgetItem *newItem4 = new QTableWidgetItem(var.content);
		newItem4->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        //
        if( var.kind == Debug::Local )
        {
			int row = tableLocalVariables->rowCount();
	        tableLocalVariables->setRowCount(row+1);
	        tableLocalVariables->setItem(row, 0, newItem1);
	        tableLocalVariables->setItem(row, 1, newItem2);
	        tableLocalVariables->setItem(row, 2, newItem3);
	        tableLocalVariables->setItem(row, 3, newItem4);
       	}
        else if( var.kind == Debug::OtherArgs )
        {
			for(int i=0; i < tableOtherVariables->rowCount(); i++)
			{
				if( tableOtherVariables->item(i, 0)->text() == var.name )
				{
			        tableOtherVariables->setItem(i, 1, newItem2);
			        tableOtherVariables->setItem(i, 2, newItem3);
			        tableOtherVariables->setItem(i, 3, newItem4);
				}
			}
       	}
		//qDebug() << var.name << var.kind << var.content << var.address;
	}
}
//
void MainImpl::slotEndDebug()
{
	actionBuild->setEnabled( true );
	actionRebuild->setEnabled( true );	
	actionCompile->setEnabled( true );	
	actionDebug->setEnabled( true );	
	actionDebug->setText(tr("Start"));
	actionDebug->setShortcut( tr("F5") );
	actionDebug->setIcon( QIcon(":/toolbar/images/dbgrun.png") );
	actionStopDebug->setIcon( QIcon(":/toolbar/images/pause.png") );
	actionStopDebug->setText( tr("Abort") );
	actionStopDebug->setToolTip( tr("Abort") );
	while( tableLocalVariables->rowCount() )
		tableLocalVariables->removeRow(0);
	emit resetExecutedLine();
	if( m_buildAfterDebug )
		slotBuild();
}
//
void MainImpl::slotMessagesDebug(QString message)
{
	char identifiantPointArret[] = { 26, 26, 0x0 };
	if( message.indexOf( identifiantPointArret )==0 )
	{
		actionStopDebug->setEnabled( true );
		QString filename = message.section(":", 0, -5).mid(2);
		int numLine = message.section(":", -4, -4).toInt();
		if( !filename.isEmpty() && numLine )
		{
			Editor *editor = openFile( QStringList(filename), numLine, true);
			if( editor )
				editor->setExecutedLine( numLine );
			slotOnPause();
    		setWindowState(windowState() & ~Qt::WindowMinimized);
			raise();
			activateWindow();
		}
	}
	else if( message.simplified().indexOf( '#' ) == 0 )
	{
		m_stack->addLine( message );
	}
	else if( message.indexOf( "InfoSources" ) == 0 )
	{
		m_stack->infoSources( message );
	}
	else if( message.indexOf( "Breakpoint" ) == 0 )
	{
		// Nothing
	}
	else
	{
		logDebug->append( message );
		//qDebug()<<message<<"FIN";
	}
}
//
void MainImpl::slotContinueDebug()
{
	if( actionStopDebug->text() == tr("Abort") )
	{
		// Ne marche pas avec gdb sous Windows
#ifdef Q_OS_LINUX
		emit pauseDebug();
#else
	logDebug->append(tr("Stopping is not possible under Windows. Put breakpoints.")); 
#endif
	}
	else
	{
		actionStopDebug->setIcon( QIcon(":/toolbar/images/pause.png") );
		actionStopDebug->setText( tr("Abort") );
		actionStopDebug->setToolTip( tr("Abort") );
		emit resetExecutedLine();
		emit debugCommand("cont\n");
	}
}
//
void MainImpl::slotStepInto()
{
	emit resetExecutedLine();
	emit debugCommand("step\n");
}
//
void MainImpl::slotStepOver()
{
	emit resetExecutedLine();
	emit debugCommand("next\n");
}
//
void MainImpl::slotStepOut()
{
	emit resetExecutedLine();
	emit debugCommand("finish\n");
}
//
void MainImpl::slotBacktraces()
{
	m_stack->list->clear();
	m_stack->show();
	emit debugCommand("info sources\n");
	emit debugCommand("bt\n");
}
//
void MainImpl::slotEditToGdb(QString texte)
{
	logDebug->append( texte );
	emit debugCommand(texte+"\n");
}
//
void MainImpl::slotOnPause()
{
	actionStopDebug->setIcon( QIcon(":/toolbar/images/resume.png") );
	actionStopDebug->setToolTip( tr("Continue") );
	actionStopDebug->setText( tr("Continue") );
}
//
void MainImpl::updateActionsRecentsFiles()
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif

	settings.beginGroup("RecentFiles");
	QStringList files = settings.value("RecentFilesList").toStringList();
	
	QStringList existingFiles;
	foreach (QString fileName, files)
	{
		if (QFile(fileName).exists())
		existingFiles.push_back(fileName);
	}
	if (existingFiles.size() < files.size())
	{
		settings.setValue("RecentFilesList", files);
		files = existingFiles;
	}
	
	int numRecentFiles = qMin(files.size(), (int)maxRecentsFiles);
	
	for (int i = 0; i < numRecentFiles; ++i) 
	{
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		actionsRecentsFiles[i]->setText(text);
		actionsRecentsFiles[i]->setData("Recent|"+files[i]);
		actionsRecentsFiles[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < maxRecentsFiles; ++j)
		actionsRecentsFiles[j]->setVisible(false);
}
//
void MainImpl::updateActionsRecentsProjects()
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif	

	settings.beginGroup("RecentProjects");
	QStringList files = settings.value("RecentProjectsList").toStringList();
	
	QStringList existingFiles;
	foreach (QString fileName, files)
	{
		if (QFile(fileName).exists())
		existingFiles.push_back(fileName);
	}
	if (existingFiles.size() < files.size())
	{
		settings.setValue("RecentProjectsList", files);
		files = existingFiles;
	}
	
	int numRecentFiles = qMin(existingFiles.size(), (int)maxRecentsProjects);
	
	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		actionsProjetsRecents[i]->setText(text);
		actionsProjetsRecents[i]->setData("Recent|"+files[i]);
		actionsProjetsRecents[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < maxRecentsProjects; ++j)
		actionsProjetsRecents[j]->setVisible(false);
}
//
void MainImpl::slotOpenRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		openFile(QStringList(action->data().toString().remove("Recent|")));
}
//
void MainImpl::slotOpenRecentProject()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		openProject(action->data().toString().remove("Recent|"));
}
//
QString MainImpl::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
//
void MainImpl::setCurrentFile(const QString &file)
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif

	settings.beginGroup("RecentFiles");
	QStringList files = settings.value("RecentFilesList").toStringList();
	files.removeAll(file);
	files.prepend(file);
	while (files.size() > maxRecentsFiles)
		files.removeLast();
	
	settings.setValue("RecentFilesList", files);
	updateActionsRecentsFiles();
}
//
void MainImpl::setCurrentProject(const QString &file)
{
#ifdef Q_OS_WIN32
	QSettings settings(QDir::homePath()+"/Application Data/qdevelop.ini", QSettings::IniFormat);
#else
	QSettings settings(PROJECT_NAME);
#endif

	settings.beginGroup("RecentProjects");
	QStringList files = settings.value("RecentProjectsList").toStringList();
	files.removeAll(file);
	files.prepend(file);
	while (files.size() > maxRecentsProjects)
		files.removeLast();
	
	settings.setValue("RecentProjectsList", files);
	updateActionsRecentsProjects();
}
//
void MainImpl::slotFindInFiles()
{
	if( !m_findInFiles )
	{
		QStringList directories;
		if( m_projectManager )
		{
			QList<QTreeWidgetItem *> listeProjets;
			m_projectManager->childsList(0, "PROJECT", listeProjets);
			for(int nbProjets=0; nbProjets < listeProjets.count(); nbProjets++)
			{
				directories << m_projectManager->projectDirectory(listeProjets.at(nbProjets)->text(0));
			}
		}
		m_findInFiles = new FindFileImpl(this, directories, findFiles, findLines);
	}
	dockOutputs->setVisible(true);
	tabOutputs->setCurrentIndex( 2 ); 
	m_findInFiles->exec();
	// Not delete dialog to save options, location and pattern on next showing.
}
//
void MainImpl::slotToolsControl(bool show)
{
	ToolsControlImpl *toolsControlImpl = new ToolsControlImpl( this );
	if( (!toolsControlImpl->toolsControl() && m_checkEnvironmentOnStartup ) || show )
		toolsControlImpl->exec();
	
	m_qmakeName = toolsControlImpl->qmakeName();
	m_makeName = toolsControlImpl->makeName();
	m_gdbName = toolsControlImpl->gdbName();
	m_ctagsName = toolsControlImpl->ctagsName();
	m_linguistName = toolsControlImpl->linguistName();
	m_lupdateName = toolsControlImpl->lupdateName();
	m_lreleaseName = toolsControlImpl->lreleaseName();
	m_designerName = toolsControlImpl->designerName();
	//
	m_ctagsIsPresent = toolsControlImpl->ctagsIsPresent();
	m_checkEnvironment = toolsControlImpl->checkEnvironment();
	m_checkEnvironmentOnStartup = toolsControlImpl->checkEnvOnStartup();
	delete toolsControlImpl;
	treeClasses->setCtagsIsPresent( m_ctagsIsPresent );
	treeClasses->setCtagsName( m_ctagsName );
}
//
void MainImpl::slotAbout()
{
	QDialog *about = new QDialog;
	Ui::About ui;
	ui.setupUi(about);
	ui.version->setText( QString("Version ")+VERSION );
	about->exec();
	delete about;
}
//
void MainImpl::slotAddDebugVariable()
{
	bool ok;
	QString var = QInputDialog::getText(this, "QDevelop",	tr("New Variable:"), QLineEdit::Normal,
	"", &ok);
	if (!ok || var.isEmpty())
		return;
	for(int i=0; i < tableOtherVariables->rowCount(); i++)
	{
		if( tableOtherVariables->item(i, 0)->text() == var )
		{
			QMessageBox::warning(0, 
				"QDevelop", tr("The variable")+" \""+var+"\"\n "+tr("already exist."),
				tr("Cancel") );
			return;
		}
	}
	QTableWidgetItem *newItem1 = new QTableWidgetItem(var);
	newItem1->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	QTableWidgetItem *newItem2 = new QTableWidgetItem();
	newItem2->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	QTableWidgetItem *newItem3 = new QTableWidgetItem();
	newItem3->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	QTableWidgetItem *newItem4 = new QTableWidgetItem();
	newItem4->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	//
	int row = tableOtherVariables->rowCount();
	tableOtherVariables->setRowCount(row+1);
	tableOtherVariables->setItem(row, 0, newItem1);
	tableOtherVariables->setItem(row, 1, newItem2);
	tableOtherVariables->setItem(row, 2, newItem3);
	tableOtherVariables->setItem(row, 3, newItem4);
	QStringList list;
	for(int i=0; i < tableOtherVariables->rowCount(); i++)
		list << tableOtherVariables->item(i, 0)->text();
	emit otherVariables(list);
}
//
void MainImpl::slotRemoveDebugVariable()
{
	if( tableOtherVariables->currentRow() == -1 )
		return;
	tableOtherVariables->removeRow( tableOtherVariables->currentRow() );
	QStringList list;
	for(int i=0; i < tableOtherVariables->rowCount(); i++)
		list << tableOtherVariables->item(i, 0)->text();
	emit otherVariables(list);
}
//
void MainImpl::loadPlugins()
{
    QDir pluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "bin" )
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) 
    {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) 
        {
		    TextEditInterface *iTextEdit = qobject_cast<TextEditInterface *>(plugin);
		    if (iTextEdit)
		    {
		        QAction *action = new QAction(iTextEdit->menuName(), plugin);
		        connect(action, SIGNAL(triggered()), this, SLOT(slotTextEditPlugin()));
		        menuPlugins->addAction(action);
		        if( iTextEdit->hasConfigDialog() )
		        {
			        QAction *action = new QAction(iTextEdit->menuName(), plugin);
			        connect(action, SIGNAL(triggered()), this, SLOT(slotConfigPlugin()));
			        menuPluginsSettings->addAction(action);
	        	}
	    	}
        }
    }
    if( menuPlugins->actions().isEmpty() )
    {
    	delete menuPlugins;
    	delete menuPluginsSettings;
   	}
}
//
void MainImpl::slotTextEditPlugin()
{
	Editor *editor = ((Editor*)m_tabEditors->currentWidget());
	if ( !editor )
		return;
    QAction *action = qobject_cast<QAction *>(sender());
    TextEditInterface *iTextEdit = qobject_cast<TextEditInterface *>(action->parent());
	editor->textPlugin( iTextEdit );
}
//
void MainImpl::slotConfigPlugin()
{
	QAction *action = qobject_cast<QAction *>(sender());
	TextEditInterface *iTextEdit = qobject_cast<TextEditInterface *>(action->parent());
	if( iTextEdit )
		iTextEdit->config();
}
//