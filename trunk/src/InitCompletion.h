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
#include <QPair>
#include <QSqlQuery>

class Tree;
class TreeClasses;
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
	QString returned;
	bool isFunction;
	bool isStatic;
};
typedef QList<Tag> TagList;
//
#include <QTime>
//
class InitCompletion : public QThread
{
	Q_OBJECT

public:
	InitCompletion (QObject *parent, TreeClasses *treeClasses);
	~InitCompletion();
	void setCtagsCmdPath (const QString &cmdPath);
	void addIncludes (QStringList includesPath, QString projectDirectory);
	void run();
	QString className(const QString &text);
	//void setEmitResults(bool r) { m_emitResults = r; };
	/*
		* @param: filename is a name like "string.h"
		* @return: the file descriptor (fd) and stores
		*          "/usr/include/string.h" in fullname
	*/
	void setQtInclude(QString value) { m_qtInclude = value; }
	QFile* getFiledescriptor(const QString &filename, QString &fullname);
	void setStopRequired() { m_stopRequired = true;}
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
	void writeInheritanceToDB(QMap<QString, QString> inheritsList, QSqlQuery query);

	TagList readFromDB(TagList list, Expression exp, QString functionName);
	void writeToDB(Expression exp, TagList list, QSqlQuery query);
	bool connectQDevelopDB(QString const& dbName);
	void createTables();
	void populateQtDatabase();
	QStringList inheritanceList( const QString classname, QStringList &list );
	Expression parseLine( QString text );
	QString returned(QString className, QString function );

	QStringList cpp_includes;
	QString m_text;
	bool m_showAllResults;
	bool m_emitResults;
    bool m_showDuplicateEntries;
    bool m_checkQt;
    QString m_name;
    QString m_projectDirectory;
    QList<QPair<QString, QString> > parsingList;
	QString m_qtInclude;
	bool m_stopRequired;
	TreeClasses *m_treeClasses;
public slots:
	void slotInitParse(QString filename, const QString &text, bool showAllResults, bool emitResults, bool showDuplicateEntries, QString name, bool checkQt);
signals:
	void completionList( TagList ); 
	void completionHelpList( TagList ); 
};

#endif
