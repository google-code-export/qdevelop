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
#ifndef MAINIMPL_H
#define MAINIMPL_H

#include "ui_main.h"
#include "debug.h"
#include <QDateTime>
#include <QFont>
#include <QTextCharFormat>
#include "treeproject.h"

class QProcess;
class Editor;
class Build;
class QComboBox;
class ProjectManager;
class Assistant;
class QActionGroup;
class TabWidget;
class FindFileImpl;
class StackImpl;
class InitCompletion;
//
typedef QPair<QString, int> Bookmark;
Q_DECLARE_METATYPE(Bookmark)
//
class MainImpl : public QMainWindow, Ui::Main
{
Q_OBJECT
public:
	MainImpl(QWidget * parent = 0);
	~MainImpl();
	Editor * openFile(QStringList locationsList, int numLine=-1, bool silentMode=false, bool moveTop=false);
	void loadINI();
	bool saveBeforeBuild() { return m_saveBeforeBuild; };
	bool openProject(QString s);
	QString qmakeName() { return m_qmakeName; };
	QString ctagsName() { return m_ctagsName; };
	enum EndLine { Default, Unix, Windows };
	TabWidget *tabEditors() const { return m_tabEditors; };
	bool ctagsIsPresent() { return m_ctagsIsPresent; };
	QKeySequence shortcutCut() { return actionCut->shortcut(); };
	QKeySequence shortcutPaste() { return actionPaste->shortcut(); };
	QKeySequence shortcutUndo() { return actionUndo->shortcut(); };
	QKeySequence shortcutRedo() { return actionRedo->shortcut(); };
	void closeTab(int numTab);
	void closeOtherTab(int numTab);
	//void closeAllTab();
	bool showTreeClasses() { return m_showTreeClasses; };
private:
	// Functions
	void createConnections();
	void updateActionsRecentsFiles();
	void updateActionsRecentsProjects();
	QString strippedName(const QString &fullFileName);
	void setCurrentProject(const QString &file);
	void setCurrentFile(const QString &file);
	void saveINI();
	bool modifiedEditors();
	void configureCompletion();
	// Objects and Variables
	Build *m_builder;
	Debug *m_debug;
	ProjectManager *m_projectManager;
	enum { maxRecentsFiles = 10 };
	QAction *actionsRecentsFiles[maxRecentsFiles];
	enum { maxRecentsProjects = 10 };
	QAction *actionsProjetsRecents[maxRecentsProjects];
	QStringList m_projectsDirectoriesList;
	bool m_clean;
	bool m_build;
	Assistant *m_assistant;
	QFont m_font;
	//QTimer *m_timer;
	int m_tabStopWidth;
	QString m_qmakeName;
	QString m_makeName;
	QString m_gdbName;
	QString m_ctagsName;
	QString m_qtInstallHeaders;
	bool m_lineNumbers, m_selectionBorder, m_autoIndent, m_cppHighlighter;
	bool m_saveBeforeBuild;
	bool m_restoreOnStart;
	bool m_promptBeforeQuit;
	bool m_debugAfterBuild;
	bool m_buildAfterDebug;
	bool m_checkEnvironment;
	bool m_autoMaskDocks;
    bool m_autoCompletion;
    bool m_autobrackets;
    bool m_ctagsIsPresent;
    QColor m_backgroundColor;
    QColor m_currentLineColor;
	EndLine m_endLine;
	bool m_tabSpaces;
	QPointer<QActionGroup> m_projectGroup;
	QTextCharFormat m_formatPreprocessorText, m_formatQtText, m_formatSingleComments;
	QTextCharFormat m_formatMultilineComments, m_formatQuotationText, m_formatMethods, m_formatKeywords;
    TabWidget *m_tabEditors;
    FindFileImpl *m_findInFiles;
    StackImpl *m_stack;
    int m_intervalUpdatingClasses;
    bool m_showTreeClasses;
    InitCompletion *m_completion;
protected:
	void closeEvent( QCloseEvent * event );
private slots:
	void slotOpen();
	void slotSaveFile();
	void slotCompleteCode();
	void slotSaveFileAs();
	bool slotSaveAll();
	void slotBuild(bool clean=false, bool build=true);
	void slotCompile();
	void slotNewProject();
	void slotRebuild();
	void slotStopBuild();
	void slotClean();
	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotUndo();
	void slotRedo();
	void slotGotoLine();
	void slotSelectAll();
	void slotUnindent();
	void slotIndent();
	bool slotCloseProject();
	void slotFind();
	void slotReplace();
	void slotFindContinue();
	void slotEndBuild();
	bool slotDebug(bool executeOnly=false);
	void slotExecuteWithoutDebug();
	void slotContinueDebug();
	void slotEndDebug();
	void slotMessagesDebug(QString message);
	void slotMessagesBuild(QString list, QString directory);
	void slotDoubleClickLogBuild( QListWidgetItem *item);	
	void slotFindFilesActivated(QListWidgetItem *item, QListWidgetItem *);
	void slotDoubleClickFindLines( QListWidgetItem *item);	
	void slotModifiedEditor(Editor *editor, bool modified);
	void slotStepInto();
	void slotStepOver();
	void slotStepOut();
	void slotEditToGdb(QString);
	void slotOnPause();
	void slotOpenRecentFile();
	void slotOpenRecentProject();
	void slotAbout();
	void slotHelpQtWord();
	void slotOptions();
	void slotClickTreeFiles(QTreeWidgetItem *item, int);
	void slotCloseCurrentTab();
	void slotPreviousTab();
	void slotNextTab();
	void slotOtherFile();
	void slotSetFocusToEditor();
	void slotToggleBreakpoint();	
	void slotShortcuts();
	void slotFindInFiles();
	void slotNewFile();
	void slotToggleComment();
	void slotComment();
	void slotUncomment();
	void slotParameters();
	void slotBacktraces();
	void slotCurrentTabChanged(int index);
	void slotUpdateClasses(QString filename, QString buffer);
	void slotDebugVariables( QList<Variable> list);
	void slotAddDebugVariable();
	void slotRemoveDebugVariable();
	void slotToggleBookmark(QString filename, QString text, QPair<bool,unsigned int> bookmarkLine);
	void slotToggleBookmark();	
	void slotActivateBookmark();
	void slotClearAllBookmarks();
public slots:
	void slotDoubleClickTreeFiles(QTreeWidgetItem *item, int);
	bool slotCloseAllFiles();
	void slotToolsControl(bool show=true);
signals:
	void debugCommand(QString);
	void stopBuild();
	void stopDebug();
	void pauseDebug();
	void resetExecutedLine();
	void otherVariables(QStringList);
};

#endif
