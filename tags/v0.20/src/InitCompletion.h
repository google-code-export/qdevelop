/********************************************************************************************************
 * PROGRAM	  : 
 * DATE - TIME  : lundi 10 avril 2006 - 22:28
 * AUTHOR	   : Anacr0x ( fred.julian at gmail.com )
//  * FILENAME	 : InitCompletion.h
 * LICENSE	  : GPL
 * COMMENTARY   : Initialisation class for the icomplete's autocompletion
 ********************************************************************************************************/
#ifndef _INIT_COMPLETION_H
#define _INIT_COMPLETION_H

#include <QStringList>
#include <QList>
#include <QThread>
#include <QFile>

class Tree;
struct Expression;
struct Scope;

class Tag
{
public:
	Tag(){};
	QString name;
	QString parameters;
	QString longName;
	QString kind;
	QString access;
	QString signature;

	bool isFunction;
};
typedef QList<Tag> TagList;

class InitCompletion : public QThread
{
	Q_OBJECT

public:
	InitCompletion (QObject *parent = 0);

	void setTempFilePath (const QString &Path);	// Optionnal
	void setCtagsCmdPath (const QString &cmdPath);
	void addIncludes (QStringList includesPath);
	void run();
	QString className(const QString &text);
	void initParse(const QString &text, bool showAllResults = false, bool emitResults = true);
	void setEmitResults(bool r) { m_emitResults = r; };
	/*
		* @param: filename is a name like "string.h"
		* @return: the file descriptor (fd) and stores
		*          "/usr/include/string.h" in fullname
	*/
	QFile* getFiledescriptor(const QString &filename, QString &fullname);

	QString tagsFilePath,
		tagsIncludesPath,
		ctagsCmdPath,
		smallTagsFilePath,
		parsedFilePath;

private:
	Expression getExpression(const QString &text, Scope &sc, bool showAllResults = false);
	
	/* creates a simple hash for all #include lines of a line */
	long calculateHash(const QString &ParsedText);
	/* forks and executes ctags to rebuild a tags file
	* storing cache_value in the tags file */
	bool buildTagsFile(long cache_value, const QString &parsedText);
	
	QStringList includesList(const QString &parsedText);
	QString includesPathList(const QString &parsedText);

	QStringList cpp_includes;
	QString m_text;
	bool m_showAllResults;
	bool m_emitResults;
signals:
	void completionList( TagList ); 
};

#endif
