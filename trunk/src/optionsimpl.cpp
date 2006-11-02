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
#include "optionsimpl.h"
#include "cpphighlighter.h"
#include <QFontDatabase>
#include <QComboBox>
#include <QPalette>
#include <QColorDialog>
#include <QFileDialog>
#include <QDebug>
//
OptionsImpl::OptionsImpl(QWidget * parent, QFont f, bool num, bool marge, bool ind, 
	bool color, int tab, bool enr, bool res,
	QTextCharFormat pre, QTextCharFormat qt, QTextCharFormat commSimples, 
	QTextCharFormat commMulti, QTextCharFormat guil, QTextCharFormat meth, 
    QTextCharFormat cles, bool autoMask, int end, bool spaces, bool complete, 
    QColor back, bool prompt, QColor lc, bool bk, bool tc, int in, QString directory,
    bool m)
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
    match->setChecked( m );
    projectsDirectory->setText( directory );
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
	if( lc.isValid() )
	{
		pix.fill( lc );
		lineColor->setIcon( pix );
		m_colorCurrentLine = lc;
		groupHighlightCurrentLine->setChecked( true );
		lineColor->setEnabled( true );
	}
	else
	{
		groupHighlightCurrentLine->setChecked( false );
		lineColor->setEnabled( false );
	}
	//
	connect(preprocessor, SIGNAL(clicked()), this, SLOT(slotChangeColor())); 
	connect(qtWords, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(singleComment, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(multilinesComment, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(quotation, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(methods, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(keywords, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(background, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(lineColor, SIGNAL(clicked()), this, SLOT(slotChangeColor()));
	connect(defaults, SIGNAL(clicked()), this, SLOT(slotDefault()));
	connect(chooseProjectsDirectory, SIGNAL(clicked()), this, SLOT(slotChooseProjectsDirectory()));
	textEdit->setPlainText( textEdit->toPlainText() );

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
QColor OptionsImpl::currentLineColor() 
{
	return m_colorCurrentLine;
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
	else if( button == lineColor )
		color = m_colorCurrentLine;
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
		else if( button == lineColor )
			m_colorCurrentLine = color;
		cppHighLighter->setDocument( textEdit->document() );
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
	pix.fill( QColor(238,246,255) );
	m_colorCurrentLine = QColor(238,246,255);
	lineColor->setIcon( pix );
	//
	saveAll->setChecked( true );
	restore->setChecked( true );
	promptBeforeQuit->setChecked( false );
	groupHighlightCurrentLine->setChecked( true );
	numbers->setChecked( true );
	selectionBorder->setChecked( true );
	completion->setChecked( true );
	indent->setChecked( true );
	brackets->setChecked( true );
	highlight->setChecked( true );
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
