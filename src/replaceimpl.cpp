#include "textEdit.h"
#include "replaceimpl.h"
#include "promptreplaceimpl.h"
#include <QMessageBox>
#include <QCloseEvent>

//
ReplaceImpl::ReplaceImpl( QWidget * parent, TextEdit *textEdit, ReplaceOptions options) 
	: QDialog(parent), m_textEdit(textEdit), m_replaceOptions(options)
{
	setupUi(this);
	if( m_textEdit->textCursor().selectedText().length() )
	{
		textFind->insertItem(0, m_textEdit->textCursor().selectedText() );
		textFind->lineEdit()->selectAll();
	}
	textFind->insertItems(0, m_replaceOptions.textFind );
	textReplace->insertItems(0, m_replaceOptions.textReplace );
	backwards->setChecked( m_replaceOptions.backwards );
	caseSensitive->setChecked( m_replaceOptions.caseSensitive );
	prompt->setChecked( m_replaceOptions.prompt );
	wholeWords->setChecked( m_replaceOptions.wholeWords );
}
//
void ReplaceImpl::on_replace_clicked()
{
	PromptReplaceImpl *promptReplace = new PromptReplaceImpl(0);
	QTextDocument::FindFlags options = 0;
    if( backwards->isChecked() )
		options |= QTextDocument::FindBackward;
	if( wholeWords->isChecked() )
		options |= QTextDocument::FindWholeWords;
	if( caseSensitive->isChecked() )
		options |= QTextDocument::FindCaseSensitively;
	hide();
	saveReplaceOptions();
	if( m_textEdit->textCursor().hasSelection() )
	{
		QTextCursor cursor = m_textEdit->textCursor();
    	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, m_textEdit->textCursor().selectedText().length());
    	m_textEdit->setTextCursor(cursor);
   	}
   	int nbFound = 0;
	int choice = 0;
	bool found;
	do
	{
		found = m_textEdit->find(textFind->currentText(), options);
		if( found )
		{
			if( prompt->isChecked() && choice != 1 )
			{
				promptReplace->exec();
				choice = promptReplace->choice();
			}
			if( choice == 0 || choice == 1 ) // Replace, Replace All
			{
				m_textEdit->textCursor().removeSelectedText();
				m_textEdit->textCursor().insertText( textReplace->currentText() );
				nbFound++;
			}
		}
		else
		{
			QString message;
			if( backwards->isChecked() )
                                message = tr("Beginning of document reached.")+"\n"+tr("Continue from the end")+"?";
			else
                                message = tr("End of document reached.")+"\n"+tr("Continue from the beginning")+"?";
            int rep = QMessageBox::question(
                this,
                tr("Replace"),
                QString::number(nbFound)+" "+tr("replacement(s) made.")+"\n"+
                message,
                tr("&Continue"), tr("&Stop"),
                QString(), 0, 1);
            if( rep == 0 )
            {
				QTextCursor cursor = m_textEdit->textCursor();
				if( backwards->isChecked() )
		    		cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
				else	
		    		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
		    	m_textEdit->setTextCursor(cursor);
            	found = true;
            	nbFound = 0;
            }
		}
	} while( found && choice != 3 );
	delete promptReplace;
}
//

void ReplaceImpl::saveReplaceOptions()
{
	QStringList items;
	items << textFind->currentText();
	for(int i=0; i<textFind->count(); i++)
		items << textFind->itemText(i);
	m_replaceOptions.textFind = items;
	items.clear();
	items << textReplace->currentText();
	for(int i=0; i<textReplace->count(); i++)
		items << textReplace->itemText(i);
	m_replaceOptions.textReplace = items;
	m_replaceOptions.backwards = backwards->isChecked();
	m_replaceOptions.prompt = prompt->isChecked();
	m_replaceOptions.caseSensitive = caseSensitive->isChecked();
	m_replaceOptions.wholeWords = wholeWords->isChecked();
}
//
