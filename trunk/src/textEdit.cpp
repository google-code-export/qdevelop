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
#include "textEdit.h"
#include "editor.h"
#include "linenumbers.h"
#include "selectionborder.h"
#include "cpphighlighter.h"
#include "ui_gotoline.h"
#include "pluginsinterfaces.h"
//
#include <QTextCursor>
#include <QDialog>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMenu>
#include <QTextDocumentFragment>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QListWidget>
#include <QProcess>
#include <QClipboard>
#include <QFileInfo>
#include <QPrintDialog>
#include <QPrinter>

TextEdit::TextEdit(Editor * parent, MainImpl *mainimpl, InitCompletion *completion)
        : QTextEdit(parent), m_editor(parent), m_mainImpl(mainimpl), m_completion(completion)
{
    setObjectName( "editorZone" );
    m_lineNumbers = 0;
    m_selectionBorder = 0;
    cpphighlighter = 0;
    m_autoindent = true;
    m_autobrackets = true;
    setAcceptRichText( false );
    setLineWrapMode( QTextEdit::NoWrap );
    m_findOptions = 0;
    m_findExp = "";
    m_findImpl = 0;
    m_match = true;
    m_matchingBegin = -1;
    m_matchingEnd = -1;
    m_endLine = MainImpl::Default;
    connect(document(), SIGNAL(modificationChanged(bool)), this, SIGNAL(editorModified(bool)));
    connect( this, SIGNAL( cursorPositionChanged() ), this, SLOT( slotCursorPositionChanged()));
    connect( document(), SIGNAL( contentsChange(int, int, int) ), this, SLOT( slotContentsChange(int, int, int) ));
    actionToggleBreakpoint = new QAction(this);
    actionToggleBreakpoint->setShortcut( Qt::Key_F9 );
    connect(actionToggleBreakpoint, SIGNAL(triggered()), this, SLOT(slotToggleBreakpoint()) );
    //
    m_completionList = new QListWidget(this);
    m_completionList->setSelectionMode( QAbstractItemView::SingleSelection );
    m_completionList->hide();
#ifdef Q_WS_MAC
    m_completionList->setFont(QFont(m_completionList->font().family(), 12) );
#else
    m_completionList->setFont( QFont(m_completionList->font().family(), 8) );
#endif
    connect(m_completionList, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(slotWordCompletion(QListWidgetItem *)) );
    setBackgroundColor( m_backgroundColor );
}
//

void TextEdit::setBackgroundColor( QColor c )
{
    if ( c == m_backgroundColor )
        return;
    m_backgroundColor = c;
    QPalette pal = palette();
    pal.setColor(QPalette::Base, m_backgroundColor);
    setPalette( pal );
    viewport()->update();
}
//
void TextEdit::slotContentsChange ( int position, int charsRemoved, int charsAdded )
{
    //qDebug()<<"slotContentsChange"<<position<<charsRemoved<<charsAdded;

    // TODO remove gcc warnings
    position = 0;
    charsRemoved = 0;
    charsAdded = 0;
}
//
void TextEdit::setCurrentLineColor( QColor c )
{
    if ( c == m_currentLineColor )
        return;
    m_currentLineColor = c;
    viewport()->update();
}
//
void TextEdit::print()
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setFullPage(true);
    QPrintDialog dlg(&printer, this);
    //dlg.addEnabledOption( QAbstractPrintDialog::PrintSelection );
    //if( 1 )
        //dlg.setEnabledOptions( QAbstractPrintDialog::PrintSelection );
    if (dlg.exec() == QDialog::Accepted)
    {
        document()->print(&printer);
    }
}
//
void TextEdit::completeCode()
{
    if ( !m_completion )
        return;
    if ( m_completion->isRunning() )
    {
        m_completion->setEmitResults( false );
        m_completion->wait();
    }
    QString c = m_plainText.left(textCursor().position());
    m_completion->initParse(c, true);
    m_completion->start();
}

