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
class TextEditInterface;
class QPixmap;

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
    void setExecutedLine(int line);
    void selectLines(int debut, int fin);
    QString wordUnderCursor(const QPoint & pos=QPoint(), bool select=false);
    QString wordUnderCursor(const QString text);
    QString classNameUnderCursor(const QPoint & pos=QPoint(), bool addThis=true);
    void activateLineNumbers(bool activate);
    void setSelectionBorder(bool activate);
    void setSyntaxHighlight(bool activate );
    void setAutoIndent( bool activate )
    {
        m_autoindent = activate;
    };
    void setAutoCompletion( bool activate )
    {
        m_autoCompletion = activate;
    };
    void setTabStopWidth(int taille);
    void setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g);
    void setEndLine(MainImpl::EndLine end)
    {
        m_endLine = end;
    };
    void setTabSpaces(bool t)
    {
        m_tabSpaces = t;
    };
    void setAutobrackets(bool b)
    {
        m_autobrackets = b;
    };
    void setMatch(bool b)
    {
        m_match = b;
    };
    void setHighlightCurrentLine(bool b)
    {
        m_highlightCurrentLine = b;
    }
    void setBackgroundColor( QColor c );
    void setCurrentLineColor( QColor c );
    void setMatchingColor( QColor c )
    {
        m_matchingColor = c;
    };
    int currentLineNumber(QTextCursor cursor=QTextCursor());
    int currentLineNumber(QTextBlock block);
    int linesCount();
    void dialogGotoLine();
    void completeCode();
    void setFocus(Qt::FocusReason reason);
    void gotoMatchingBracket();
    void textPlugin(TextEditInterface *iTextEdit);
    void print();
    void printWhiteSpaces( QPainter &p );

    enum ActionComment
    {
        Toggle, Comment, Uncomment
    };
    void insertText(QString text, int insertAfterLine);
    void setMouseHidden( bool hidden );
public slots:
    void gotoLine( int line, bool moveTop );
    void slotFind(Ui::FindWidget ui, QString ttf=0,  QTextDocument::FindFlags options=0, bool fromButton=false);
    void slotIndent(bool indent=true);
    void slotUnindent();
    void comment(ActionComment action);
    void slotGotoImplementation();
    void slotGotoDeclaration();
private slots:
    void slotAdjustSize();
    void slotWordCompletion(QListWidgetItem *item);
    void slotCursorPositionChanged();
    void slotContentsChange ( int position, int charsRemoved, int charsAdded );
    void slotCompletionList(TagList TagList);
    void slotCompletionHelpList(TagList TagList);
    void slotToggleBreakpoint();
    void slotToggleBookmark();
private:
    void completionHelp();
    QString m_plainText;
    long m_matchingBegin;
    long m_matchingEnd;
    QPointer<LineNumbers> m_lineNumbers;
    QPointer<SelectionBorder> m_selectionBorder;
    CppHighlighter *cpphighlighter;
    QString m_findExp;
    int m_findOptions;
    FindImpl *m_findImpl;
    int lineNumber(QTextCursor cursor);
    int lineNumber(QTextBlock b);
    int lineNumber(QPoint point);
    void key_home();
    QPoint mousePosition;
    Editor *m_editor;
    MainImpl *m_mainImpl;
    bool m_autoindent;
    void autoIndent();
    void autobrackets();
    void autoUnindent();
    void match();
    void clearMatch();
    MainImpl::EndLine m_endLine;
    bool m_tabSpaces;
    InitCompletion *m_completion;
    QListWidget *m_completionList;
    bool m_autoCompletion;
    bool m_autobrackets;
    bool m_match;
    bool m_highlightCurrentLine;
    bool m_showWhiteSpaces;
    QColor m_backgroundColor;
    QColor m_currentLineColor;
    QColor m_matchingColor;
    QAction *actionToggleBreakpoint;
    int m_lineNumber;
    bool m_mouseHidden;
	QPixmap	m_tabPixmap;
	QPixmap m_spacePixmap;

protected:
    void resizeEvent( QResizeEvent* );
    void keyPressEvent ( QKeyEvent * event );
    void contextMenuEvent(QContextMenuEvent * e);
    void mouseDoubleClickEvent( QMouseEvent * event );
    void dropEvent( QDropEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void paintEvent ( QPaintEvent * event );
    void mouseMoveEvent( QMouseEvent * event );
signals:
    void editorModified(bool);
};

#endif
