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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QPointer>
#include <QTextCursor>
#include "mainimpl.h"
#include "InitCompletion.h"
#include "ui_findwidget.h"
//
typedef QMap<QString, QStringList> classesMethodsList; 
//
class LineNumbers;
class SelectionBorder;
class Editor;
class FindImpl;
class CppHighlighter;
//
class TextEdit : public QTextEdit
{
Q_OBJECT
public:
	TextEdit(Editor * parent, MainImpl *mainimpl, InitCompletion *completion);
	~TextEdit();
	bool open(bool silentMode, QString filename, QDateTime &lastModified);
	bool save(QString filename, QDateTime &lastModified);
	bool close(QString filename);
	QTextCursor getLineCursor( int line ) const;
	void setLineNumbers( LineNumbers* );
	LineNumbers* lineNumbers();
	void setSelectionBorder( SelectionBorder* );
	SelectionBorder* selectionBorder();
	void findText();
	QList<int> breakpoints() { return m_breakpoints; };
	void deleteBreakpoint(int line) { m_breakpoints.removeAll( line ); };
	void setExecutedLine(int line);
	void emitBreakpointsList();
	void selectLines(int debut, int fin);
	QString wordUnderCursor(const QPoint & pos=QPoint(), bool select=false);
	QString classNameUnderCursor();
	QString methodeMotSousCurseur();
	void activateLineNumbers(bool activate);
	void setSelectionBorder(bool activate);
	void setSyntaxHighlight(bool activate );
	void setAutoIndent( bool activate ) { m_autoindent = activate; };
	void setAutoCompletion( bool activate ) { m_autoCompletion = activate; };
	void setTabStopWidth(int taille);
	void setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g);
	void setEndLine(MainImpl::EndLine end) { m_endLine = end; };
	void setTabSpaces(bool t) { m_tabSpaces = t; };
	void setAutobrackets(bool b) { m_autobrackets = b; };
	void setBackgroundColor( QColor c );
	void setCurrentLineColor( QColor c );
	int currentLineNumber();
	int linesCount();
    void dialogGotoLine();
    void completeCode();
    void setFocus(Qt::FocusReason reason);
    enum ActionComment {Toggle, Comment, Uncomment};
public slots:
	bool slotToggleBreakpoint(int line=0);
	void gotoLine( int line, bool moveTop );
	void slotFind(Ui::FindWidget ui, QString ttf=0,  QTextDocument::FindFlags options=0, bool fromButton=false);
	void slotIndent(bool indent=true);
	void slotUnindent();
	void paste();
	void undo();
	void redo();
	void cut();
	void comment(ActionComment action);
private slots:	
	void slotAdjustSize();
    void slotWordCompletion(QListWidgetItem *item);
	void slotCursorPositionChanged();
	void slotContentsChange ( int position, int charsRemoved, int charsAdded );
	void slotCompletionList(TagList TagList);
private:
	QPointer<LineNumbers> m_lineNumbers;
	QPointer<SelectionBorder> m_selectionBorder;
	CppHighlighter *cpphighlighter;
	//QTextCursor previousCursor;
	QString m_findExp;
	int m_findOptions;
	QList<int> m_breakpoints;
	FindImpl *m_findImpl;
	int lineNumber(QTextCursor cursor);
	int lineNumber(QPoint point);
	QPoint mousePosition;
	Editor *m_editor;
	MainImpl *m_mainImpl;
	bool m_autoindent;
	void autoIndent();
	void autobrackets();
	void autoUnindent();
	QAction *actionToggleBreakpoint;
	MainImpl::EndLine m_endLine;
	bool m_tabSpaces;
    InitCompletion *m_completion;
    QListWidget *m_completionList;
    bool m_autoCompletion;
    bool m_autobrackets;
    QColor m_backgroundColor;
    QColor m_currentLineColor;
protected:
	void resizeEvent( QResizeEvent* );
	void keyPressEvent ( QKeyEvent * event );
	void contextMenuEvent(QContextMenuEvent * e);
	void mouseDoubleClickEvent( QMouseEvent * event );
	void dropEvent( QDropEvent * event );
    void mousePressEvent ( QMouseEvent * event );
	void paintEvent ( QPaintEvent * event );
signals:
	void editorModified(bool);
};

#endif