void TextEdit::slotCompletionList(TagList TagList)
{
    if ( TagList.count() )
    {
        int w = 0;
        int h = 0;
        m_completionList->clear();
        foreach(Tag tag, TagList)
        {
            w = qMax(w, fontMetrics().width( tag.name+tag.parameters ));
            m_completionList->addItem( tag.name+tag.parameters );
            h += 15;
            QListWidgetItem *item = m_completionList->item(m_completionList->count()-1);
            item->setData(Qt::UserRole, QVariant(tag.name) );

            if ( tag.kind == "function" || tag.kind == "prototype")
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_meth.png"));
            else if ( tag.kind == "member" )
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_var.png"));
            //m_completionList->addItem( tag.name );
            //qDebug() << tag.name << tag.longName << tag.parameters << tag.access << tag.kind;
        }
        w = qMin(w+20, 350);
        int posX = qMax(cursorRect().x(), 80);
        if ( posX+w > width() )
            posX = width()-220;
        if ( cursorRect().y() > viewport()->height()/2 )
        {
            h = qMin( qMin(h+20, cursorRect().y()), 250);
            m_completionList->setGeometry(posX, cursorRect().y()-h, w, h);

        }
        else
        {
            h = qMin( qMin(h+20, viewport()->height()-22-cursorRect().y()), 250);
            m_completionList->setGeometry(posX, cursorRect().y()+fontMetrics().height(), w, h);

        }
        m_completionList->show();
        m_completionList->setCurrentRow( 0 );
        //
        QString s = wordUnderCursor();
        QList<QListWidgetItem *> listeItems = m_completionList->findItems(s, Qt::MatchExactly);
        listeItems = m_completionList->findItems(s, Qt::MatchStartsWith);
        if ( listeItems.count()==1 )
        {
            m_completionList->setCurrentItem( listeItems.first() );
            slotWordCompletion( m_completionList->currentItem() );
        }
        else if ( listeItems.count()>1 )
            m_completionList->setCurrentItem( listeItems.first() );
        //
    }
    else
        m_completionList->hide();
}
//
void TextEdit::setFocus(Qt::FocusReason reason)
{
    m_completionList->hide();
    QTextEdit::setFocus(reason);
}
//
void TextEdit::mousePressEvent ( QMouseEvent * event )
{
    m_completionList->hide();
    QTextEdit::mousePressEvent ( event );
}
//
TextEdit::~TextEdit()
{
    delete lineNumbers();
}
//
void TextEdit::setTabStopWidth(int taille)
{
    bool m = document()->isModified();
    int posScrollbar = verticalScrollBar()->value();
    QTextEdit::setTabStopWidth(fontMetrics().width( " " ) * taille);
    setPlainText( toPlainText() );
    verticalScrollBar()->setValue( posScrollbar );
    document()->setModified( m );
}
//
bool TextEdit::open(bool silentMode, QString filename, QDateTime &lastModified)
{
    if ( cpphighlighter && !QString(":c:cc:cpp:h:").contains( ":"+filename.section(".", -1, -1).toLower()+":" ) )
    {
        delete cpphighlighter;
        cpphighlighter = 0;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if ( !silentMode )
            QMessageBox::critical(0, "QDevelop", tr("The file ")+" \""+filename+"\" "+tr("could not be loaded."),tr("Cancel") );
        return false;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    lastModified = QFileInfo( file ).lastModified();
    file.close();
    if ( m_lineNumbers )
        m_lineNumbers->setDigitNumbers( QString::number(linesCount()).length() );
    if ( m_completion )
    {
        m_completion->initParse(toPlainText(), true, false);
        m_completion->start();
    }
    QApplication::restoreOverrideCursor();
    return true;
}
//
void TextEdit::activateLineNumbers(bool activate)
{
    if ( activate && m_lineNumbers==0 )
        setLineNumbers( new LineNumbers(this, m_editor) );
    else if ( !activate && m_lineNumbers )
        setLineNumbers( 0 );
}
//
void TextEdit::setSelectionBorder(bool activate)
{
    if ( activate && m_selectionBorder==0 )
        setSelectionBorder( new SelectionBorder(this) );
    else if ( !activate && m_selectionBorder )
        setSelectionBorder( (SelectionBorder*)0 );
}
//
void TextEdit::autobrackets()
{
    textCursor().insertText( "\n" );
    autoIndent();
    textCursor().insertText( "\n" );
    textCursor().insertText( "}" );
    autoUnindent();
    setTextCursor( getLineCursor(currentLineNumber()-1) );
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfLine );
    setTextCursor( cursor );
}
//
void TextEdit::autoIndent()
{
    QTextBlock blocAIndenter;
    QTextBlock b = textCursor().block();
    if (  b.previous().isValid() && !b.previous().text().isEmpty() )
        blocAIndenter = b;
    else
        return;
    QTextBlock blocAvant = blocAIndenter.previous();
    if ( !blocAvant.isValid() )
        return;
    QString simple = blocAvant.text().simplified();
    QString blancs;
    for (int i=0; i< blocAvant.text().length(); i++)
    {
        if ( blocAvant.text().at(i) == ' ' || blocAvant.text().at(i) == '\t' )
        {
            QString s = blocAvant.text().at(i);
            if ( m_tabSpaces )
            {
                int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
                QString spaces;
                for (int i = 0; i<nbSpaces; i++)
                    spaces += " " ;
                s.replace("\t", spaces);
            }
            blancs += s;
        }
        else
            break;
    }
    if ( simple.simplified().length() && ((simple.contains("(") && simple.contains(")")
                                           && QString("if:while:do:switch:foreach").contains( simple.section("(", 0, 0).simplified() ) )
                                          || QString("else:case:default").indexOf( simple.simplified() ) == 0
                                          || simple.simplified().at(0) == '{' || simple.simplified().at( simple.simplified().length()-1 ) == '{' ))
    {
        if ( m_tabSpaces )
        {
            int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
            for (int i = 0; i<nbSpaces; i++)
                blancs +=  " " ;
        }
        else
            blancs += "\t";
    }
    textCursor().insertText( blancs );
    return;
}
//
void TextEdit::comment(ActionComment action)
{
    // Trent's implementation
    QTextCursor cursor = textCursor();
    if ( cursor.selectedText().isEmpty() )
        return;
    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
    QTextBlock startBlock = document()->findBlock(startPos);
    QTextBlock endBlock = document()->findBlock(endPos).previous();
    int firstLine = lineNumber( startBlock );
    int lastLine = lineNumber( endBlock );
    QTextBlock block = startBlock;
    cursor.setPosition(startPos);
    while (!(endBlock < block))
    {
        QString text = block.text();
        if (text.isEmpty())
            continue;
        int i = 0;
        while (i < text.length() && text.at(i).isSpace())
            i++;
        if (action == Comment)
        {
            if (text.mid(i, 2) != "//")
                text.insert(i, "//");
        }
        else if (action == Uncomment)
        {
            if (text.mid(i, 2) == "//")
                text.remove(i, 2);
        }
        else if (action == Toggle)
        {
            if (text.mid(i, 2) == "//")
                text.remove(i, 2);
            else
                text.insert(i, "//");
        }
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::NextBlock);
        block = cursor.block();
    }
    // Reselect blocks
    selectLines(firstLine, lastLine);
}
//
void TextEdit::autoUnindent()
{
    QTextBlock b = textCursor().block();
    if (  !b.previous().isValid() || b.previous().text().isEmpty() )
        return;
    QString caractere = b.text().simplified();
    if (  caractere!="{" && caractere!="}" )
        return;
    QTextBlock blocAvant = b.previous();
    if ( !blocAvant.isValid() )
        return;
    QString blancs;
    for (int i=0; i< blocAvant.text().length(); i++)
    {
        if ( blocAvant.text().at(i) == ' ' || blocAvant.text().at(i) == '\t' )
            blancs += blocAvant.text().at(i);
        else
            break;
    }
    if ( blancs.length() && caractere=="}" && blocAvant.text().simplified()!="{")
    {
        if ( m_tabSpaces )
        {
            int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
            for (int i = 0; i<nbSpaces; i++)
                if ( blancs.at(0) == ' ' )
                    blancs.remove(0, 1);
        }
        else
            blancs.remove(0, 1);
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor );
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
    cursor.insertText(blancs+caractere);
    setTextCursor( cursor );
}
//
void TextEdit::setSyntaxHighlight(bool activate )
{
    if ( activate && cpphighlighter==0 )
        cpphighlighter = new CppHighlighter( this->document() );
    else if ( !activate && cpphighlighter )
    {
        delete cpphighlighter;
        cpphighlighter = 0;
    }
}
//
void TextEdit::setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g)
{
    if (!cpphighlighter)
        return;
    cpphighlighter->setPreprocessorFormat( a );
    cpphighlighter->setClassFormat( b );
    cpphighlighter->setSingleLineCommentFormat( c );
    cpphighlighter->setMultiLineCommentFormat( d );
    cpphighlighter->setQuotationFormat( e );
    cpphighlighter->setFunctionFormat( f );
    cpphighlighter->setKeywordFormat( g );
    cpphighlighter->setDocument( document() );
}

