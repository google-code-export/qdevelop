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
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
#include "mainimpl.h"
#include "editor.h"
#include "build.h"
#include "debug.h"
#include "ui_about.h"
#include "ui_warning.h"
#include "parametersimpl.h"
#include "findfileimpl.h"
#include "openfileimpl.h"
#include "shortcutsimpl.h"
#include "projectmanager.h"
#include "assistant.h"
#include "designer.h"
#include "optionsimpl.h"
#include "newprojectimpl.h"
#include "cpphighlighter.h"
#include "tabwidget.h"
#include "stackimpl.h"
#include "toolscontrolimpl.h"
#include "InitCompletion.h"
#include "pluginsinterfaces.h"
#include "misc.h"
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
#define VERSION "0.25-svn"

MainImpl::MainImpl(QWidget * parent)
        : QMainWindow(parent)
{
    setupUi(this);
    setStatusBar( false );
    m_saveBeforeBuild = true;
    m_restoreOnStart = true;
    m_projectManager = 0;
    m_debug = 0;
    m_debugAfterBuild = ExecuteNone;
    m_buildAfterDebug = false;
    m_checkEnvironmentOnStartup = true;
    m_endLine = Default;
    m_tabSpaces = false;
    m_autoCompletion = true;
    m_autobrackets = true;
    m_match = true;
    m_highlightCurrentLine = true;
    m_backgroundColor = Qt::white;
    m_textColor = Qt::black;
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
    m_assistant = 0;
    m_designer = 0;
    crossButton = 0;
    m_pluginsDirectory = "";
    m_includeDirectory = QLibraryInfo::location( QLibraryInfo::HeadersPath );
    m_documentationDirectory = QLibraryInfo::location( QLibraryInfo::DocumentationPath );
    m_configureCompletionNeeded = false;
    m_mibCodec = 106; // UTF-8 by default
    m_buildQtDatabase = 0;
    m_buildQtDatabaseAsked = false;
    m_displayEditorToolbars = true;
    m_displayWhiteSpaces = true;

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

    separatorOtherFile = toolBarEdit->addSeparator();
    separatorOtherFile->setVisible(false);
    actionOtherFile = new QAction(this);
    actionOtherFile->setVisible(false);
    toolBarEdit->addAction(actionOtherFile);

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
    connect(m_tabEditors, SIGNAL(currentChanged(int)), this, SLOT(slotUpdateOtherFileActions()) );
    //
    //
    setCentralWidget( m_tabEditors );
    //
    m_assistant = new Assistant();
    m_designer = new Designer();
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
    menuView->addAction(dockCallsStack->toggleViewAction());
    menuView->addAction(dockRegisters->toggleViewAction());
    menuView->addSeparator();
    //
    menuToolbar->addAction(toolBarFiles->toggleViewAction());
    menuToolbar->addAction(toolBarEdit->toggleViewAction());
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
    dockCallsStack->setFloating( false );
    //
    dockExplorer->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dockExplorer);
    //
    m_stack = new StackImpl(this, callStacks);
    //
    treeClasses->setCtagsName( m_ctagsName );
    logBuild->setMainImpl( this );
}
//
MainImpl::~MainImpl()
{
    if ( m_completion )
    {
        m_completion->terminate();
        m_completion->wait();
        delete m_completion;
        m_completion = 0;
    }
    if( m_buildQtDatabase )
    {
    	m_buildQtDatabase->setStopRequired();
    	m_buildQtDatabase->wait();
    	delete m_buildQtDatabase;
    	m_buildQtDatabase = 0;
   	}
}

//convenient functions to access editor tabs
Editor * MainImpl::currentEditor()
{
	Editor *editor = 0;
	int currentIndex = m_tabEditors->currentIndex();
	if( currentIndex != -1 )
		editor = (Editor*) (m_tabEditors->widget( currentIndex ));
    return editor;
}

Editor * MainImpl::givenEditor(int i)
{
    return (Editor*) (m_tabEditors->widget(i));
}

