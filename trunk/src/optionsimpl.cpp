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
#include "optionsimpl.h"
#include "cpphighlighter.h"
#include <QFontDatabase>
#include <QComboBox>
#include <QPalette>
#include <QColorDialog>
#include <QFileDialog>
#include <QTextCodec>
#include <QLibraryInfo>
#include <QDebug>
//
OptionsImpl::OptionsImpl(QWidget * parent, QFont f, bool num, bool marge, bool ind, 
	bool color, int tab, bool enr, bool res,
	QTextCharFormat pre, QTextCharFormat qt, QTextCharFormat commSimples, 
	QTextCharFormat commMulti, QTextCharFormat guil, QTextCharFormat meth, 
    QTextCharFormat cles, bool autoMask, int end, bool spaces, bool complete, 
    QColor back, bool prompt, bool hcl, QColor lc, bool bk, bool tc, int in, QString directory,
    bool m, QColor mc, bool close, QString pd, QString mo, int mi, QString ic, 
    bool editorToolbars, bool whiteSpaces, QString docDirectory, QColor textCol )
	: QDialog(parent)
{
	setupUi(this); 
	QFontDatabase db;
	comboFont->addItems( db.families() );
	comboFont->setCurrentIndex( comboFont->findText( f.family() ) );
	fontSize->setValue( f.pointSize() );
	numbers->setChecked( num );
	selectionBorder->setChecked( marge );
	indent->setChecked( ind );
	highlight->setChecked( color );
	tabStopWidth->setValue( tab );
	saveAll->setChecked( enr );
	restore->setChecked( res );
	endLine->setCurrentIndex( end );
	tabSpaces->setChecked( spaces );
	completion->setChecked( complete );
	promptBeforeQuit->setChecked( prompt );
	brackets->setChecked( bk );
	showTreeClasses->setChecked( tc );
	interval->setValue( in );
	interval->setEnabled( tc );
	match->setChecked( m );
	groupHighlightCurrentLine->setChecked( hcl );
	closeButton->setChecked( close );
	projectsDirectory->setText( directory );
	pluginsDirectory->setText( pd );
	includeDirectory->setText( ic );
	documentationDirectory->setText( docDirectory );
	makeOptions->setText( mo );
	showEditorToolbars->setChecked( editorToolbars );
	displayWhiteSpaces->setChecked( whiteSpaces );
	
	//
	QPixmap pix(25, 25);
	pix.fill( pre.foreground().color() );
	preprocessor->setIcon( pix );
	pix.fill( qt.foreground().color() );
	qtWords->setIcon( pix );
	pix.fill( commSimples.foreground().color() );
	singleComment->setIcon( pix );
	pix.fill( commMulti.foreground().color() );
	multilinesComment->setIcon( pix );
	pix.fill( guil.foreground().color() );
	quotation->setIcon( pix );
	pix.fill( meth.foreground().color() );
	methods->setIcon( pix );
	pix.fill( cles.foreground().color() );
	keywords->setIcon( pix );
	pix.fill( back );
	background->setIcon( pix );
	m_backgroundColor = back;
	pix.fill( textCol );
	text->setIcon( pix );
	m_textColor = textCol;
	pix.fill( mc );
	matching->setIcon( pix );
	m_matchingColor = mc;
	pix.fill( lc );
	lineColor->setIcon( pix );
	m_colorCurrentLine = lc;
	//
	cppHighLighter = new CppHighlighter( 0 );
	cppHighLighter->setPreprocessorFormat( pre );
	cppHighLighter->setClassFormat( qt );
	cppHighLighter->setSingleLineCommentFormat( commSimples );
	cppHighLighter->setMultiLineCommentFormat( commMulti );
	cppHighLighter->setQuotationFormat( guil );
	cppHighLighter->setFunctionFormat( meth );
	cppHighLighter->setKeywordFormat( cles );
	cppHighLighter->setDocument( textEdit->document() );
	QPalette p = textEdit->palette();
    p.setColor(QPalette::Base, m_backgroundColor);
	p.setColor( QPalette::Text, m_textColor);
	textEdit->setPalette(p);     
	//
	connect(preprocessor, SIGNAL(clicked()), this, SLOT(slotChangeColor())); 
	connect(qtWords, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(singleComment, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(multilinesComment, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(quotation, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(methods, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(keywords, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(background, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(text, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(lineColor, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(matching, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(defaults, SIGNAL(clicked()), this, SLOT(slotDefault()));
	connect(chooseProjectsDirectory, SIGNAL(clicked()), this, SLOT(slotChooseProjectsDirectory()));
	connect(choosePluginsDirectory, SIGNAL(clicked()), this, SLOT(slotChoosePluginsDirectory()));
	connect(chooseIncludeDirectory, SIGNAL(clicked()), this, SLOT(slotChooseIncludeDirectory()));
	connect(chooseDocumentationDirectory, SIGNAL(clicked()), this, SLOT(slotChooseDocumentationDirectory()));
	textEdit->setPlainText( textEdit->toPlainText() );

    findCodecs();
    setCodecList(codecs, mi);

	// TODO remove gcc warnings
	autoMask = false;
}
//
QFont OptionsImpl::font() 
{
	return QFont(comboFont->currentText(), fontSize->value() );
}
//
QColor OptionsImpl::backgroundColor() 
{
	return m_backgroundColor;
}
//
QColor OptionsImpl::textColor() 
{
	return m_textColor;
}
//
QColor OptionsImpl::currentLineColor() 
{
	return m_colorCurrentLine;
}
//
QColor OptionsImpl::matchingColor() 
{
	return m_matchingColor;
}
//
void OptionsImpl::slotChangeColor()
{
	QTextCharFormat format;
	QToolButton *button = (QToolButton*)sender();
	QColor color;
	if( button == preprocessor )
		color = cppHighLighter->preprocessorFormat().foreground().color();
	else if( button == qtWords )
		color = cppHighLighter->classFormat().foreground().color();
	else if( button == singleComment )
		color = cppHighLighter->singleLineCommentFormat().foreground().color();
	else if( button == multilinesComment )
		color = cppHighLighter->multiLineCommentFormat().foreground().color();
	else if( button == quotation )
		color = cppHighLighter->quotationFormat().foreground().color();
	else if( button == methods )
		color = cppHighLighter->functionFormat().foreground().color();
	else if( button == keywords )
		color = cppHighLighter->keywordFormat().foreground().color();
	else if( button == background )
		color = m_backgroundColor;
	else if( button == text )
		color = m_textColor;
	else if( button == lineColor )
		color = m_colorCurrentLine;
	else if( button == matching )
		color = m_matchingColor;
	color = QColorDialog::getColor(color);
	if( color.isValid() )
	{
		QPixmap pix(25, 25);
		pix.fill( color );
		button->setIcon( pix );
		format.setForeground(color);
		if( button == preprocessor )
			cppHighLighter->setPreprocessorFormat( format );
		else if( button == qtWords )
			cppHighLighter->setClassFormat( format );
		else if( button == singleComment )
			cppHighLighter->setSingleLineCommentFormat( format );
		else if( button == multilinesComment )
			cppHighLighter->setMultiLineCommentFormat( format );
		else if( button == quotation )
			cppHighLighter->setQuotationFormat( format );
		else if( button == methods )
			cppHighLighter->setFunctionFormat( format );
		else if( button == keywords )
			cppHighLighter->setKeywordFormat( format );
		else if( button == background )
			m_backgroundColor = color;
		else if( button == text )
			m_textColor = color;
		else if( button == lineColor )
			m_colorCurrentLine = color;
		else if( button == matching )
			m_matchingColor = color;
		cppHighLighter->setDocument( textEdit->document() );
		QPalette p = textEdit->palette();
	    p.setColor(QPalette::Base, m_backgroundColor);
		p.setColor( QPalette::Text, m_textColor);
		textEdit->setPalette(p);     
	}
}
//
void OptionsImpl::slotDefault()
{
	QPixmap pix(25, 25);
	QTextCharFormat format;
	//
	pix.fill( Qt::blue );
	format.setForeground( Qt::blue );
	preprocessor->setIcon( pix );
	cppHighLighter->setPreprocessorFormat( format );
	//
	qtWords->setIcon( pix );
	cppHighLighter->setClassFormat( format );
	//
	pix.fill( Qt::red );
	format.setForeground( Qt::red );
	singleComment->setIcon( pix );
	cppHighLighter->setSingleLineCommentFormat( format );
	//
	multilinesComment->setIcon( pix );
	cppHighLighter->setMultiLineCommentFormat( format );
	//
	pix.fill( Qt::darkGreen );
	format.setForeground( Qt::darkGreen );
	quotation->setIcon( pix );
	cppHighLighter->setQuotationFormat( format );
	//
	pix.fill( Qt::black );
	format.setForeground( Qt::black );
	methods->setIcon( pix );
	cppHighLighter->setFunctionFormat( format );
	//
	pix.fill( Qt::blue );
	format.setForeground( Qt::blue );
	keywords->setIcon( pix );
	cppHighLighter->setKeywordFormat( format );
	//
	cppHighLighter->setDocument( textEdit->document() );
	//
	pix.fill( Qt::white );
	m_backgroundColor = Qt::white;
	background->setIcon( pix );
	//
	pix.fill( Qt::black );
	m_textColor = Qt::black;
	text->setIcon( pix );
	//
	pix.fill( Qt::red );
	m_matchingColor = Qt::red;
	matching->setIcon( pix );
	//
	pix.fill( QColor(215,252,255) );
	m_colorCurrentLine = QColor(215,252,255);
	lineColor->setIcon( pix );
	//
	saveAll->setChecked( true );
	restore->setChecked( true );
	promptBeforeQuit->setChecked( false );
	groupHighlightCurrentLine->setChecked( true );
	//lineColor->setEnabled( true );
	numbers->setChecked( true );
	selectionBorder->setChecked( true );
	completion->setChecked( true );
	indent->setChecked( true );
	brackets->setChecked( true );
	highlight->setChecked( true );
	match->setChecked( true );
	projectsDirectory->setText( QDir::homePath() );
	pluginsDirectory->setText( "" );
	includeDirectory->setText( QLibraryInfo::location( QLibraryInfo::HeadersPath ) );
	documentationDirectory->setText( QLibraryInfo::location( QLibraryInfo::DocumentationPath ) );
	makeOptions->setText( "" );
	tabStopWidth->setValue( 4 );
	tabSpaces->setChecked( false );
#ifdef WIN32
    comboFont->setCurrentIndex( comboFont->findText( "Courier New" ) );
#else
    comboFont->setCurrentIndex( comboFont->findText( "Monospace" ) );
#endif
	fontSize->setValue( 10 );
	closeButton->setChecked( false );
	setCodecList( codecs, 106 );  // UTF-8 by default
	endLine->setCurrentIndex( 0 );
	interval->setValue( 5 );
	showTreeClasses->setChecked( true );
	showEditorToolbars->setChecked( true );
	displayWhiteSpaces->setChecked( true );
}
//
void OptionsImpl::slotChooseProjectsDirectory()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project directory"),
		QDir::homePath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	projectsDirectory->setText( s );
}
//
void OptionsImpl::slotChoosePluginsDirectory()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project directory"),
		pluginsDirectory->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	pluginsDirectory->setText( s );
}
//
void OptionsImpl::slotChooseIncludeDirectory()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project directory"),
		includeDirectory->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	includeDirectory->setText( s );
}
//
void OptionsImpl::slotChooseDocumentationDirectory()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the project directory"),
		documentationDirectory->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel clicked
		return;
	}
	documentationDirectory->setText( s );
}
//
void OptionsImpl::findCodecs()
{
    QMap<QString, QTextCodec *> codecMap;
    QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

    foreach (int mib, QTextCodec::availableMibs())
    {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith("UTF-8"))
        {
            rank = 1;
        }
        else if (sortKey.startsWith("UTF-16"))
        {
            rank = 2;
        }
        else if (iso8859RegExp.exactMatch(sortKey))
        {
            if (iso8859RegExp.cap(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        }
        else
        {
            rank = 5;
        }
        sortKey.prepend(QChar('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    codecs = codecMap.values();
}
void OptionsImpl::setCodecList(const QList<QTextCodec *> &list, int m)
{
    encodingComboBox->clear();
    foreach (QTextCodec *codec, list)
    {
    	encodingComboBox->addItem(codec->name(), codec->mibEnum());
    	if( codec->mibEnum() == m )
    		encodingComboBox->setCurrentIndex( encodingComboBox->count()-1 );
   	}
}
//
int OptionsImpl::mib()
{
    return encodingComboBox->itemData( encodingComboBox->currentIndex() ).toInt();
}