bool TextEdit::close(QString filename)
{
    if ( document()->isModified() )
    {
        // Proposer sauvegarde
        int rep = QMessageBox::question(this, "QDevelop",
                                        tr("Save")+" \""+filename+"\"", tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
        if ( rep == 2 )
            return false;
        if ( rep == 0 )
        {
            QDateTime info;
            return save(filename, info);
        }
    }
    return true;
}
//
bool TextEdit::save(QString filename, QDateTime &lastModified)
{
    if ( !document()->isModified() )
        return true;
    QFile file( filename );
    if (!file.open(QIODevice::WriteOnly /*| QIODevice::Text*/))
    {
        QMessageBox::about(0, "QDevelop",tr("Unable to save")+" "+filename);
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //QTextStream out(&file);
    QString s = toPlainText();
    if ( m_endLine != MainImpl::Default )
    {
        s.replace("\r\n", "\n");
        if ( m_endLine == MainImpl::Windows )
            s.replace("\n", "\r\n");
    }
    //out << s;
    //file.write( s.toLatin1() );
    file.write( s.toLocal8Bit() );
    file.close();
    QFile last( filename );
    lastModified = QFileInfo( last ).lastModified();
    QApplication::restoreOverrideCursor();
    document()->setModified( false );
    return true;
}
//
void TextEdit::resizeEvent( QResizeEvent* e )
{
    QTextEdit::resizeEvent( e );
    QRect margeNumerotationGeometry;
    if ( m_lineNumbers )
    {
        margeNumerotationGeometry = QRect( viewport()->geometry().topLeft(), QSize( m_lineNumbers->width(), viewport()->height() ) );
        margeNumerotationGeometry.moveLeft( margeNumerotationGeometry.left() -m_lineNumbers->width() );
        if ( m_selectionBorder )
            margeNumerotationGeometry.moveLeft( margeNumerotationGeometry.left() -m_selectionBorder->width() );
        if ( m_lineNumbers->geometry() != margeNumerotationGeometry )
            m_lineNumbers->setGeometry( margeNumerotationGeometry );
    }
    else
    {
        margeNumerotationGeometry.setTopRight( viewport()->geometry().topLeft() );
    }

    QRect margeSelectionGeometry;
    if ( m_selectionBorder )
    {
        margeSelectionGeometry = QRect(  margeNumerotationGeometry.topRight(), QSize( m_selectionBorder->width(), viewport()->height() ) );
        if ( m_lineNumbers )
            margeSelectionGeometry.moveLeft( m_lineNumbers->width() );
        else
            margeSelectionGeometry.moveLeft( margeSelectionGeometry.left() -m_selectionBorder->width() );

        if ( m_selectionBorder->geometry() != margeSelectionGeometry )
            m_selectionBorder->setGeometry( margeSelectionGeometry );
    }
}

QTextCursor TextEdit::getLineCursor( int line ) const
{
    int count = 1;
    for ( QTextBlock b = document()->begin(); b.isValid(); b = b.next(), count++ )
    {
        if ( count == line )
        {
            return QTextCursor( b );
            break;
        }
    }
    QTextCursor c = textCursor();
    c.movePosition( QTextCursor::End );
    c.movePosition( QTextCursor::StartOfLine );
    return c;
}
//
void TextEdit::dialogGotoLine()
{
    QDialog *dial = new QDialog;
    Ui::GotoLine ui;
    ui.setupUi(dial);
    ui.horizontalSlider->setMaximum( linesCount()-1 );
    ui.horizontalSlider->setPageStep( (linesCount()-1)/10 );
    ui.spinBox->setMaximum( linesCount()-1 );
    ui.spinBox->setValue( currentLineNumber() );
    ui.spinBox->selectAll();
    ui.spinBox->setFocus();
    if ( dial->exec() == QDialog::Accepted )
        gotoLine( ui.spinBox->value(), true );
    delete dial;
}
//
void TextEdit::gotoLine( int line, bool moveTop )
{
    if ( moveTop )
        setTextCursor( getLineCursor( linesCount() ) );
    setTextCursor( getLineCursor( line ) );
    setFocus( Qt::OtherFocusReason );
    //
    ensureCursorVisible();
    if ( moveTop )
    {
        QTextCursor cursor = textCursor();
        if ( cursor.isNull() )
            return;
        QTextCursor c = textCursor();
        bool mouvementReussi;
        do
        {
            mouvementReussi = c.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1 );
            setTextCursor( c );
            ensureCursorVisible();
        }
        while (mouvementReussi && cursorRect(cursor).y() > 25 );
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor, 1 );
        setTextCursor( cursor );
    }
}
//
void TextEdit::setLineNumbers( LineNumbers* g )
{
    if ( g == m_lineNumbers )
        return;
    if ( m_lineNumbers )
        delete m_lineNumbers;
    m_lineNumbers = g;
    connect( m_mainImpl, SIGNAL( resetExecutedLine() ), m_lineNumbers, SLOT( slotResetExecutedLine() ) );
    connect(m_lineNumbers, SIGNAL( digitNumbersChanged() ), this, SLOT( slotAdjustSize() ) );
    int margeGaucheSelection = 0;
    if ( m_selectionBorder )
        margeGaucheSelection = m_selectionBorder->width();
    if ( m_lineNumbers )
    {
        setViewportMargins( m_lineNumbers->width()+margeGaucheSelection, 0, 0, 0 );
        if ( !m_lineNumbers->isVisible() )
            m_lineNumbers->show();
    }
    else
        setViewportMargins( margeGaucheSelection, 0, 0, 0 );
}
//
void TextEdit::slotAdjustSize()
{
    int margeGaucheSelection = 0;
    int margeGaucheMargeNumerotation = 0;
    if ( m_selectionBorder )
        margeGaucheSelection = m_selectionBorder->width();
    if ( m_lineNumbers )
        margeGaucheMargeNumerotation = m_lineNumbers->width();
    setViewportMargins( margeGaucheMargeNumerotation+margeGaucheSelection, 0, 0, 0 );
}
//
LineNumbers* TextEdit::lineNumbers()
{
    return m_lineNumbers;
}
//
void TextEdit::setSelectionBorder( SelectionBorder* m )
{
    if ( m == m_selectionBorder )
        return;
    if ( m_selectionBorder )
        delete m_selectionBorder;
    m_selectionBorder = m;
    int margeGaucheMargeNumerotation = 0;
    if ( m_lineNumbers )
        margeGaucheMargeNumerotation = m_lineNumbers->width();
    if ( m_selectionBorder )
    {
        setViewportMargins( m_selectionBorder->width()+margeGaucheMargeNumerotation, 0, 0, 0 );
        if ( !m_selectionBorder->isVisible() )
            m_selectionBorder->show();
    }
    else
    {
        setViewportMargins( margeGaucheMargeNumerotation, 0, 0, 0);
    }
}
//
SelectionBorder* TextEdit::selectionBorder()
{
    return m_selectionBorder;
}
//
void TextEdit::findText()
{
    m_editor->find();
}
//
void TextEdit::gotoMatchingBracket()
{
    int pos;
    QTextCursor cursor = textCursor();
    if ( cursor.position() == m_matchingBegin )
        pos = m_matchingEnd;
    else
        pos = m_matchingBegin;
    if ( pos != -1 )
    {
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        setTextCursor( cursor );
    }
}
//
void TextEdit::slotFind(Ui::FindWidget ui, QString ttf,QTextDocument::FindFlags options, bool fromButton)
{
    QTextDocument *doc = document();
    QString oldText = ui.editFind->text();
    QTextCursor c = textCursor();
    QPalette p = ui.editFind->palette();
    p.setColor(QPalette::Active, QPalette::Base, Qt::white);

    if (c.hasSelection())
    {
        if ( options & QTextDocument::FindBackward || fromButton)
            c.setPosition(c.position(), QTextCursor::MoveAnchor);
        else
            c.setPosition(c.anchor(), QTextCursor::MoveAnchor);
    }


    QTextCursor newCursor = c;

    if (!ttf.isEmpty())
    {
        newCursor = doc->find(ttf, c, options);
        ui.labelWrapped->hide();

        if (newCursor.isNull())
        {
            QTextCursor ac(doc);
            ac.movePosition(options & QTextDocument::FindBackward
                            ? QTextCursor::End : QTextCursor::Start);
            newCursor = doc->find(ttf, ac, options);
            if (newCursor.isNull())
            {
                p.setColor(QPalette::Active, QPalette::Base, QColor(255, 102, 102));
                newCursor = c;
            }
            else
                ui.labelWrapped->show();
        }
    }

    setTextCursor(newCursor);
    ui.editFind->setPalette(p);
}
//
void TextEdit::paintEvent ( QPaintEvent * event )
{
    QPainter painter( viewport() );
    if ( m_currentLineColor.isValid() )
    {
        QRect r = cursorRect();
        r.setX( 0 );
        r.setWidth( viewport()->width() );
        painter.fillRect( r, QBrush( m_currentLineColor ) );
    }
    painter.end();
    //
    QTextEdit::paintEvent( event );
    //
    if ( m_matchingBegin != -1 )
    {
        painter.begin( viewport() );
        QFont f = font();
        f.setBold( true );
        painter.setFont( f );
        painter.setPen( m_matchingColor );
        QTextCursor cursor = textCursor();
        cursor.setPosition(m_matchingBegin+1, QTextCursor::MoveAnchor);
        QRect r = cursorRect( cursor );
        painter.drawText(r.x()-2, r.y(), r.width(), r.height(), Qt::AlignLeft | Qt::AlignVCenter, m_plainText.at(m_matchingBegin));
        //
        cursor.setPosition(m_matchingEnd+1, QTextCursor::MoveAnchor);
        r = cursorRect( cursor );
        painter.drawText(r.x()-2, r.y(), r.width(), r.height(), Qt::AlignLeft | Qt::AlignVCenter, m_plainText.at(m_matchingEnd));
        painter.end();
    }
    //
}
//
void TextEdit::slotCursorPositionChanged()
{
    if ( m_currentLineColor.isValid() )
        viewport()->update();
    m_plainText = toPlainText();
    if ( m_match )
    {
        clearMatch();
        match();
    }
}
//
void TextEdit::clearMatch()
{
    m_matchingBegin = -1;
    m_matchingEnd = -1;
}
//
void TextEdit::match()
{
    QTextCursor cursor = textCursor();
    int pos = textCursor().position();
    QChar car;
    if ( pos != -1 )
        car = m_plainText.at( pos );
    if ( QString("({").contains( car ) && !m_editor->inQuotations(pos, m_plainText) )
    {
        // First match
        m_matchingBegin = pos;
        // Second match
        QChar match = ')';
        if ( car == '{' )
            match = '}';
        int nb = 0;
        for (int i = pos+1;  i < m_plainText.length(); i++)
        {
            if ( m_plainText.at(i) ==  car && !m_editor->inQuotations(i, m_plainText) )
            {
                nb++;
            }
            else if ( m_plainText.at(i) == match && !m_editor->inQuotations(i, m_plainText) )
            {
                if ( nb == 0 )
                {
                    m_matchingEnd = i;
                    break;
                }
                else
                {
                    nb--;
                }
            }
        }
    }
    else if ( QString(")}").contains( car ) && !m_editor->inQuotations(pos, m_plainText) )
    {
        // First match
        m_matchingEnd = pos;
        // Second match
        QChar match = '(';
        if ( car == '}' )
            match = '{';
        int nb = 0;
        for (int i = pos-1;  i > 0; i--)
        {
            if ( m_plainText.at(i) ==  car && !m_editor->inQuotations(i, m_plainText)  )
            {
                nb++;
            }
            else if ( m_plainText.at(i) == match && !m_editor->inQuotations(i, m_plainText) )
            {
                if ( nb == 0 )
                {
                    m_matchingBegin = i;
                    break;
                }
                else
                {
                    nb--;
                }
            }
        }

    }
}
//
void TextEdit::slotWordCompletion(QListWidgetItem *item)
{
    m_completionList->hide();
    QString text = item->data(Qt::UserRole).toString();
    //QString texte = item->text();
    wordUnderCursor(QPoint(), true);
    textCursor().insertText( text );
    ensureCursorVisible();
    setFocus( Qt::OtherFocusReason );
    return;
}
//
void TextEdit::keyPressEvent ( QKeyEvent * event )
{
    QTextCursor cursor = textCursor();
    clearMatch();
    if ( event->key() == Qt::Key_Tab )
    {
        slotIndent( !(event->modifiers() == Qt::ControlModifier) );
    }
    else if ( m_completionList->isVisible() )
    {
        if (event->key() == Qt::Key_Backspace && (m_plainText.left(textCursor().position()).right(1) == "."
                || m_plainText.left(textCursor().position()).right(1) == ">"
                || m_plainText.left(textCursor().position()).right(1) == ":"))
            m_completionList->hide();
        else if ( event->key() == Qt::Key_Up )
        {
            int row = m_completionList->currentRow();
            if ( row > 0 )
                m_completionList->setCurrentRow( row-1 );
        }
        else if ( event->key() == Qt::Key_Down )
        {
            int row = m_completionList->currentRow();
            if ( row+1 < m_completionList->count() )
                m_completionList->setCurrentRow( row+1 );
        }
        else if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
            slotWordCompletion( m_completionList->currentItem() );
        else if ( QChar(event->key()).category() == QChar::Other_Control && event->key() != Qt::Key_Backspace )
            m_completionList->hide();
        else
        {
            QTextEdit::keyPressEvent ( event );
            QString s = wordUnderCursor();
            QList<QListWidgetItem *> listeItems = m_completionList->findItems(s, Qt::MatchExactly);
            if ( listeItems.count()==1 )
            {
                m_completionList->hide();
            }
            else
            {
                listeItems = m_completionList->findItems(s, Qt::MatchStartsWith);
                if ( listeItems.count() )
                    m_completionList->setCurrentItem( listeItems.first() );
            }
        }
    }
    else if ( m_autoCompletion &&
              (
                  event->key() == '.'
                  || ( event->key() == '>' && m_plainText.left(textCursor().position()).right(1) == "-"  )
                  || ( event->key() == ':' && m_plainText.left(textCursor().position()).right(1) == ":"  ) )
            )
    {
        QTextEdit::keyPressEvent ( event );
        completeCode();
    }
    else if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
    {
        if ( m_autoindent )
        {
            QTextEdit::keyPressEvent ( event );
            autoIndent();
        }
        else
            QTextEdit::keyPressEvent ( event );
        //m_editor->updateNumLines(currentLineNumber(), 1);
    }
    else if ( event->key() == Qt::Key_Home && !event->modifiers() )
    {
        key_home();
    }
    else if ( m_autoindent && event->key() == '{' || event->key() == '}' )
    {
        QTextEdit::keyPressEvent ( event );
        autoUnindent();
        if ( m_autobrackets && event->key() == '{' )
            autobrackets();
    }
    else if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
    {
        QTextEdit::keyPressEvent ( event );
    }
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutCut() )
        cut();
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutPaste() )
        paste();
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutUndo() )
        document()->undo();
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutRedo() )
        document()->redo();
    else
        QTextEdit::keyPressEvent ( event );
    event->accept();
}
//
void TextEdit::key_home()
{
    QTextCursor cursor = textCursor();
    int col = cursor.columnNumber();
    if ( col > 0 )
    {
        cursor.movePosition(QTextCursor::StartOfLine);
    }
    else
    {
        cursor.movePosition(QTextCursor::StartOfLine);
        QTextBlock b = textCursor().block();
        int i = 0;
        while ( b.text().at(i) == ' ' || b.text().at(i) == '\t' )
        {
            cursor.movePosition(QTextCursor::NextCharacter);
            i++;
        }
    }
    setTextCursor( cursor );
}