//
void MainImpl::renameEditor(QString oldName, QString newName)
{
    foreach(Editor *editor, allEditors() )
    {
        if ( editor->filename() == oldName)
        {
            editor->setFilename( newName );
            slotModifiedEditor(editor, editor->isModified() );
            editor->save();
            break;
        }
    }
}
//
void MainImpl::configureCompletion(QString projectDirectory)
{
    QString QTDIR;
    QStringList includes;
    includes << projectDirectory;
    m_completion->setCtagsCmdPath( ctagsName() );
    m_completion->addIncludes( includes, projectDirectory);
    m_configureCompletionNeeded = false;
}
//
void MainImpl::gotoFileInProject(QString& filename)
{
    if ( !m_projectManager )
        return;

    tabExplorer->setCurrentIndex(0);
    m_projectManager->setCurrentItem(filename);
}
//
void MainImpl::setCrossButton(bool activate)
{
    if (crossButton && !activate)
    {
        crossButton->hide();
    }
    else if ( activate )
    {
        if ( !crossButton )
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
    Editor *editor = currentEditor();
    if ( editor  )
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
    connect(actionQmake, SIGNAL(triggered()), this, SLOT(slotQmake()) );
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
    connect(addDebugVariable, SIGNAL(clicked()), this, SLOT(slotAddDebugVariable()) );
    connect(removeDebugVariable, SIGNAL(clicked()), this, SLOT(slotRemoveDebugVariable()) );
    connect(actionPrint, SIGNAL(triggered()), this, SLOT(slotPrint()) );
    connect(actionGotoDeclaration, SIGNAL(triggered()), this, SLOT(slotGotoDeclaration()) );
    connect(actionGotoImplementation, SIGNAL(triggered()), this, SLOT(slotGotoImplementation()) );
    connect(actionMethodsList, SIGNAL(triggered()), this, SLOT(slotMethodsList()) );
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
    connect(findFiles, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slotFindFilesActivated( QListWidgetItem *, QListWidgetItem *)) );
    connect(findLines, SIGNAL(itemDoubleClicked ( QListWidgetItem *)), this, SLOT(slotDoubleClickFindLines( QListWidgetItem *)) );
    connect(actionHelpQtWord, SIGNAL(triggered()), this, SLOT(slotHelpQtWord()) );
    connect(actionSwitchHeaderSources, SIGNAL(triggered()), this, SLOT(slotOtherFile()) );
    connect(actionToggleBookmark, SIGNAL(triggered()), this, SLOT(slotToggleBookmark()) );
    connect(actionNextBookmark, SIGNAL(triggered()), this, SLOT(slotNextBookmark()) );
    connect(actionPreviousBookmark, SIGNAL(triggered()), this, SLOT(slotPreviousBookmark()) );
    connect(actionClearAllBookmarks, SIGNAL(triggered()), this, SLOT(slotClearAllBookmarks()) );
    connect(actionOpenFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()) );
    connect(actionNewQtVersion, SIGNAL(triggered()), this, SLOT(slotNewQtVersion()) );
    connect(actionOtherFile, SIGNAL(triggered()), this, SLOT(slotOtherFile()) );
    //
    m_projectGroup = new QActionGroup( this );
    m_projectGroup->addAction( actionCloseProject );
    m_projectGroup->addAction( actionSaveProject );
    m_projectGroup->addAction( actionAddNewItem );
    m_projectGroup->addAction( actionAddNewClass );
    m_projectGroup->addAction( actionAddScope );
    m_projectGroup->addAction( actionAddExistingFiles );
    m_projectGroup->addAction( actionProjectPropertie );
    m_projectGroup->addAction( actionBuild );
    m_projectGroup->addAction( actionRebuild );
    m_projectGroup->addAction( actionClean );
    m_projectGroup->addAction( actionStopBuild );
    m_projectGroup->addAction( actionCompile );
    m_projectGroup->addAction( actionQmake );
    m_projectGroup->addAction( actionDebug );
    m_projectGroup->addAction( actionStopDebug );
    m_projectGroup->addAction( actionExecuteWithoutDebug );
    m_projectGroup->addAction( actionStepOut );
    m_projectGroup->addAction( actionStepOver );
    m_projectGroup->addAction( actionStepInto );
    m_projectGroup->addAction( actionResetExecutablesList );
    m_projectGroup->setEnabled( false );
    //
    m_buildingGroup = new QActionGroup( this );
    m_buildingGroup->addAction( actionCompile );
    m_buildingGroup->addAction( actionBuild );
    m_buildingGroup->addAction( actionRebuild );
    m_buildingGroup->addAction( actionExecuteWithoutDebug );
    m_buildingGroup->addAction( actionDebug );
    m_buildingGroup->addAction( actionStopDebug );
    m_buildingGroup->addAction( actionStepInto );
    m_buildingGroup->addAction( actionStepOver );
    m_buildingGroup->addAction( actionStepOut );
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
    if ( s.isEmpty() )
    {
        // Cancel is clicked
        return;
    }
    QFile file(s);
    if ( file.exists() )
    {
        QMessageBox::warning(0,
                             "QDevelop", tr("The file \"%1\"\n already exists in directory.").arg(s),
                             tr("Cancel") );
        return;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::about(0, "QDevelop",tr("Unable to create %1").arg(s));
        return;
    }
    file.close();
    openFile( QStringList( s ) );
}
//
void MainImpl::slotSetFocusToEditor()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->setFocus();
}
//
void MainImpl::slotToggleBreakpoint()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->toggleBreakpoint();
}
//
void MainImpl::slotPreviousBookmark()
{
    QList<QAction *> actionsList = menuBookmarks->actions();
    int pos = actionsList.indexOf( actionActiveBookmark );
    int posFirstBookmark = actionsList.indexOf( actionClearAllBookmarks ) + 2;
    if ( posFirstBookmark > actionsList.count() )
        posFirstBookmark = -1;
    int posLastBookmark = actionsList.count()-1;
    if ( pos != -1 && posFirstBookmark < pos )
    {
        QAction *newAction = actionsList.at( pos - 1 );
        slotActivateBookmark( newAction );
    }
    else if ( posFirstBookmark != -1 )
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
    if ( pos != 1 && pos+1 < count )
    {
        QAction *newAction = actionsList.at( pos + 1 );
        slotActivateBookmark( newAction );
    }
    else
    {
        int posFirstBookmark = actionsList.indexOf( actionClearAllBookmarks ) + 2;
        if ( posFirstBookmark < count )
        {
            QAction *newAction = actionsList.at( posFirstBookmark );
            slotActivateBookmark( newAction );
        }
    }
}
//
void MainImpl::slotToggleBookmark()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->toggleBookmark();
}
//
void MainImpl::slotToggleComment()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->comment( TextEdit::Toggle );
}
//
void MainImpl::slotComment()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->comment( TextEdit::Comment );
}
//
void MainImpl::slotMethodsList()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->methodsList();
}
//
void MainImpl::slotGotoImplementation()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->gotoImplementation();
}
//
void MainImpl::slotGotoDeclaration()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->gotoDeclaration();
}
//
void MainImpl::slotGotoMatchingBracket()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->gotoMatchingBracket();
}
//
void MainImpl::slotUncomment()
{
    Editor *editor = currentEditor();
    if ( editor  )
        editor->comment( TextEdit::Uncomment );
}
//
void MainImpl::slotPreviousTab()
{
    if ( 0 < m_tabEditors->count() )
    {
        int i = m_tabEditors->currentIndex()-1;
        Editor *editor = givenEditor( 0>i?m_tabEditors->count()-1:i );

        if ( editor  )
            m_tabEditors->setCurrentWidget( editor );
    }
}
//
void MainImpl::slotNextTab()
{
    if ( 0 < m_tabEditors->count() )
    {
        int i = (m_tabEditors->currentIndex()+1)%m_tabEditors->count();
        Editor *editor = givenEditor(i);
        if ( editor  )
            m_tabEditors->setCurrentWidget( editor );
    }
}
//
void MainImpl::slotParameters()
{
    if (!m_projectManager)
        return;
    ParametersImpl *parametersimpl = new ParametersImpl(this);
    parametersimpl->setParameters( m_projectManager->parameters() );
    if ( parametersimpl->exec() == QDialog::Accepted )
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
                                           m_backgroundColor, m_promptBeforeQuit, m_highlightCurrentLine, m_currentLineColor, m_autobrackets,
                                           m_showTreeClasses, m_intervalUpdatingClasses, m_projectsDirectory, m_match, m_matchingColor,
                                           m_closeButtonInTabs, m_pluginsDirectory, m_makeOptions, m_mibCodec,
                                           m_includeDirectory, m_displayEditorToolbars, m_displayWhiteSpaces, m_documentationDirectory,
                                           m_textColor
     );

    if ( options->exec() == QDialog::Accepted )
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
        m_highlightCurrentLine = options->groupHighlightCurrentLine->isChecked();
        m_promptBeforeQuit = options->promptBeforeQuit->isChecked();
        m_projectsDirectory = options->projectsDirectory->text();
        m_pluginsDirectory = options->pluginsDirectory->text();
        m_includeDirectory = options->includeDirectory->text();
        m_documentationDirectory = options->documentationDirectory->text();
        m_makeOptions = options->makeOptions->text();
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
        m_textColor = options->textColor();
        m_currentLineColor = options->currentLineColor();
        m_matchingColor = options->matchingColor();
        m_mibCodec = options->mib();
        m_displayEditorToolbars = options->showEditorToolbars->isChecked();
        m_displayWhiteSpaces = options->displayWhiteSpaces->isChecked();

        slotUpdateOtherFileActions();
        foreach(Editor *editor, allEditors() )
        {
            editor->setShowTreeClasses( m_showTreeClasses );
            editor->setIntervalUpdatingTreeClasses( m_intervalUpdatingClasses );
            editor->setFont( m_font );
            editor->setTabStopWidth( m_tabStopWidth );
            editor->setSyntaxHighlight( m_cppHighlighter );
            editor->setLineNumbers( m_lineNumbers );
            editor->setAutoIndent( m_autoIndent );
            editor->setMatch( m_match );
            editor->setHighlightCurrentLine( m_highlightCurrentLine );
            editor->setSelectionBorder( m_selectionBorder );
            editor->setAutoCompletion( m_autoCompletion );
            editor->setEndLine( m_endLine );
            editor->setTabSpaces( m_tabSpaces );
            editor->setBackgroundColor( m_backgroundColor );
            editor->setTextColor( m_textColor );
            editor->setCurrentLineColor( m_currentLineColor );
            editor->setMatchingColor( m_matchingColor );
            editor->setAutobrackets( m_autobrackets );
            editor->setShowWhiteSpaces( m_displayWhiteSpaces );
            editor->displayEditorToolbar( m_displayEditorToolbars );
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
        }
        tabExplorer->setTabEnabled( 1, m_showTreeClasses );
        if (!m_showTreeClasses) //ToolsOptions/General
            tabExplorer->setTabToolTip( 1, tr("Classes explorer is disabled, please enable it in the Options dialog") );
        else
            tabExplorer->setTabToolTip( 1, "" );
	    m_assistant->setdocumentationDirectory( m_documentationDirectory );
        QApplication::restoreOverrideCursor();
    }
    delete options;
}

