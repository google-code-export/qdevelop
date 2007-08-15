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
#ifndef OPTIONSIMPL_H
#define OPTIONSIMPL_H

#include "ui_options.h"
//
class CppHighlighter;
class QTextCodec;
//
class OptionsImpl : public QDialog, public Ui::Options
{
Q_OBJECT
public:
	OptionsImpl(QWidget * parent, QFont f, bool num, bool marge, bool ind, 
		bool coloration, int tab, bool enr, bool res,
		QTextCharFormat pre, QTextCharFormat qt, QTextCharFormat commSimples, 
		QTextCharFormat commMulti, QTextCharFormat guillemets, QTextCharFormat meth, 
		QTextCharFormat cles, bool autoMask, int end, bool spaces, bool complete, 
		QColor back, bool prompt, bool hcl, QColor lc, bool bk, 
		bool tc, int in, QString directory, bool m, QColor mc, bool close, QString pd, QString mo, int mi);
	QFont font();
	CppHighlighter *syntaxe() { return cppHighLighter; };
	QColor backgroundColor();
	QColor currentLineColor();
	QColor matchingColor();
	int mib();
private:
	CppHighlighter *cppHighLighter;
	QColor m_backgroundColor;
    QColor m_colorCurrentLine;
    QColor m_matchingColor;
	void findCodecs();
	void setCodecList(const QList<QTextCodec *> &list, int m);
    QList<QTextCodec *> codecs;
private slots:
	void slotChangeColor();
	void slotDefault();
	void slotChooseProjectsDirectory();
	void slotChoosePluginsDirectory();
signals:
};

#endif