void TextEdit::textPlugin(TextEditInterface *iTextEdit)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextCursor cursor = textCursor();
    QString s = iTextEdit->text(m_plainText, cursor.selection().toPlainText());
    if (s.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        return;
    }
    if ( iTextEdit->action() == TextEditInterface::ReplaceAll )
    {
        int posScrollbar = verticalScrollBar()->value();
        int line = currentLineNumber();
        setPlainText( s );
        document()->setModified( true );
        gotoLine(line, false);
        verticalScrollBar()->setValue( posScrollbar );
    }
    else
    {
        cursor.insertText( s );
        setTextCursor( cursor );
    }
    QApplication::restoreOverrideCursor();
}
//
void TextEdit::dropEvent( QDropEvent * event )
{
    int posScrollbar = verticalScrollBar()->value();
    QString text;
    if ( event->mimeData()->hasText() )
    {
        QTextCursor save = textCursor();
        setTextCursor( cursorForPosition( QPoint( event->pos().x(),  event->pos().y() ) ) );
        text = event->mimeData()->text();
        int linesAdded = 0;
        if ( text.length() )
            linesAdded = text.count( QChar('\n') );
        setTextCursor( save );
    }
    QTextEdit::dropEvent( event );
    if ( event->keyboardModifiers() == Qt::NoModifier )
    {
        QTextCursor cursor = textCursor();
        int pos = textCursor().position();
        cursor.setPosition(pos-text.length(), QTextCursor::MoveAnchor);
        cursor.setPosition(pos, QTextCursor::KeepAnchor);
        setTextCursor( cursor );
    }
    verticalScrollBar()->setValue( posScrollbar );
}
//
void TextEdit::slotIndent(bool indenter)
{
    QTextCursor curseurActuel = textCursor();
    QTextCursor c = textCursor();
    if ( indenter && c.selectedText().isEmpty() )
    {
        if ( m_tabSpaces )
        {
            int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
            for (int i = 0; i<nbSpaces; i++)
                c.insertText( " " );
        }
        else
        {
            if ( m_tabSpaces )
            {
                int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
                for (int i = 0; i<nbSpaces; i++)
                    c.insertText( " " );
            }
            else
                c.insertText( "\t" );
        }
        return;
    }
    else if ( !indenter && c.selectedText().isEmpty() )
    {}
    int debut = c.selectionStart();
    int fin = c.selectionEnd();
    c.clearSelection();
    //
    c.setPosition(debut, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    QTextBlock blocDebut = c.block();
    //
    c.setPosition(fin, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    QTextBlock blocFin = c.block();
    //
    if ( blocDebut == blocFin )
    {
        if ( m_tabSpaces )
        {
            int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
            for (int i = 0; i<nbSpaces; i++)
                curseurActuel.insertText( " " );
        }
        else
            curseurActuel.insertText( "\t" );
        setTextCursor( curseurActuel );
        return;
    }
    //
    int ligneDebut = 1;
    for ( QTextBlock block = document()->begin(); block.isValid() && block!=blocDebut; block = block.next(), ligneDebut++)
        ;
    //
    int ligneFin = ligneDebut-1;
    QTextBlock block = blocDebut;
    do
    {
        c.setPosition(block.position(), QTextCursor::MoveAnchor);
        if ( !indenter )
        {
            if ( block.text().count() && (block.text().at(0) == '\t' || block.text().at(0) == ' ') )
                c.deleteChar();
        }
        else
        {
            if ( m_tabSpaces )
            {
                int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
                for (int i = 0; i<nbSpaces; i++)
                    c.insertText( " " );
            }
            else
            {
                if ( m_tabSpaces )
                {
                    int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
                    for (int i = 0; i<nbSpaces; i++)
                        c.insertText( " " );
                }
                else
                    c.insertText( "\t" );
            }
        }
        setTextCursor( c );
        ligneFin++;
        block = block.next();
    }
    while (  block.isValid() && block != blocFin );
    selectLines(ligneDebut, ligneFin);
}
//
void TextEdit::slotUnindent()
{
    slotIndent(false);
}
//
void TextEdit::mouseDoubleClickEvent( QMouseEvent * event )
{
    mousePosition = event->pos();
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    while ( pos>0  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( m_plainText.at( pos-1 ).toUpper()  ) )
        pos--;
    cursor.setPosition(pos, QTextCursor::MoveAnchor);
    setTextCursor( cursor );
    //
    while ( pos < m_plainText.length()  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( m_plainText.at( pos ).toUpper()  ) )
        pos++;
    cursor.setPosition(pos, QTextCursor::KeepAnchor);
    setTextCursor( cursor );
}
//
void TextEdit::setExecutedLine(int line)
{
    if ( m_lineNumbers)
    {
        m_lineNumbers->setExecutedLine(line);
    }
}
//
void TextEdit::contextMenuEvent(QContextMenuEvent * e)
{
    mousePosition = e->pos();
    m_lineNumber = lineNumber( e->pos() );
    QMenu *menu = createStandardContextMenu();
    menu->clear();
    connect(menu->addAction(QIcon(":/toolbar/images/undo.png"), tr("Undo")), SIGNAL(triggered()), this, SLOT(undo()) );
    connect(menu->addAction(QIcon(":/toolbar/images/redo.png"), tr("Redo")), SIGNAL(triggered()), this, SLOT(redo()) );
    menu->addSeparator();
    connect(menu->addAction(QIcon(":/toolbar/images/editcut.png"), tr("Cut")), SIGNAL(triggered()), this, SLOT(cut()) );
    connect(menu->addAction(QIcon(":/toolbar/images/editcopy.png"), tr("Copy")), SIGNAL(triggered()), this, SLOT(copy()) );
    connect(menu->addAction(QIcon(":/toolbar/images/editpaste.png"), tr("Paste")), SIGNAL(triggered()), this, SLOT(paste()) );
    menu->addSeparator();
    connect(menu->addAction(QIcon(":/toolbar/images/indente.png"), tr("Selection Indent")), SIGNAL(triggered()), this, SLOT(slotIndent()) );
    connect(menu->addAction(QIcon(":/toolbar/images/desindente.png"), tr("Selection Unindent")), SIGNAL(triggered()), this, SLOT(slotUnindent()) );
    QTextCursor cursor = textCursor();
    menu->addSeparator();
    connect(menu->addAction(tr("Select All")), SIGNAL(triggered()), this, SLOT(selectAll()) );
    menu->addSeparator();
    connect(menu->addAction(QIcon(":/toolbar/images/find.png"), tr("Find...")), SIGNAL(triggered()), m_editor, SLOT(find()) );
    menu->addSeparator();
    connect(menu->addAction(QIcon(":/divers/images/bookmark.png"), tr("Toggle Bookmark")), SIGNAL(triggered()), this, SLOT(slotToggleBookmark()) );
    connect(menu->addAction(QIcon(":/divers/images/pointArret.png"), tr("Toggle Breakpoint")), SIGNAL(triggered()), this, SLOT(slotToggleBreakpoint()) );
    //
    menu->exec(e->globalPos());
    delete menu;
}
//
void TextEdit::selectLines(int debut, int fin)
{
    if ( debut > fin )
        qSwap( debut, fin);
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::Start );
    c.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, debut-1 );
    c.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, fin-debut+1 );
    setTextCursor( c );
    ensureCursorVisible();
}
//
//
QString TextEdit::wordUnderCursor(const QPoint & pos, bool select)
{
    QTextCursor cursor;
    if ( pos.isNull() )
        cursor = textCursor();
    else
        cursor = cursorForPosition ( pos );
    QTextCursor save(cursor);
    //
    int curpos = cursor.position();
    while ( curpos>0  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( m_plainText.at( curpos-1 ).toUpper()  ) )
        curpos--;
    cursor.setPosition(curpos, QTextCursor::MoveAnchor);
    setTextCursor( cursor );
    //
    while ( curpos < m_plainText.length()  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( m_plainText.at( curpos ).toUpper()  ) )
        curpos++;
    cursor.setPosition(curpos, QTextCursor::KeepAnchor);
    QString word = cursor.selectedText().simplified();
    //
    if ( select )
        setTextCursor( cursor );
    else
        setTextCursor( save );
    return word;
}
//
QString TextEdit::classNameUnderCursor()
{
    QString c = m_plainText.left(textCursor().position());
    return m_completion->className(c);
}
//
QString TextEdit::methodeMotSousCurseur()
{
    QString contenu, retour="";
    int posDebut = 0;
    QTextCursor sauveCurseur = textCursor();
    int posScrollBar = verticalScrollBar()->value();
    int positionMot = textCursor().block().position();
    //
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::Start);
    setTextCursor( c );
    //
    foreach(QString nomClasse, m_editor->classes() )
    {
        foreach(QString nomMethode, m_editor->methodes( nomClasse ) )
        {
            gotoLine( 1, false );
            bool trouve = find (nomMethode);
            if ( trouve )
            {
                contenu = "";
                posDebut = textCursor().block().position();
                int nbAccolades = -1;
                QTextBlock block;
                for ( block = textCursor().block(); block.isValid() && nbAccolades; block = block.next() )
                {
                    contenu += block.text() + "\n";
                    if ( block.text().contains("{") )
                        if ( nbAccolades == -1 )
                            nbAccolades = 1;
                        else
                            nbAccolades++;
                    if ( block.text().contains("}") )
                        nbAccolades--;
                }
                if ( posDebut <= positionMot && positionMot <= block.position() )
                {
                    retour = contenu;
                    break;
                }
            }
        }
    }
    //
    setTextCursor( sauveCurseur );
    verticalScrollBar()->setValue(posScrollBar);
    return retour;
}
//
int TextEdit::currentLineNumber()
{
    return lineNumber( textCursor() );
}
//
int TextEdit::currentLineNumber(QTextBlock block)
{
    int lineNumber = 1;
    for ( QTextBlock b =document()->begin(); b.isValid(); b = b.next(), lineNumber++)
    {
        if ( b == block )
        {
            return lineNumber;
        }
    }
    return -1;
}
//
int TextEdit::lineNumber(QTextCursor cursor)
{
    QTextBlock blocCurseur = cursor.block();
    int m_lineNumber = 1;
    for ( QTextBlock block =document()->begin(); block.isValid() && block != blocCurseur; block = block.next() )
        m_lineNumber++;
    return m_lineNumber++;
}
//
int TextEdit::lineNumber(QTextBlock b)
{
    int m_lineNumber = 1;
    for ( QTextBlock block =document()->begin(); block.isValid() && block != b; block = block.next() )
        m_lineNumber++;
    return m_lineNumber++;
}
//
int TextEdit::linesCount()
{
    int line = 1;
    for ( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
        line++;
    return line;
}
//
int TextEdit::lineNumber(QPoint point)
{
    return lineNumber( cursorForPosition( point ) );
}
//
//
void TextEdit::slotToggleBookmark()
{
    m_editor->toggleBookmark( m_lineNumber );
    m_lineNumbers->update();
}
//
//
void TextEdit::slotToggleBreakpoint()
{
    m_editor->toggleBreakpoint( m_lineNumber );
    m_lineNumbers->update();
}