//
void MainImpl::saveINI()
{
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

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
    settings.setValue("m_highlightCurrentLine", m_highlightCurrentLine);
    settings.setValue("m_checkEnvironmentOnStartup", m_checkEnvironmentOnStartup);
    settings.setValue("m_endLine", m_endLine);
    settings.setValue("m_tabSpaces", m_tabSpaces);
    settings.setValue("m_backgroundColor", m_backgroundColor.name());
    settings.setValue("m_textColor", m_textColor.name());
    settings.setValue("m_currentLineColor", m_currentLineColor.name());
    settings.setValue("m_matchingColor", m_matchingColor.name());
    settings.setValue("m_projectsDirectory", m_projectsDirectory);
    settings.setValue("m_pluginsDirectory", m_pluginsDirectory);
    settings.setValue("m_includeDirectory", m_includeDirectory);
    settings.setValue("m_documentationDirectory", m_documentationDirectory);
    settings.setValue("m_makeOptions", m_makeOptions);
    settings.setValue("m_mibCodec", m_mibCodec);
    //
    settings.setValue("m_formatPreprocessorText", m_formatPreprocessorText.foreground().color().name());
    settings.setValue("m_formatQtText", m_formatQtText.foreground().color().name());
    settings.setValue("m_formatSingleComments", m_formatSingleComments.foreground().color().name());
    settings.setValue("m_formatMultilineComments", m_formatMultilineComments.foreground().color().name());
    settings.setValue("m_formatQuotationText", m_formatQuotationText.foreground().color().name());
    settings.setValue("m_formatMethods", m_formatMethods.foreground().color().name());
    settings.setValue("m_formatKeywords", m_formatKeywords.foreground().color().name());
    settings.setValue("m_displayEditorToolbars", m_displayEditorToolbars);
    settings.setValue("m_displayWhiteSpaces", m_displayWhiteSpaces);
    settings.endGroup();

    // Save shortcuts
    settings.beginGroup("Shortcuts");
    QList<QObject*> childrens;
    childrens = children();
    QListIterator<QObject*> iterator(childrens);
    while ( iterator.hasNext() )
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
    //if ( !m_projectManager )
    //return;

    if ( m_restoreOnStart )
    {
        settings.beginGroup("Project");
        QString projectDirectory;
        if ( m_projectManager )
            projectDirectory = m_projectManager->absoluteNameProjectFile(treeFiles->topLevelItem(0));
        settings.setValue("absoluteNameProjectFile", projectDirectory);
        settings.endGroup();
    }
    //
    settings.beginGroup("mainwindowstate");
    if (!isMinimized() && !isMaximized() && !isFullScreen())
    {
        settings.setValue("pos", pos());
        settings.setValue("size", size());
    }
    settings.setValue("maximized", isMaximized());
    settings.setValue("fullscreen", isFullScreen());
    settings.setValue("geometry", saveGeometry()); // Window geometry and state (only needed for Windows!).
    settings.setValue("state", saveState()); // Toolbar and DockWidget state.
    settings.setValue("tabExplorer", tabExplorer->currentIndex());
    settings.endGroup();
}
//
void MainImpl::slotNewProject()
{
    NewProjectImpl *window = new NewProjectImpl(this, m_projectsDirectory);
    window->labelProjetParent->setHidden( true );
    window->parentProjectName->setHidden( true );
    if ( window->exec() == QDialog::Accepted )
    {
        openProject( window->absoluteProjectName() );
        QTreeWidgetItem *itProject = m_projectManager->itemProject(  window->filename() );
        m_projectManager->setSrcDirectory(itProject, window->srcDirectory->text());
        m_projectManager->setUiDirectory(itProject, window->uiDirectory->text());
    }
    delete window;
}
//
QString MainImpl::loadINI()
{
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

    QString projectName;
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
    m_checkEnvironmentOnStartup = settings.value("m_checkEnvironmentOnStartup", m_checkEnvironmentOnStartup).toBool();
    m_autoMaskDocks = settings.value("m_autoMaskDocks", m_autoMaskDocks).toBool();
    m_endLine = (EndLine)settings.value("m_endLine", m_endLine).toInt();
    m_tabSpaces = settings.value("m_tabSpaces", m_tabSpaces).toBool();
    m_match = settings.value("m_match", m_match).toBool();
    m_highlightCurrentLine = settings.value("m_highlightCurrentLine", m_highlightCurrentLine).toBool();
    m_backgroundColor = QColor(settings.value("m_backgroundColor", m_backgroundColor).toString());
    m_textColor = QColor(settings.value("m_textColor", m_textColor).toString());
    m_currentLineColor = QColor(settings.value("m_currentLineColor", m_currentLineColor).toString());
    m_matchingColor = QColor(settings.value("m_matchingColor", m_matchingColor).toString());
    m_projectsDirectory = settings.value("m_projectsDirectory", m_projectsDirectory).toString();
    m_pluginsDirectory = settings.value("m_pluginsDirectory", m_pluginsDirectory).toString();
    m_includeDirectory = settings.value("m_includeDirectory", m_includeDirectory).toString();
    m_documentationDirectory = settings.value("m_documentationDirectory", m_documentationDirectory).toString();
    m_makeOptions = settings.value("m_makeOptions", m_makeOptions).toString();
    m_showTreeClasses = settings.value("m_showTreeClasses", m_showTreeClasses).toBool();
    m_closeButtonInTabs = settings.value("m_closeButtonInTabs", m_closeButtonInTabs).toBool();
    m_displayEditorToolbars = settings.value("m_displayEditorToolbars", m_displayEditorToolbars).toBool();
    slotUpdateOtherFileActions();
    m_displayWhiteSpaces = settings.value("m_displayWhiteSpaces", m_displayWhiteSpaces).toBool();

    setCrossButton( !m_closeButtonInTabs );
    m_intervalUpdatingClasses = settings.value("m_intervalUpdatingClasses", m_intervalUpdatingClasses).toInt();
    if ( m_currentLineColor == Qt::black )
        m_currentLineColor = QColor();
    m_mibCodec = settings.value("m_mibCodec", m_mibCodec).toInt();
    //
    m_formatPreprocessorText.setForeground( QColor(settings.value("m_formatPreprocessorText", m_formatPreprocessorText.foreground().color().name()).toString() ) );
    m_formatQtText.setForeground( QColor(settings.value("m_formatQtText", m_formatQtText.foreground().color().name()).toString() ) );
    m_formatSingleComments.setForeground( QColor(settings.value("m_formatSingleComments", m_formatSingleComments.foreground().color().name()).toString() ) );
    m_formatMultilineComments.setForeground( QColor(settings.value("m_formatMultilineComments", m_formatMultilineComments.foreground().color().name()).toString() ) );
    m_formatQuotationText.setForeground( QColor(settings.value("m_formatQuotationText", m_formatQuotationText.foreground().color().name()).toString() ) );
    m_formatMethods.setForeground( QColor(settings.value("m_formatMethods", m_formatMethods.foreground().color().name()).toString() ) );
    m_formatKeywords.setForeground( QColor(settings.value("m_formatKeywords", m_formatKeywords.foreground().color().name()).toString() ) );
    settings.endGroup();

    tabExplorer->setTabEnabled( 1, m_showTreeClasses );
    if (!m_showTreeClasses) //ToolsOptions/General
        tabExplorer->setTabToolTip( 1, tr("Classes explorer is disabled, please enable it in the Options dialog") );
    else
        tabExplorer->setTabToolTip( 1, "" );
    m_assistant->setdocumentationDirectory( m_documentationDirectory );
    // Load shortcuts
    settings.beginGroup("Shortcuts");
    QList<QObject*> childrens;
    childrens = children();
    QListIterator<QObject*> iterator(childrens);
    while ( iterator.hasNext() )
    {
        QObject *object = iterator.next();
        QAction *action = qobject_cast<QAction*>(object);

        if ( (action) && (!(action->data().toString().contains( "Recent|" ))) && (!action->objectName().isEmpty()) )
        {
            QString text = object->objectName();

            if ( !text.isEmpty() )
            {
                QString shortcut = action->shortcut();
                shortcut = settings.value(text, shortcut).toString();
                action->setShortcut( shortcut );
            }
        }
    }
    settings.endGroup();

    if ( m_restoreOnStart )
    {
        settings.beginGroup("Project");
        projectName = settings.value("absoluteNameProjectFile").toString();
        settings.endGroup();
    }
    //
    settings.beginGroup("mainwindowstate");
#ifdef Q_OS_WIN32
    // Restores position, size and state for both normal and maximized/fullscreen state. Problems reported unter X11.
    // See Qt doc: Geometry: Restoring a Window's Geometry for details.
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray()); // Window geometry and state.
#else
    // Restores position, size and state including maximized/fullscreen.
    move(settings.value("pos", pos()).toPoint()); // Window position.
    resize(settings.value("size", size()).toSize()); // Window size.
    // Note: Yes, the window can be maximized and fullscreen!
    if (settings.value("maximized", isMaximized()).toBool()) // Window maximized.
        setWindowState(windowState() | Qt::WindowMaximized);
    if (settings.value("fullscreen", isFullScreen()).toBool()) // Window fullscreen.
        setWindowState(windowState() | Qt::WindowFullScreen);
