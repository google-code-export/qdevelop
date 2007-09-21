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
#include "textEdit.h"
#include "editor.h"
#include "linenumbers.h"
#include "selectionborder.h"
#include "cpphighlighter.h"
#include "ui_gotoline.h"
#include "pluginsinterfaces.h"
#include "treeclasses.h"
#include "InitCompletion.h"
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
#include <QTime>
#include <QPrinter>
#include <QTextLayout>
#include <QTextCodec>

#define QD qDebug() << __FILE__ << __LINE__ << ":"

extern QString simplifiedText( QString );
//
static const char * tabPixmap_img[] = 
{
/* width height ncolors cpp [x_hot y_hot] */
	"8 8 3 2 0 0",
/* colors */
	"  s none       m none  c none",
	"O s iconColor1 m black c black",
	"X s iconColor2 m black c #D0D0D0",
/* pixels */
	"  X     X       ",
	"    X     X     ",
	"      X     X   ",
	"        X     X ",
	"      X     X   ",
	"    X     X     ",
	"  X     X       ",
	"                ",
};

static const char * spacePixmap_img[] = 
{
/* width height ncolors cpp [x_hot y_hot] */
	"8 8 3 2 0 0",
/* colors */
	"  s none       m none  c none",
	"O s iconColor1 m black c black",
	"X s iconColor2 m black c #D0D0D0",
/* pixels */
	"                ",
	"                ",
 	"                ",
	"                ",
	"                ",
	"      X         ",
	"      X X       ",
	"                ",
};

TextEdit::TextEdit(Editor * parent, MainImpl *mainimpl, InitCompletion *completion)
        : QTextEdit(parent), m_editor(parent), m_mainImpl(mainimpl), m_completion(completion), m_mouseHidden(false)
{
    setObjectName( "editorZone" );
    m_lineNumbers = 0;
    m_selectionBorder = 0;
    cpphighlighter = 0;
    m_autoindent = true;
    m_autobrackets = true;
    setAcceptRichText( false );
    setLineWrapMode( QTextEdit::FixedPixelWidth );
    setLineWrapColumnOrWidth( 65535 );
    m_findOptions = 0;
    m_findExp = "";
    m_findImpl = 0;
    m_match = true;
    m_highlightCurrentLine = true;
    m_matchingBegin = -1;
    m_matchingEnd = -1;
    m_endLine = MainImpl::Default;
    m_tabPixmap		= QPixmap( tabPixmap_img ); 
	m_spacePixmap		= QPixmap( spacePixmap_img ); 
	m_showWhiteSpaces	= true;

    
    connect(document(), SIGNAL(modificationChanged(bool)), this, SIGNAL(editorModified(bool)));
    connect( this, SIGNAL( cursorPositionChanged() ), this, SLOT( slotCursorPositionChanged()));
    connect( document(), SIGNAL( contentsChange(int, int, int) ), this, SLOT( slotContentsChange(int, int, int) ));
    connect(this, SIGNAL(initParse(QString, QString, bool, bool, bool, QString, bool)), m_completion, SLOT(slotInitParse(QString, QString, bool, bool, bool, QString, bool)) );
    actionToggleBreakpoint = new QAction(this);
    actionToggleBreakpoint->setShortcut( Qt::Key_F9 );
    connect(actionToggleBreakpoint, SIGNAL(triggered()), this, SLOT(slotToggleBreakpoint()) );
    //
    m_completionList = new QListWidget(this);
    m_completionList->setSelectionMode( QAbstractItemView::SingleSelection );
    m_completionList->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    m_completionList->hide();
    m_completionList->setSortingEnabled( true );
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
    if (dlg.exec() == QDialog::Accepted)
    {
        document()->print(&printer);
    }
}

