/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006 - 2010 Jean-Luc Biord
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
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/
#ifndef MAINIMPL_H
#define MAINIMPL_H

#include "ui_main.h"
#include "debug.h"
#include <QDateTime>
#include <QFont>
#include <QTextCharFormat>
#include <QTextBlock>
#include "treeproject.h"

class QProcess;
class Editor;
class Build;
class QComboBox;
class ProjectManager;
class Assistant;
class Designer;
class QActionGroup;
class TabWidget;
class FindFileImpl;
class StackImpl;
class InitCompletion;
class GetLastVersion;

//
typedef QPair<Editor *, QTextBlock> Bookmark;
Q_DECLARE_METATYPE(Bookmark)
//
class MainImpl : public QMainWindow, public Ui::Main
{
Q_OBJECT
public:
	void automaticCompilationState(Editor *editor, int state);
	QList<Editor *> allEditors();
	MainImpl(QWidget * parent = 0);
	~MainImpl();
	enum ExecuteVersion { ExecuteNone, ExecuteRelease, ExecuteDebug};
	Editor * openFile(QStringList locationsList, int numLine=-1, bool silentMode=false, bool moveTop=false);
	QString loadINI();
	bool saveBeforeBuild() { return m_saveBeforeBuild; };
	bool openProject(QString s);
	QString qmakeName() { return m_qmakeName; };
	QString ctagsName() { return m_ctagsName; };
	QString lupdateName() { return m_lupdateName; };
	QString lreleaseName() { return m_lreleaseName; };
	enum EndLine { Default, Unix, Windows };
	TabWidget *tabEditors() const { return m_tabEditors; };
	bool ctagsIsPresent() { return m_ctagsIsPresent; };
	void closeTab(int numTab);
	void closeOtherTab(int numTab);
	GetLastVersion *m_httpVersion;
	bool showTreeClasses() { return m_showTreeClasses; };
	void toggleBookmark(Editor *editor, QString text, bool activate, QTextBlock block);
	QMenu *bookmarksMenu() { return menuBookmarks; };
	const QList<ParsedItem> *treeClassesItems() { return treeClasses->treeClassesItems(); };
	void loadPlugins();
	void renameEditor(QString oldName, QString newName);
	void configureCompletion(QString projectDirectory);
	void gotoFileInProject(QString& filename);
	Editor * currentEditor();
	Editor * givenEditor(int i);
	void resetProjectsDirectoriesList();
	void resetDebugAfterBuild();
	int mibCodec() { return m_mibCodec;	}
	void checkQtDatabase();
	InitCompletion* buildQtDatabase() { return m_buildQtDatabase; }
	inline void setQmakeForcedMode(const QString & mode) {
		actionDebug_mode->setChecked(false);
		actionRelease_mode->setChecked(false);
		if (mode == "debug")
		{
			actionDebug_mode->setChecked(true);
		}
		else if (mode == "release")
		{
			actionRelease_mode->setChecked(true);
		}
			
		m_lastForcedMode = mode;
	}
	inline QString qmakeForcedMode() const {
		return m_lastForcedMode;
	}
private:
	// Functions
	void createConnections();
	void updateActionsRecentsFiles();
	void updateActionsRecentsProjects();
	QString strippedName(const QString &fullFileName);
	void setCurrentProject(const QString &file);
	void setCurrentFile(const QString &file);
	void saveINI();
	void setCrossButton(bool activate);
	bool modifiedEditors();
	// Objects and Variables
	bool m_automaticCompilation;
	QColor m_textColor;
	bool m_buildQtDatabaseAsked;
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
	Designer *m_designer;
	QFont m_font;
	int m_tabStopWidth;
	QString m_qmakeName;
	QString m_makeName;
	QString m_makeOptions;
	QString m_gdbName;
	QString m_ctagsName;
	QString m_linguistName;
	QString m_lupdateName;
	QString m_lreleaseName;
	QString m_designerName;
	QString m_qtInstallHeaders;
	QString m_projectsDirectory;
	QString m_pluginsDirectory;
	QString m_includeDirectory;
	QString m_documentationDirectory;
	bool m_lineNumbers, m_selectionBorder, m_autoIndent, m_cppHighlighter;
	bool m_saveBeforeBuild;
	bool m_restoreOnStart;
	bool m_promptBeforeQuit;
	ExecuteVersion m_debugAfterBuild;
	bool m_buildAfterDebug;
	bool m_checkEnvironmentOnStartup;
	bool m_autoMaskDocks;
	bool m_autoCompletion;
	bool m_autobrackets;
	// Divius: comments autoclose
	bool m_autocomments;
	//
	bool m_match;
	bool m_highlightCurrentLine;
	bool m_ctagsIsPresent;
	bool m_configureCompletionNeeded;
	bool m_extraSelection;
	QColor m_extraSelectionColor;
	QColor m_backgroundColor;
	QColor m_currentLineColor;
	QColor m_matchingColor;
	EndLine m_endLine;
	bool m_tabSpaces;
	bool m_closeButtonInTabs;
	bool m_displayEditorToolbars;
	bool m_displayWhiteSpaces;
	int m_rightMarginLine;
	bool m_wordWrap;
	int m_hideFindReplace;
	QToolButton *crossButton;
	QPointer<QActionGroup> m_projectGroup;
	QPointer<QActionGroup> m_buildingGroup;
	QTextCharFormat m_formatPreprocessorText, m_formatQtText, m_formatSingleComments;
	QTextCharFormat m_formatMultilineComments, m_formatQuotationText, m_formatMethods, m_formatKeywords;
	TabWidget *m_tabEditors;
	FindFileImpl *m_findInFiles;
	StackImpl *m_stack;
	int m_intervalUpdatingClasses;
	bool m_showTreeClasses;
	InitCompletion *m_completion;
	InitCompletion *m_buildQtDatabase;
	QAction *actionActiveBookmark;
	int m_mibCodec;
	bool m_smartClick;
	//QList<Editor *> m_maximizedEditors;
	QAction *separatorOtherFile, *actionOtherFile;
	QHash<QWidget*,bool> toolbarStatus;
	QString m_lastForcedMode;
protected:
	void closeEvent( QCloseEvent * event );
private slots:
	void slotClearAllWarningsErrors();
	void slotPreviousWarningError();
	void slotNextWarningError();
	void slotQmake();
	void slotOpen();
	void slotOpenProject();
	void slotSaveFile();
	void slotCompleteCode();
	void slotSaveFileAs();
	bool slotSaveAll();
	void slotBuild(bool clean=false, bool build=true, bool forceQmake=false, QString forceMode = QString());
	void slotBuildRelease();
	void slotBuildDebug();
	void slotForgetBuildMode();
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
	void slotFind();
	void slotReplace();
	void slotFindContinue();
	void slotFindPrevious();
	void slotEndBuild();
	bool slotDebug(bool executeOnly=false);
	void slotExecuteWithoutDebug();
	void slotContinueDebug();
	void slotEndDebug();
	void slotMessagesDebug(QString message);
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
	void slotCurrentTabChanged(int index);
	void slotDebugVariables( QList<Variable> list);
	void slotAddDebugVariable();
	void slotRemoveDebugVariable();
	void slotToggleBookmark();	
	void slotActivateBookmark(QAction *action=0);
	void slotPreviousBookmark();
	void slotNextBookmark();
	void slotClearAllBookmarks();
	void slotGotoMatchingBracket();
	void slotSimplePlugin();
	void slotTextEditPlugin();
	void slotConfigPlugin();
	void slotPrint();
	void slotOpenFile();
	void slotGotoDeclaration();
	void slotGotoImplementation();
	void slotMethodsList();
	void slotNewQtVersion();
	void slotUpdateOtherFileActions();
	void on_actionMinimalDocksView_triggered();
	void on_actionFullDocksView_triggered();
public slots:
	void slotCompile(bool automaticCompilation=false);
	void slotBuildQtDatabaseEnded();
	void slotShowMessage(QString message);
	void slotDoubleClickTreeFiles(QTreeWidgetItem *item, int);
	bool slotCloseAllFiles();
	void slotToolsControl(bool show=true);
	void slotUpdateClasses(QString filename, QString buffer);
	bool slotCloseProject(bool hide=false);
	void on_actionEditor_mode_triggered(bool fromDblClickOnTab=false);
signals:
	void debugCommand(QString);
	void stopBuild();
	void stopDebug();
	void pauseDebug();
	void resetExecutedLine();
	void otherVariables(QStringList);
};

#endif