#endif
    restoreState(settings.value("state", saveState()).toByteArray()); // Toolbar and DockWidget state.
    tabExplorer->setCurrentIndex( settings.value("tabExplorer", 0).toInt() );
    settings.endGroup();
    return projectName;
}

void MainImpl::closeEvent( QCloseEvent * event )
{
    int response = 0;
    if ( m_promptBeforeQuit )
    {
        response = QMessageBox::question(this, "QDevelop",
                                         tr("Do you want to quit QDevelop ?"),
                                         tr("Yes"), tr("No"), QString(), 0, 1);
    }
    if ( response == 1 )
    {
        event->ignore();
        return;
    }
    saveINI();

    if ( slotCloseProject(true) )
    {
        delete m_assistant;
        delete m_designer;
        this->hide();  //give an impression of quick close
        event->accept();
    }
    else
        event->ignore();
}

//
bool MainImpl::slotCloseAllFiles()
{
    bool ok = true;
    foreach(Editor *editor, allEditors() )
    {
        if ( !editor->close() )
            ok = false;
        else
            delete editor;
    }
    slotUpdateOtherFileActions();
    return ok;
}

//
static QString dir;

void MainImpl::slotOpen()
{
	static QString selectedFilter;
	
    if ( dir.isEmpty() && m_projectManager )
        dir = m_projectManager->projectDirectory( treeFiles->topLevelItem ( 0 ) );

    QString s = QFileDialog::getOpenFileName(
                    this,
                    tr("Choose a file to open"),
                    dir,
                    tr("Sources")+" (*.cpp *.h);;"+
                    tr("Projects")+" (*.pro);;"+
                    tr("Texts")+" (*.txt *.TXT);;"+
                    tr("All Files")+" (* *.*)",
                    &selectedFilter
                );
    if ( s.isEmpty() )
    {
        // Cancel is clicked
        return;
    }
    if ( s.right(4).toLower() == ".pro" )
    {
        openProject(s);
    }
    else
        openFile( QStringList( s ) );
    dir = QDir().absoluteFilePath( s );
}