void TextEdit::printWhiteSpacesAndMatching( QPainter &p )
{		
	const int contentsY = verticalScrollBar()->value();
	const qreal pageBottom = contentsY + viewport()->height();
	const QFontMetrics fm = QFontMetrics( currentFont() );
	
	for ( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
	{
		QTextLayout* layout = block.layout();
		const QRectF boundingRect = layout->boundingRect();
		QPointF position = layout->position();
		
		if ( position.y() +boundingRect.height() < contentsY )
			continue;
		if ( position.y() > pageBottom )
			break;
		
		const QString txt = block.text();
		const int len = txt.length();
		
		for ( int i=0; i<len; i++)
		{
			QTextCursor cursor = textCursor();
			cursor.setPosition( block.position() + i, QTextCursor::MoveAnchor);
			
			QRect r = cursorRect( cursor );
			if( block.position() + i == m_matchingBegin || block.position() + i == m_matchingEnd )
			{
				QTextCursor cursor = textCursor();
				cursor.setPosition( block.position() + i, QTextCursor::MoveAnchor);
				QRect r2 = cursorRect( cursor );
				if( QString("({[").contains( m_plainText.at( block.position() + i ) ) )
					r2.adjust(r2.width()/3, r2.height()-1, r2.width()/3, 0);
				else
					r2.adjust(r2.width()/4, r2.height()-1, r2.width()/4, 0);
				r2.adjust(2, 0, -2, 0);
				p.setPen( m_matchingColor );
				//int x = r2.x() + 4;
				//int y = r2.y();// + fm.height();// / 2 - 5;
				p.drawRect(r2);
			}
			
			if( m_showWhiteSpaces )
			{
				// pixmaps are of size 8x8 pixels
				QPixmap *p1 = 0;
				
				if (txt[i] == ' ' )
					p1 = &m_spacePixmap;
				else if (txt[i] == '\t' )
					p1 = &m_tabPixmap;
				else 
					continue;
				
				int x = r.x() + 4;
				int y = r.y() + fm.height() / 2 - 5;
				p.drawPixmap( x, y, *p1 );
					
			}
		}
	}
}

//
void TextEdit::completeCode()
{
    if ( m_mainImpl->buildQtDatabase() )
    {
    	QMessageBox::warning(m_mainImpl, "QDevelop", tr("The Qt database building is in progress.\nTry to complete code later."));
        return;
   	}
    if ( !m_completion )
        return;
    QString c = m_plainText.left(textCursor().position());
    if( c.simplified().right(1) == "(" )
    {
    	completionHelp();
    	return;
   	}
	bool addThis = true;
	QString word;
	int i;
	for(i = c.length()-1; i>0; i--)
	{
		if( c.at(i) == QChar('\n') || c.at(i) == QChar(';') )
		{
			i++;
			word = c.mid( i );
			c = c.left( i );
			break;
		}
		if( QString(":.>(").contains( c.at(i) ) )
		{
			addThis = false;
			break;
		}
	}
    if( addThis )
    {
        c += "this->" + word;
    }
    emit initParse(m_editor->filename(), c, true, true, false, QString(), false);
}

void TextEdit::slotCompletionList(TagList tagList )
{
    if ( tagList.count() )
    {
        int w = 0;
        int h = 0;
        m_completionList->clear();
        foreach(Tag tag, tagList)
        {
            w = qMax(w, fontMetrics().width( tag.name+tag.parameters ));
            QListWidgetItem *item = new QListWidgetItem( m_completionList );
            item->setText(tag.name+tag.parameters );
            h += 15;
            QVariant v;
            v.setValue( tag );
            item->setData(Qt::UserRole, v );
            //item->setData(Qt::UserRole, QVariant(tag.name) );
			if( tag.access.isEmpty() )
				tag.access = "public";
            if ( tag.kind == "function" || tag.kind == "prototype")
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_meth.png"));
            else if ( tag.kind == "member" )
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_var.png"));
            else if ( tag.kind == "struct" )
                item->setIcon(QIcon(":/CV/images/CVstruct.png"));
            else if ( tag.kind == "class" )
                item->setIcon(QIcon(":/CV/images/CVclass.png"));
            m_completionList->addItem(item);
            //m_completionList->addItem( tag.name );
        }
        m_completionList->setSelectionMode( QAbstractItemView::SingleSelection );
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        m_completionList->setPalette(palette);
        w = qMin(w+20, 550);
        w = qMax(w, 150);
        int posX = qMax(cursorRect().x(), 80);
        //if ( posX+w > width() )
            //posX = width()-220;
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
        if ( listeItems.count()>1 )
            m_completionList->setCurrentItem( listeItems.first() );
        //
    }
    else
        m_completionList->hide();
}
//
void TextEdit::slotCompletionHelpList(TagList tagList)
{
    if ( tagList.count() )
    {
        int w = 0;
        int h = 0;
        m_completionList->clear();
        foreach(Tag tag, tagList)
        {
            w = qMax(w, fontMetrics().width( tag.name+tag.parameters ));
            QListWidgetItem *item = new QListWidgetItem( m_completionList );
            item->setText( tag.name+tag.parameters );
            h += 15;
			if( tag.access.isEmpty() )
				tag.access = "public";
            if ( tag.kind == "function" || tag.kind == "prototype")
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_meth.png"));
            else if ( tag.kind == "member" )
                item->setIcon(QIcon(":/CV/images/CV"+tag.access+"_var.png"));
            m_completionList->addItem( item );
        }
        m_completionList->setSelectionMode( QAbstractItemView::NoSelection );
        QPalette palette;
        QBrush brush(QColor(255, 255, 127, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        m_completionList->setPalette(palette);
        w = qMin(w+20, 550);
        w = qMax(w, 150);
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
    if ( cpphighlighter && !QString(":c:cc:cpp:h:hpp").contains( ":"+filename.section(".", -1, -1).toLower()+":" ) )
    {
        delete cpphighlighter;
        cpphighlighter = 0;
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if ( !silentMode )
            QMessageBox::critical(0, "QDevelop", tr("The file \"%1\" could not be loaded.").arg(filename),tr("Cancel") );
        return false;
    }
    QByteArray data = file.readAll();
    QTextStream in(&data);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int mib = m_mainImpl->mibCodec();
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    in.setAutoDetectUnicode(false);
    in.setCodec(codec);
    QString decodedStr = in.readAll();
    setPlainText(decodedStr);
    lastModified = QFileInfo( file ).lastModified();
    file.close();
    if ( m_lineNumbers )
        m_lineNumbers->setDigitNumbers( QString::number(linesCount()).length() );
    if ( m_completion  && !m_mainImpl->buildQtDatabase() )
    {
        emit initParse(m_editor->filename(), toPlainText(), true, false, false, QString(), false);
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
	
	//when there is no selection startPos and endPos are equal to position()
    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
	QTextBlock startBlock = document()->findBlock(startPos);
    QTextBlock endBlock = document()->findBlock(endPos);
    
	//special case : the end of the selection is at the beginning of a line
	if ( startPos != endPos && cursor.atBlockStart()) {
		endBlock = document()->findBlock(endPos).previous();
	}
	
	int firstLine = lineNumber( startBlock );
    int lastLine = lineNumber( endBlock );
    QTextBlock block = startBlock;
    cursor.beginEditBlock();
    cursor.setPosition(startPos);
	while (!(endBlock < block))
    {
        QString text = block.text();
        if (!text.isEmpty()) {
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
		}
        cursor.movePosition(QTextCursor::NextBlock);
        block = cursor.block();
    }
	cursor.endEditBlock();
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
                                        tr("Save \"%1\"").arg(filename), tr("Yes"), tr("No"), tr("Cancel"), 0, 2 );
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
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::about(0, "QDevelop",tr("Unable to save %1").arg(filename));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString s = toPlainText();
    if ( m_endLine != MainImpl::Default )
    {
        s.replace("\r\n", "\n");
        if ( m_endLine == MainImpl::Windows )
            s.replace("\n", "\r\n");
    }
    int mib = m_mainImpl->mibCodec();
    QTextCodec *codec = QTextCodec::codecForMib(mib);
    QTextStream out(&file);
    out.setCodec(codec);
    out << s;
    file.close();
    QFile last( filename );
    lastModified = QFileInfo( last ).lastModified();
    QApplication::restoreOverrideCursor();
    document()->setModified( false );
    return true;
}
//
void TextEdit::setMouseHidden( bool hidden )
{
	if ( hidden == m_mouseHidden )
		return;
	viewport()->setCursor( hidden ? Qt::BlankCursor : Qt::IBeamCursor );
	setMouseTracking( hidden );
	m_mouseHidden = hidden;
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
    if ( m_highlightCurrentLine && m_currentLineColor.isValid() )
    {
        QRect r = cursorRect();
        r.setX( 0 );
        r.setWidth( viewport()->width() );
        painter.fillRect( r, QBrush( m_currentLineColor ) );
    }

	if (m_showWhiteSpaces || m_matchingBegin != -1)
		printWhiteSpacesAndMatching( painter );
		
    QTextEdit::paintEvent( event );
}
//
void TextEdit::mouseMoveEvent( QMouseEvent * event )
{
    setMouseHidden( false );
    event->setAccepted( false );
    QTextEdit::mouseMoveEvent( event );
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
	QString matchText =  simplifiedText( m_plainText );
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
   	m_matchingBegin = -1;
   	m_matchingEnd = -1;
    if( pos==-1 || !QString("({[]})").contains( m_plainText.at( pos ) ) )
    	return;
    QChar car;
    if ( pos != -1 )
    {
        if ( !cursor.atEnd() )
        {
            car = matchText.at( pos );
        }
        else
        {
            car = matchText.at( pos - 1);
        }
    }
    QChar matchCar;
    long inc = 1;
    if( car == '(' )
    	matchCar = ')';
    else if( car == '{' )
    	matchCar = '}';
    else if( car == '[' )
    	matchCar = ']';
    else if( car == ')' )
    {
    	matchCar = '(';
    	inc = -1;
   	}
    else if( car == '}' )
    {
    	matchCar = '{';
    	inc = -1;
   	}
    else if( car == ']' )
    {
    	matchCar = '[';
    	inc = -1;
   	}
    else
    {
    	return;
   	}
    m_matchingBegin = pos;
    int nb = 0;
    do
    {
    	if( matchText.at( pos ) == car )
    		nb++;
    	else if( matchText.at( pos ) == matchCar )
    	{
    		nb--;
    		if( nb == 0 )
    		{
    			m_matchingEnd = pos;
    			break;
   			}
   		}
   		pos += inc;
   	}
    while( pos >= 0 && pos < matchText.length() );
    if( m_matchingBegin > m_matchingEnd )
    	qSwap(m_matchingBegin, m_matchingEnd );
    return;
}
//
void TextEdit::slotWordCompletion(QListWidgetItem *item)
{
    m_completionList->hide();
    if ( m_completionList->selectionMode() == QAbstractItemView::NoSelection )
    {
        ensureCursorVisible();
        setFocus( Qt::OtherFocusReason );
        return;
    }
    QString signature = item->text();
    Tag tag = item->data(Qt::UserRole).value<Tag>();
    QString text = tag.name;
    wordUnderCursor(QPoint(), true);
    textCursor().insertText( text );
    if ( m_completion && tag.signature.contains("(") && !tag.signature.contains("()") )
    {
        completionHelp();
    }
    if ( tag.signature.contains("(") && m_plainText.at( textCursor().position() ) != '(' )
    {
        textCursor().insertText( "()" );
        if ( !tag.signature.contains("()") )
        {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::PreviousCharacter);
            setTextCursor( cursor );
        }
    }
    ensureCursorVisible();
    setFocus( Qt::OtherFocusReason );
    return;
}
//
void TextEdit::keyPressEvent ( QKeyEvent * event )
{
    QTextCursor cursor = textCursor();
    clearMatch();
    setMouseHidden( true );
    if ( event->key() == Qt::Key_Tab )
    {
        slotIndent( !(event->modifiers() == Qt::ControlModifier) );
    }
    else if ( m_completionList->isVisible() )
    {
        if (event->key() == Qt::Key_Backspace && (m_plainText.left(textCursor().position()).right(1) == "."
                || m_plainText.left(textCursor().position()).right(1) == ">"
                || m_plainText.left(textCursor().position()).right(1) == ":"))
        {
            m_completionList->hide();
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
        {
            m_completionList->hide();
            QTextEdit::keyPressEvent ( event );
       	}
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
        {
            QTextEdit::keyPressEvent ( event );
        }
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
    else if ( event->key() == '(' && m_completion )
    {
        QTextEdit::keyPressEvent ( event );
        completionHelp();
    }
    else if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
    {
        QTextEdit::keyPressEvent ( event );
    }
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutCut() )
    {
        cut();
    }
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutPaste() )
    {
        paste();
    }
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutUndo() )
    {
        document()->undo();
    }
    else if ( QKeySequence(event->key() | event->modifiers()) == m_mainImpl->shortcutRedo() )
    {
        document()->redo();
    }
    else
    {
        QTextEdit::keyPressEvent ( event );
    }
    event->accept();
}
//
void TextEdit::key_home()
{
    QTextCursor cursor = textCursor();
    QTextBlock b = cursor.block();
    QString s = b.text();
    int col = cursor.columnNumber();
    int firstWordCol = cursor.columnNumber();

    cursor.movePosition(QTextCursor::StartOfLine);
    if ( firstWordCol < s.length())
    {
        while ( s.at(firstWordCol) == ' ' || s.at(firstWordCol) == '\t' )
        {
             cursor.movePosition(QTextCursor::NextCharacter);
             firstWordCol++;
        }
        if ( col > 0 &&   col == firstWordCol )
        {
                cursor.movePosition(QTextCursor::StartOfLine);
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
    setMouseHidden( true );
	//string used to indent text
	QString indentString;
	if ( m_tabSpaces )
	{
		int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
		indentString.fill(' ', nbSpaces);
	} else {
		indentString = QString("\t");
	}

    QTextCursor curseurActuel = textCursor();
    QTextCursor c = textCursor();
    c.beginEditBlock();
    
	if ( !c.hasSelection() )
    {
		if(indenter)
		{
			c.insertText( indentString );
		} else {
			//delete the previous character if it's a space or a tab
			int p = textCursor().position() - 1;
			if( p>0 && p< m_plainText.length() ) {
				QChar s = m_plainText.at( p );
				if ( s == '\t' || s == ' ') {
					c.deletePreviousChar();
				}
			}
		}
		
		c.endEditBlock();
        return;
    }

    int debut = c.selectionStart();
    int fin = c.selectionEnd();
    //
	QTextBlock blocDebut = document()->findBlock(debut);
    QTextBlock blocFin = document()->findBlock(fin);

    //special case
    if ( c.atBlockStart()) {
		blocFin = document()->findBlock(fin).previous();
	}

    c.clearSelection();

    if ( blocDebut == blocFin )
    {
        curseurActuel.insertText( indentString );
        setTextCursor( curseurActuel );
        c.endEditBlock();
        return;
    }
    
    QTextBlock block = blocDebut;
    while (  block.isValid() && !(blocFin < block) )
    {
        c.setPosition(block.position(), QTextCursor::MoveAnchor);
        if ( !indenter )
        {
            if ( block.text().count() && (block.text().at(0) == '\t' || block.text().at(0) == ' ') )
                c.deleteChar();
        }
        else
        {
            c.insertText( indentString );
        }
        setTextCursor( c );
        block = block.next();
    }

    int ligneDebut = lineNumber(blocDebut);
    int ligneFin = lineNumber(blocFin);
    selectLines(ligneDebut, ligneFin);
	c.endEditBlock();   
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
    connect(menu->addAction(QIcon(":/treeview/images/cpp.png"), tr("Goto Implementation")), SIGNAL(triggered()), this, SLOT(slotGotoImplementation()) );
    connect(menu->addAction(QIcon(":/treeview/images/h.png"), tr("Goto Declaration")), SIGNAL(triggered()), this, SLOT(slotGotoDeclaration()) );
//    connect(menu->addAction(QIcon(":/toolbar/images/undo.png"), tr("Undo")), SIGNAL(triggered()), this, SLOT(undo()) );
//    connect(menu->addAction(QIcon(":/toolbar/images/redo.png"), tr("Redo")), SIGNAL(triggered()), this, SLOT(redo()) );
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
    QTextCursor save(textCursor());
    QTextCursor cursor;
    if ( pos.isNull() )
        cursor = textCursor();
    else
        cursor = cursorForPosition ( pos );
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
QString TextEdit::wordUnderCursor(const QString text)
{
    int begin = text.length();
    while ( begin>0  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( text.at( begin-1 ).toUpper()  ) )
        begin--;
    //
    int end = begin;
    while ( end < text.length()  && QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_").contains( text.at( end ).toUpper()  ) )
        end++;
    QString word = text.mid(begin, end-begin);
    return word;
}
//
QString TextEdit::classNameUnderCursor(const QPoint & pos, bool addThis)
{
    QTextCursor cursor;
    if ( pos.isNull() )
        cursor = textCursor();
    else
        cursor = cursorForPosition ( pos );
    QString c = m_plainText.left(cursor.position());
    QString classname = m_completion->className(c);
    if ( classname.isEmpty() && addThis )
    {
        c += " this->";
        classname = m_completion->className(c);
    }
    return classname;
}
//
int TextEdit::currentLineNumber(QTextCursor cursor)
{
	if( cursor.isNull() )
    	return lineNumber( textCursor() );
    else
    	return lineNumber( cursor );
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
void TextEdit::insertText(QString text, int insertAfterLine)
{
    if ( m_tabSpaces )
    {
        int nbSpaces = tabStopWidth() / fontMetrics().width( " " );
        QString spaces;
        for (int i = 0; i<nbSpaces; i++)
            spaces += " " ;
        text.replace("\t", spaces);
    }
    if ( insertAfterLine == -1 )
    {
        QTextCursor c = textCursor();
        c.movePosition( QTextCursor::End );
        c.movePosition( QTextCursor::EndOfLine );
        c.insertText( "\n" + text );
        setTextCursor( c );
    }
    else
    {
        gotoLine(insertAfterLine, false);
        textCursor().insertText( text );
    }
}
//
void TextEdit::slotGotoImplementation()
{
    QString classname;
    // classNameUnderCursor is a long computing. Call only for .cpp files because with .h the result
    // is always ""
    if ( m_editor->filename().toLower().endsWith(".cpp") )
        classname = classNameUnderCursor(mousePosition, false);
    QString name = wordUnderCursor(mousePosition);
    if ( name.isEmpty() )
        return;
    const QList<ParsedItem> *itemsList;
    itemsList = m_mainImpl->treeClassesItems();
    bool found = false;
    for (int i = 0; i < itemsList->size(); ++i)
    {
        ParsedItem parsedItem = itemsList->at( i );
        if ( parsedItem.classname == classname && parsedItem.name == name)
        {
            if ( !parsedItem.implementation.isEmpty() )
            {
                QString s = parsedItem.implementation;
                QString filename = s.section("|", 0, 0);
                int numLine = s.section("|", -1, -1).toInt();
                if ( QFileInfo(filename).isFile() )
                    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
                found = true;
                break;
            }
        }
        /* Below, the item in database has the same filename that the current editor and the same line number.
        The cursor is on a declaration in a header (.h). Open the implementation (.cpp).
        */
        /* If sender() is not null, this function is called from the context menu. Otherwise from the main menu */
        else if (m_editor->filename().toLower().endsWith(".h")
                 && parsedItem.declaration.section("|", 0, 0) == m_editor->filename()
                 && parsedItem.name == name
                 && parsedItem.declaration.section("|", 1, 1).toInt() == currentLineNumber( sender() ? cursorForPosition(mousePosition) : QTextCursor() )
                )
        {
            QString s = parsedItem.implementation;
            QString filename = s.section("|", 0, 0);
            int numLine = s.section("|", -1, -1).toInt();
            if ( QFileInfo(filename).isFile() )
                m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
            found = true;
            break;
        }
    }
    // Now if the text is an .cpp, find the first name in database with the name "name"
    // Perhaps return a bad result but it should work many time.
    if ( !found && m_editor->filename().toLower().endsWith(".cpp"))
    {
        for (int i = 0; i < itemsList->size(); ++i)
        {
            ParsedItem parsedItem = itemsList->at( i );
            if ( parsedItem.name == name )
            {
                QString s = parsedItem.implementation;
                QString filename = s.section("|", 0, 0);
                int numLine = s.section("|", -1, -1).toInt();
                if ( QFileInfo(filename).isFile() )
                    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
                break;

            }
        }
    }
}
//
void TextEdit::slotGotoDeclaration()
{
    QString classname = classNameUnderCursor(mousePosition, false);
    QString name = wordUnderCursor(mousePosition);
    const QList<ParsedItem> *itemsList = m_mainImpl->treeClassesItems();
    bool found = false;
    for (int i = 0; i < itemsList->size(); ++i)
    {
        ParsedItem parsedItem = itemsList->at( i );
        if ( parsedItem.classname == classname && parsedItem.name == name)
        {
            if ( !parsedItem.declaration.isEmpty() )
            {
                QString s = parsedItem.declaration;
                QString filename = s.section("|", 0, 0);
                int numLine = s.section("|", -1, -1).toInt();
                if ( QFileInfo(filename).isFile() )
                    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
                found = true;
                break;
            }
        }
    }
    if ( !found && m_editor->filename().toLower().endsWith(".cpp"))
    {
        for (int i = 0; i < itemsList->size(); ++i)
        {
            ParsedItem parsedItem = itemsList->at( i );
            if ( parsedItem.name == name )
            {
                QString s = parsedItem.declaration;
                QString filename = s.section("|", 0, 0);
                int numLine = s.section("|", -1, -1).toInt();
                if ( QFileInfo(filename).isFile() )
                    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
                break;

            }
        }
    }
}
//

void TextEdit::completionHelp()
{
    if ( !m_completion )
        return;
    QString c = m_plainText.left( textCursor().position() );
    if( c.right(1) == "(" )
    	c = c.left( c.count()-1 );
    QString name = wordUnderCursor(c);
	if( QString(":if:else:for:return:connect:while:do:").contains( name ) )
		return;
    c = c.section(name, 0, 0);
    emit initParse(m_editor->filename(), c, true, true, true, name, false);
}