void MainImpl::slotOpenProject()
{
    if ( dir.isEmpty() && m_projectManager )
        dir = m_projectManager->projectDirectory( treeFiles->topLevelItem ( 0 ) );

    QString s = QFileDialog::getOpenFileName(
                    this,
                    tr("Choose a project to open"),
                    dir,
                    tr("Projects")+" (*.pro)"
                );
    if ( s.isEmpty() )
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
    s = QDir::cleanPath( s );
    QFile file ( s );
    if ( !file.exists() )
    {
        QMessageBox::warning(this,
                             "QDevelop", tr("The project %1 doesn't exist.").arg(s),
                             tr("Cancel") );
        return false;
    }
    if ( !slotCloseProject() )
        return false;
    if ( m_completion )
        delete m_completion;
    m_completion = new InitCompletion (this, treeClasses);
    QString includes;
    includes = m_includeDirectory;
#ifdef WIN32
    includes += "\" \"" + QDir::cleanPath( QFileInfo(m_qmakeName).absoluteDir().path()+"/../src" ) ;
#endif
    m_completion->setCtagsCmdPath( ctagsName() );
    m_completion->setQtInclude( includes );
    configureCompletion( QFileInfo(s).absoluteDir().path() );
    m_projectManager = new ProjectManager(this, treeFiles, treeClasses);
    m_projectManager->init(s);
    treeFiles->setProjectManager( m_projectManager );
    treeClasses->setProjectManager( m_projectManager );
    connect(actionResetExecutablesList, SIGNAL(triggered()), m_projectManager, SLOT(slotResetExecutablesList()) );
    connect(actionSaveProject, SIGNAL(triggered()), m_projectManager, SLOT(slotSaveProject()) );
    connect(actionAddExistingFiles, SIGNAL(triggered()), m_projectManager, SLOT(slotAddExistingFiles()) );
    connect(actionAddNewItem, SIGNAL(triggered()), m_projectManager, SLOT(slotAddNewItem()) );
    connect(actionAddNewClass, SIGNAL(triggered()), m_projectManager, SLOT(slotAddNewClass()) );
    connect(actionAddScope, SIGNAL(triggered()), m_projectManager, SLOT(slotAddScope()) );
    connect(actionProjectPropertie, SIGNAL(triggered()), m_projectManager, SLOT(slotProjectPropertie()) );
    setWindowTitle( s );
    setCurrentProject( s );
    m_projectGroup->setEnabled( true );
    slotClickTreeFiles( treeFiles->topLevelItem ( 0 ), 0);
    return true;
}
//
bool MainImpl::slotCloseProject(bool /*hide*/)
{
	/*QList<Editor *> maximized = m_maximizedEditors;
	foreach(Editor *editor, maximized)
	{
		slotShowMaximized( editor );
	}*/
    if ( m_projectManager )
        m_projectManager->saveProjectSettings();
    slotClearAllBookmarks();
    if ( !slotCloseAllFiles() )
        return false;
    logBuild->clear();
    logDebug->clear();
    //
    if ( m_completion )
    {
        m_completion->wait();
        delete m_completion;
        m_completion = 0;
    }
    //
    if ( m_projectManager && !m_projectManager->close() )
        return false;
    delete m_projectManager;
    m_projectManager = 0;
    setWindowTitle( "QDevelop" );
    m_projectGroup->setEnabled( false );
    delete m_findInFiles;
    m_findInFiles = 0;
    return true;
}
//
void MainImpl::slotDoubleClickTreeFiles(QTreeWidgetItem *item, int)
{
    if ( item->childCount() > 0 )
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
    actionProjectPropertie->setText( tr("Properties of %1...").arg(projectName) );
}
//
void MainImpl::slotSaveFile()
{
    Editor *editor = currentEditor();
    if ( editor )
    {
        editor->save();
    }
}
//
void MainImpl::slotSaveFileAs()
{
    Editor *editor = currentEditor();
    if ( !editor )
        return;
    QString s = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose the file to create"),
                    editor->filename(),
                    tr("Files (*.cpp *.h *.txt *.* *)") );

    if ( s.isEmpty() )
    {
        return;
    }
    editor->setFilename( s );
    editor->save();
    m_tabEditors->setTabText(m_tabEditors->currentIndex(), editor->shortFilename() );
}
//
void MainImpl::slotHelpQtWord()
{
    Editor *editor = currentEditor();
    QString className = "index";
    QString word = "index";
    if ( editor )
    {
        word = editor->wordUnderCursor();
        className = editor->classNameUnderCursor();
        if (className.isEmpty() )
            className = word;
        if( !word.isEmpty() )
        	className = m_completion->classForFunction(className, word);
        if ( className.isEmpty() && word.startsWith("Q") )
        {
        	className = word;
       	}
    }
    m_assistant->showQtWord(className, word );
}
//
void MainImpl::slotCloseCurrentTab()
{
    Editor *editor = currentEditor();
    if ( editor && !editor->close() )
        return;
    delete editor;
    slotUpdateOtherFileActions();
}
//
void MainImpl::closeTab(int numTab)
{
    Editor *editor = givenEditor(numTab);
    if ( editor && !editor->close() )
        return;
    delete editor;
    slotUpdateOtherFileActions();
}
//
void MainImpl::closeOtherTab(int numTab)
{
    Editor *noClose = givenEditor(numTab);
    QList<Editor *> editorlist;
    for (int i=0; i<m_tabEditors->count(); i++)
        editorlist.append( givenEditor(i) );
    foreach(Editor *editor, editorlist)
    {
        if ( editor != noClose )
        {

            if ( editor && !editor->close() )
                return;
            delete editor;
        }
    }
    slotUpdateOtherFileActions();
}
//
void MainImpl::slotClearAllBookmarks()
{
    foreach(Editor *editor, allEditors() )
    {
        if ( editor )
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
    foreach(Editor *editor, allEditors() )
    {
        if ( editor )
        {
            if ( !editor->save() )
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
        if ( QFile::exists( s ) )
        {
            break;
        }
    }
    s = QDir::cleanPath( s );
    // The file is really opened only if it exists and if it is a file.
    if ( !QFileInfo(s).isFile() )
    {
        QApplication::restoreOverrideCursor();
        return 0;
    }
    if ( Editor::shortFilename(s).section(".", -1, -1).toLower() == "ui" )
    {
        //QProcess::startDetached (m_designerName, QStringList(s));
        m_designer->openUI( s );
        QApplication::restoreOverrideCursor();
        return 0;
    }
    else if ( Editor::shortFilename(s).section(".", -1, -1).toLower() == "ts" )
    {
        QProcess::startDetached (m_linguistName, QStringList(s));
        QApplication::restoreOverrideCursor();
        return 0;
    }
    // The file is perhaps already opened. Find filename in tabs.
    foreach(Editor *editor, allEditors() )
    {
        if ( editor->filename() == s)
        {
		    for (int i=0; i<m_tabEditors->count(); i++)
		    {
		    	if( givenEditor(i)->filename() == s)
		    	{
            		m_tabEditors->setCurrentIndex( i );
	    		}
		    }
            if ( numLine != -1 )
            {
                editor->setFocus();
                editor->gotoLine( numLine, moveTop );
                slotCurrentTabChanged( 0 );
            }
            QApplication::restoreOverrideCursor();
            return editor;
        }
    }
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
    editor->setTextColor( m_textColor );
    editor->setMatch( m_match );
    editor->setHighlightCurrentLine( m_highlightCurrentLine );
    editor->setMatchingColor( m_matchingColor );
    editor->setCurrentLineColor( m_currentLineColor );
    editor->setShowWhiteSpaces( m_displayWhiteSpaces );
    editor->displayEditorToolbar( m_displayEditorToolbars );
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

    if ( !editor->open(silentMode) )
    {
        delete editor;
        QApplication::restoreOverrideCursor();
        return 0;
    }
    editor->setTabStopWidth( m_tabStopWidth );
    m_tabEditors->setCurrentIndex( m_tabEditors->addTab(editor, editor->shortFilename()+"   ") );
    editor->setFocus();
    if ( numLine != -1 )
        editor->gotoLine(numLine, moveTop);
    connect(editor, SIGNAL(editorModified(Editor *, bool)), this, SLOT(slotModifiedEditor( Editor *, bool)) );
    connect(editor, SIGNAL(updateClasses(QString, QString)), this, SLOT(slotUpdateClasses(QString, QString)) );
    connect(editor, SIGNAL(otherFileChanged()), this, SLOT(slotUpdateOtherFileActions()));
    if ( m_debug )
        connect(editor, SIGNAL(breakpoint(QString, bool, unsigned int, QString)), m_debug, SLOT(slotBreakpoint(QString, bool, unsigned int, QString)) );
    setCurrentFile(s);
    slotCurrentTabChanged( m_tabEditors->currentIndex() );
    slotUpdateOtherFileActions();

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
    if ( activate )
    {
        QString s = text;
        if ( s.length() > 50 )
            s = s.left(50)+" ...";
        QAction *action = new QAction(s, menuBookmarks);
        connect(action, SIGNAL(triggered()), this, SLOT(slotActivateBookmark()));
        //
        QAction *before = 0;
        QList<QAction *> actionsList = menuBookmarks->actions();
        foreach(QAction *actionBefore, actionsList)
        {
            Bookmark bookmarkAction = actionBefore->data().value<Bookmark>();
            if ( bookmarkAction.first == editor && editor->currentLineNumber( bookmarkAction.second ) > line )
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
            if ( bookmarkAction.first == editor && bookmarkAction.second == block )
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
    if ( action )
        actionActiveBookmark = action;
    else
        actionActiveBookmark = (QAction *)sender();
    Bookmark bookmark = actionActiveBookmark->data().value<Bookmark>();
    Editor *editor = 0;
    Editor *bookmarkEditor = bookmark.first;
    QTextBlock block = bookmark.second;
    foreach(Editor *edit, allEditors() )
    {
        if ( edit == bookmarkEditor )
        {
            editor = edit;
            break;
        }
    }
    if ( editor )
    {
        QString filename = editor->filename();
        int line = editor->currentLineNumber( block );
        openFile(QStringList(filename), line);
    }
}
//
void MainImpl::slotUpdateClasses(QString filename, QString buffer)
{
    if ( !m_projectManager )
        return;
    QString ext = "." + filename.section(".", -1, -1);
    QList<QTreeWidgetItem *> projectsList;
    m_projectManager->childsList(0, "PROJECT", projectsList);
    for (int nbProjects=0; nbProjects < projectsList.count(); nbProjects++)
    {
        QString projectName = projectsList.at(nbProjects)->text(0);
        QString projectDir = m_projectManager->findData(projectName, "projectDirectory");
        QStringList files;
        if ( ext == ".cpp" )
            m_projectManager->sources( m_projectManager->itemProject(projectName), files );
        else
            m_projectManager->headers( m_projectManager->itemProject(projectName), files );
        foreach(QString s, files)
        {
            if ( QDir::cleanPath(s) == QDir::cleanPath(filename) )
            {
                QStringList parents = m_projectManager->parents(projectsList.at(nbProjects));
                treeClasses->updateClasses(QDir::cleanPath(filename), buffer, parents, ext);
            }
        }
    }
}
//
void MainImpl::slotModifiedEditor( Editor *editor, bool modified)
{
    for (int i=0; i<m_tabEditors->count(); i++)
    {
        if ( givenEditor(i) == editor )
        {
            if ( modified && m_tabEditors->tabText(i).left(1) != "*" )
                m_tabEditors->setTabText(i, "* "+m_tabEditors->tabText(i) );
            if ( !modified && m_tabEditors->tabText(i).left(1) == "*" )
                m_tabEditors->setTabText(i, m_tabEditors->tabText(i).mid(2) );
            return;
        }
    }
    foreach(Editor *e, allEditors() )
    {
    	if( e == editor )
    	{
            if ( modified && e->windowTitle().left(1) != "*" )
                e->setWindowTitle("* "+e->windowTitle() );
            if ( !modified && e->windowTitle().left(1) == "*" )
                e->setWindowTitle( e->windowTitle().mid(2) );
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
    Editor *editor = currentEditor();
    if ( editor && Editor::suffixe( editor->filename() ).toLower() == "cpp" )
    {
        m_buildingGroup->setEnabled( false );
        logBuild->clear();
        dockOutputs->setVisible(true);
        if ( m_saveBeforeBuild )
            slotSaveAll();
        tabOutputs->setCurrentIndex( 0 );
        m_projectsDirectoriesList << editor->directory();
        QString projectDirectory = m_projectManager->fileDirectory(editor->filename() );
        m_builder = new Build(this, m_qmakeName, m_makeName, m_makeOptions, projectDirectory+"/", false, false, true, editor->filename());

        connect(m_builder, SIGNAL(finished()), this, SLOT(slotEndBuild()) );
        connect(m_builder, SIGNAL(finished()), m_builder, SLOT(deleteLater()) );
        connect(m_builder, SIGNAL(message(QString, QString)), logBuild, SLOT(slotMessagesBuild(QString, QString)) );
        m_builder->start();
    }
}
//
void MainImpl::slotBuild(bool clean, bool build, bool forceQmake)
{
    bool qmakeNeeded = false;
    if (!m_projectManager)
    {
        return;
    }
    if ( m_qmakeName.isEmpty() || m_makeName.isEmpty() )
    {
        slotToolsControl();
        return;
    }
    if ( actionDebug->text() == tr("Stop") && !slotDebug())
        return;
    m_buildAfterDebug = false;
    qmakeNeeded = m_projectManager->isModifiedProject();
    if ( m_projectsDirectoriesList.count() == 0 )
    {
        m_buildingGroup->setEnabled( false );
        logBuild->clear();
        dockOutputs->setVisible(true);
        if ( m_saveBeforeBuild )
            slotSaveAll();
        tabOutputs->setCurrentIndex( 0 );
        //
        m_projectsDirectoriesList = m_projectManager->buildableProjectsDirectories();
        m_clean = clean;
        m_build = build;
    }
    QString projectDirectory = m_projectsDirectoriesList.first();
    QString projectName = m_projectManager->projectName( projectDirectory );

    QString makefilePath = projectDirectory + "/Makefile";
    qmakeNeeded = qmakeNeeded || !QFile::exists(makefilePath) || forceQmake;
    if ( qmakeNeeded )
    {
        m_configureCompletionNeeded = true;
    }
    m_builder = new Build(this, m_qmakeName, m_makeName, m_makeOptions, projectDirectory+"/"+projectName, qmakeNeeded|m_clean, m_clean, m_build);

    connect(logBuild, SIGNAL(incErrors()), m_builder, SLOT(slotIncErrors()) );
    connect(logBuild, SIGNAL(incWarnings()), m_builder, SLOT(slotIncWarnings()) );
    connect(m_builder, SIGNAL(finished()), this, SLOT(slotEndBuild()) );
    connect(m_builder, SIGNAL(finished()), m_builder, SLOT(deleteLater()) );
    connect(m_builder, SIGNAL(message(QString, QString)), logBuild, SLOT(slotMessagesBuild(QString, QString)) );
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
    if ( m_configureCompletionNeeded )
        configureCompletion(m_projectsDirectoriesList.first());
    m_projectsDirectoriesList.removeFirst();
    if ( m_projectsDirectoriesList.count() )
        slotBuild();
    else
    {
        QString msg;
        if ( m_builder->nbErrors()==0 && m_builder->nbWarnings()==0 )
            msg = tr("Build finished without error");
        else
            msg = tr("Build finished with")+" ";
        if ( m_builder->nbErrors() )
            msg += QString::number(m_builder->nbErrors())+" "+tr("error(s)")+ (m_builder->nbWarnings() ? " "+tr("and")+ " " : QString(" "));
        if ( m_builder->nbWarnings() )
            msg += QString::number(m_builder->nbWarnings())+" "+tr("warning(s)")+" ";
        logBuild->slotMessagesBuild( QString("\n---------------------- "+msg+"----------------------\n"), "");
        m_buildingGroup->setEnabled( true );
        if ( m_debugAfterBuild )
            slotDebug( (int)m_debugAfterBuild-1 );
    }
}
//
void MainImpl::slotFindFilesActivated(QListWidgetItem *item, QListWidgetItem *)
{
    findLines->clear();
    if ( !item )
        return;
    QStringList list = item->data(Qt::UserRole).toStringList();
    findLines->addItems( list );
}
//
void MainImpl::slotDoubleClickFindLines( QListWidgetItem *item)
{
    QString texte = item->text();
    int numLine = texte.section(":", 0, 0).section(" ", 1,1).toInt();
    if ( numLine == 0 )
        return;
    QListWidgetItem *it = findFiles->currentItem();
    if ( !it )
        it = findFiles->item(0);
    QString filename = it->data(Qt::UserRole+1).toString();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    openFile( QStringList( filename ), numLine, false, true);
    QApplication::restoreOverrideCursor();
}
//
void MainImpl::slotCut()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->cut();
}
//
void MainImpl::slotPrint()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->print();
}
//
void MainImpl::slotCurrentTabChanged(int)
{
	foreach(Editor *editor, allEditors() )
	{
		editor->setActiveEditor( editor == currentEditor() );
	}
}
//
void MainImpl::slotCopy()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->copy();
}
//
void MainImpl::slotCompleteCode()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->completeCode();
}
//
void MainImpl::slotPaste()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->paste();
}
//
void MainImpl::slotUndo()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->undo();
}
//
void MainImpl::slotIndent()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->slotIndent();
}
//
void MainImpl::slotUnindent()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->slotUnindent();
}
//
void MainImpl::slotRedo()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->redo();
}
//
void MainImpl::slotSelectAll()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->selectAll();
}
//
void MainImpl::slotFind()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->find();
}
//
void MainImpl::slotReplace()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->replace();
}
//
void MainImpl::slotGotoLine()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->dialogGotoLine();
}
//
void MainImpl::slotFindContinue()
{
    Editor *editor = currentEditor();
    if ( editor )
        editor->findContinue();
}
//
void MainImpl::slotFindPrevious()
{
    Editor *editor = currentEditor();
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
    foreach(Editor *editor, allEditors() )
    {
        if ( editor )
        {
            if ( editor->isModified() )
                modified = true;

        }
    }
    return modified;
}
//
bool MainImpl::slotDebug(bool executeOnly)
{
    if (!m_projectManager)
    {
        return false;
    }
    if ( actionDebug->text() == tr("Start")
            && ( m_projectManager->qmake()
                 || modifiedEditors() ) )
    {
        // Proposer sauvegarde
        int choice = QMessageBox::question(this, "QDevelop",
                                           tr("The project has been modified, do you want to save your changes ?"),
                                           tr("Yes"), tr("No"), tr("Cancel"),
                                           0, 2 );
        if ( choice == 2 )
            return false;
        if ( choice == 0 )
        {
            m_debugAfterBuild = (ExecuteVersion)(executeOnly+1);
            slotBuild();
            return true;
        }
    }
    QString exeName;
    if ( actionDebug->text() == tr("Stop") )
    {
        int choice = QMessageBox::question(this, "QDevelop",
                                           tr("Stop debugging ?"),
                                           tr("Yes"), tr("No") );
        if ( choice == 1 )
            return false;
    }
    else
        exeName = m_projectManager->executableName( executeOnly ? "release" : "debug");
    executeOnly = m_projectManager->isReleaseVersion();
    m_debugAfterBuild = ExecuteNone;
    if ( exeName.isEmpty() && actionDebug->text() != tr("Stop"))
    {
        QMessageBox::critical(0, "QDevelop",
                              tr("The program doesn't exist,")+"\n"+
                              tr("run Build."),tr("Ok") );
        return false;
    }
    emit resetExecutedLine();
    if ( actionDebug->text() == tr("Stop") )
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
    registersImpl->registers(QString());
    dockOutputs->setVisible(true);
    tabOutputs->setCurrentIndex( 1 );
    Parameters parameters = m_projectManager->parameters();
    if ( parameters.workingDirectory.isEmpty() )
        parameters.workingDirectory = m_projectManager->projectDirectoryOfExecutable();
    m_stack->setDirectory( m_projectManager->projectDirectoryOfExecutable() );
    m_debug = new Debug(this, registersImpl, m_gdbName, parameters, exeName, executeOnly);
    if ( !executeOnly )
    {
        foreach(Editor *editor, allEditors() )
        {
            connect(editor, SIGNAL(breakpoint(QString, unsigned int, BlockUserData *)), m_debug, SLOT(slotBreakpoint(QString, unsigned int, BlockUserData *)) );
            editor->emitListBreakpoints();
        }
        QStringList list;
        for (int i=0; i < tableOtherVariables->rowCount(); i++)
            list << tableOtherVariables->item(i, 0)->text();
        emit otherVariables(list);
    }

    if ( !executeOnly )
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
    m_stack->clear();
    while ( tableLocalVariables->rowCount() )
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
        if ( var.kind == Debug::Local )
        {
            int row = tableLocalVariables->rowCount();
            tableLocalVariables->setRowCount(row+1);
            tableLocalVariables->setItem(row, 0, newItem1);
            tableLocalVariables->setItem(row, 1, newItem2);
            tableLocalVariables->setItem(row, 2, newItem3);
            tableLocalVariables->setItem(row, 3, newItem4);
        }
        else if ( var.kind == Debug::OtherArgs )
        {
            for (int i=0; i < tableOtherVariables->rowCount(); i++)
            {
                if ( tableOtherVariables->item(i, 0)->text() == var.name )
                {
                    tableOtherVariables->setItem(i, 1, newItem2);
                    tableOtherVariables->setItem(i, 2, newItem3);
                    tableOtherVariables->setItem(i, 3, newItem4);
                }
            }
        }
    }
}
//
void MainImpl::slotEndDebug()
{
    m_buildingGroup->setEnabled( true );
    actionDebug->setText(tr("Start"));
    actionDebug->setShortcut( tr("F5") );
    actionDebug->setIcon( QIcon(":/toolbar/images/dbgrun.png") );
    actionStopDebug->setIcon( QIcon(":/toolbar/images/pause.png") );
    actionStopDebug->setText( tr("Abort") );
    actionStopDebug->setToolTip( tr("Abort") );
    while ( tableLocalVariables->rowCount() )
        tableLocalVariables->removeRow(0);
    emit resetExecutedLine();
    if ( m_buildAfterDebug )
        slotBuild();
}
//
void MainImpl::slotMessagesDebug(QString message)
{
    char identifiantPointArret[] = { 26, 26, 0x0 };
    if ( message.indexOf( identifiantPointArret )==0 )
    {
        actionStopDebug->setEnabled( true );
        QString filename = message.section(":", 0, -5).mid(2);
        int numLine = message.section(":", -4, -4).toInt();
        if ( !filename.isEmpty() && numLine )
        {
            Editor *editor = openFile( QStringList(filename), numLine, true);
            if ( editor )
                editor->setExecutedLine( numLine );
            slotOnPause();
            setWindowState(windowState() & ~Qt::WindowMinimized);
            raise();
            activateWindow();
        }
    }
    else if ( message.simplified().indexOf( '#' ) == 0 )
    {
        m_stack->addLine( message );
    }
    else if ( message.indexOf( "InfoSources" ) == 0 )
    {
        m_stack->infoSources( message );
    }
    else if ( message.indexOf( "Registers" ) == 0 )
    {
        registersImpl->registers( message );
    }
    else if ( message.indexOf( "Breakpoint" ) == 0 )
    {
        // Nothing
    }
    else
    {
        logDebug->append( message );
    }
}
//
void MainImpl::slotContinueDebug()
{
    if ( actionStopDebug->text() == tr("Abort") )
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
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

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
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

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

    for (int i = 0; i < numRecentFiles; ++i)
    {
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
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

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
    QSettings settings(getQDevelopPath() + "qdevelop.ini", QSettings::IniFormat);

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
    if ( !m_findInFiles )
    {
        QStringList directories;
        if ( m_projectManager )
        {
            QList<QTreeWidgetItem *> listeProjets;
            m_projectManager->childsList(0, "PROJECT", listeProjets);
            for (int nbProjets=0; nbProjets < listeProjets.count(); nbProjets++)
            {
                directories << m_projectManager->projectDirectory(listeProjets.at(nbProjets)->text(0));
            }
        }

        m_findInFiles = new FindFileImpl(this, directories, findFiles, findLines);
    }
    else
    {
        // BK - allow find in files dialog to be moved around
        // and on signal set the focus.
        QRect rect = m_findInFiles->geometry();
        m_findInFiles->hide();
        m_findInFiles->setGeometry(rect);
    }

    //read selected text or current word
    Editor *editor = currentEditor();
    if ( editor )
    {
        m_findInFiles->setDefaultWord(editor->selection());
    }

    dockOutputs->setVisible(true);
    tabOutputs->setCurrentIndex( 4 );
    m_findInFiles->show();
    // Not delete dialog to save options, location and pattern on next showing.
}
//
void MainImpl::slotToolsControl(bool show)
{
    ToolsControlImpl *toolsControlImpl = new ToolsControlImpl( this );
    if ( (!toolsControlImpl->toolsControl() && m_checkEnvironmentOnStartup ) || show )
    		// toolsControlImpl->toolsControl() is always called so that there was a check done - if it isn't called, all tools will appear valid if m_checkEnvironmentOnStartup is disabled
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
    m_checkEnvironmentOnStartup = toolsControlImpl->checkEnvOnStartup();
    m_assistant->setName( toolsControlImpl->assistantName() );
    m_designer->setName( toolsControlImpl->designerName() );
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
    for (int i=0; i < tableOtherVariables->rowCount(); i++)
    {
        if ( tableOtherVariables->item(i, 0)->text() == var )
        {
            QMessageBox::warning(0,
                                 "QDevelop", tr("The variable \"%1\"\n already exists.").arg(var),
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
    for (int i=0; i < tableOtherVariables->rowCount(); i++)
        list << tableOtherVariables->item(i, 0)->text();
    emit otherVariables(list);
}
//
void MainImpl::slotRemoveDebugVariable()
{
    if ( tableOtherVariables->currentRow() == -1 )
        return;
    tableOtherVariables->removeRow( tableOtherVariables->currentRow() );
    QStringList list;
    for (int i=0; i < tableOtherVariables->rowCount(); i++)
        list << tableOtherVariables->item(i, 0)->text();
    emit otherVariables(list);
}
//
void MainImpl::slotOpenFile()
{
    if ( !m_projectManager )
        return;
    OpenFileImpl dialog(this, m_projectManager, this);
    dialog.exec();
}
//
void MainImpl::loadPlugins()
{
    QStringList entryList;
#if defined(Q_OS_WIN)
    if ( m_pluginsDirectory.isEmpty() )
    {
        QDir dir = QDir(qApp->applicationDirPath()+"/plugins");
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
    }
    else
    {
        QDir dir = QDir(m_pluginsDirectory);
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
    }
#else
    if ( m_pluginsDirectory.isEmpty() )
    {
        QDir dir = QDir("/usr/lib/qdevelop/plugins");
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
        dir = QDir("~/.qdevelop/plugins");
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
        // for linux only
        dir = QDir(qApp->applicationDirPath()+"/plugins/");
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
    }
    else
    {
        QDir dir = QDir(m_pluginsDirectory);
        foreach(QString fileName, dir.entryList(QDir::Files) )
        {
            entryList += dir.absoluteFilePath(fileName);
        }
    }
#endif
    //
    foreach (QString fileName, entryList)
    {
        QPluginLoader loader(fileName);
        QObject *plugin = loader.instance();
        if (plugin)
        {
            TextEditInterface *iTextEdit = qobject_cast<TextEditInterface *>(plugin);
            if (iTextEdit)
            {
                QAction *action = new QAction(iTextEdit->menuName(), plugin);
                connect(action, SIGNAL(triggered()), this, SLOT(slotTextEditPlugin()));
                menuPlugins->addAction(action);
                if ( iTextEdit->hasConfigDialog() )
                {
                    QAction *action = new QAction(iTextEdit->menuName(), plugin);
                    connect(action, SIGNAL(triggered()), this, SLOT(slotConfigPlugin()));
                    menuPluginsSettings->addAction(action);
                }
            }
        }
    }
    if ( menuPlugins->actions().isEmpty() )
    {
        delete menuPlugins;
        delete menuPluginsSettings;
    }
}
//
void MainImpl::slotTextEditPlugin()
{
    Editor *editor = currentEditor();
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
    if ( iTextEdit )
        iTextEdit->config();
}
//
void MainImpl::resetProjectsDirectoriesList()
{
    if ( m_projectsDirectoriesList.count() )
        m_projectsDirectoriesList = QStringList(QString());
}
//
void MainImpl::resetDebugAfterBuild()
{
    m_debugAfterBuild = ExecuteNone;
}
//
void MainImpl::slotNewQtVersion()
{
    if( m_buildQtDatabase )
    {
   		QMessageBox::information(this, "QDevelop", tr("The Qt database building is already in progress."));
   		return;
   	}
	QSqlDatabase::removeDatabase(getQDevelopPath() + "qdevelop.db");
	QFile::remove( getQDevelopPath() + "qdevelop.db" );
	m_buildQtDatabaseAsked = true;
	checkQtDatabase();
}
//
void MainImpl::checkQtDatabase()
{
	actionNewQtVersion->setEnabled(false);
    m_buildQtDatabase = new InitCompletion (this, treeClasses);
    connect(m_buildQtDatabase, SIGNAL(finished()), m_buildQtDatabase, SLOT(deleteLater()) );
    connect(m_buildQtDatabase, SIGNAL(showMessage(QString)), this, SLOT(slotShowMessage(QString)) );
    connect(m_buildQtDatabase, SIGNAL(finished()), this, SLOT(slotBuildQtDatabaseEnded()) );
    QString includes;
    includes = m_includeDirectory;
#ifdef WIN32
    includes += "\" \"" + QDir::cleanPath( QFileInfo(m_qmakeName).absoluteDir().path()+"/../src" ) ;
#endif
    m_buildQtDatabase->setCtagsCmdPath( ctagsName() );
    m_buildQtDatabase->setQtInclude( includes );
    m_buildQtDatabase->slotInitParse(InitCompletion::CheckQtDatabase, QString(), QString(), true, false, true, QString());
	//
}

void MainImpl::slotQmake()
{
	slotBuild(false, false, true);
}
//
void MainImpl::slotShowMessage(QString message)
{
	QMessageBox::information(this, "QDevelop", message);
}
//
void MainImpl::slotBuildQtDatabaseEnded()
{
	actionNewQtVersion->setEnabled(true);
	m_buildQtDatabase = 0;
    if( m_buildQtDatabaseAsked )
    	QMessageBox::information(this, "QDevelop", tr("The Qt classes database build is ended.") );
}

QList<Editor *> MainImpl::allEditors()
{
    QList<Editor *> editorList;
    for (int i=0; i<m_tabEditors->count(); i++)
        editorList.append( givenEditor(i) );
    return editorList;
}

void MainImpl::slotUpdateOtherFileActions()
{
    Editor *editor = currentEditor();
    if ( !m_displayEditorToolbars && editor && editor->hasOtherFile() ) {
        actionOtherFile->setToolTip(editor->getOtherFileToolTip());
        actionOtherFile->setIcon(QIcon(editor->getOtherFileIcon()));
        separatorOtherFile->setVisible(true);
        actionOtherFile->setVisible(true);
    }
    else {
        separatorOtherFile->setVisible(false);
        actionOtherFile->setVisible(false);
    }
}

void MainImpl::on_actionEditor_mode_triggered()
{
	QWidget *w;
	bool editMode = actionEditor_mode->isChecked();
	QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
	
	foreach( w, dockWidgets )
		w->setVisible( ! editMode );
}
