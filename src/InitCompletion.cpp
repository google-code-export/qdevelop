
/********************************************************************************************************
 * PROGRAM	  :
 * DATE - TIME  : lundi 10 avril 2006 - 22:28
 * AUTHOR	   : Anacr0x ( fred.julian at gmail.com )
 * FILENAME	 : InitCompletion.h
 * LICENSE	  : GPL
 * COMMENTARY   : Initialisation class for the icomplete's autocompletion
 ********************************************************************************************************/
#include "InitCompletion.h"
#include "./QIComplete/parse.h"
#include "./QIComplete/readtags.h"
#include "./QIComplete/tree.h"
#include "misc.h"
#include "treeclasses.h"

#include <QDir>
#include <QProcess>
#include <QLibraryInfo>
#include <QMetaType>
#include <QTemporaryFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QMetaType>
#include <QCoreApplication>

#ifdef _WIN32
#define NEW_LINE "\r\n"
#else
#define NEW_LINE "\n"
#endif
#include <QDebug>

#define QD qDebug() << __FILE__ << __LINE__ << ":"

extern QString simplifiedText( QString );

InitCompletion::InitCompletion (QObject *parent, TreeClasses *treeClasses)
        : QThread(parent), m_treeClasses(treeClasses)
{
    qRegisterMetaType<TagList>("TagList");
    m_stopRequired = false;
}
//
InitCompletion::~InitCompletion()
{
    QStringList list = QDir( QDir::tempPath() ).entryList(QStringList() << "qdevelop-completion-*", QDir::Files);
    foreach(QString file, list)
    {
        QFile( QDir::tempPath()+"/"+file ).remove();
    }
    if ( m_stopRequired )
    {
        if ( !connectQDevelopDB( getQDevelopPath() + "qdevelop.db" ) )
        {
            return;
        }
        createTables();
        QSqlQuery query;
        query.exec("BEGIN TRANSACTION;");
        QString queryString = "delete from tags";
        query.exec(queryString);
        query.exec("END TRANSACTION;");
    }
}

//
void InitCompletion::slotInitParse(QString filename, const QString &text, bool showAllResults, bool emitResults, bool /*showDuplicateEntries*/, QString name, bool checkQt)
{

    m_text = text;
    m_emitResults = emitResults;
    m_showAllResults = showAllResults;
    m_name = name;
    m_checkQt = checkQt;

    QString Path = QDir::tempPath();
    tagsIncludesPath = Path + '/' + "qdevelop-completion-" + QFileInfo(filename).baseName() + "-tags_includes";
    tagsFilePath = Path + '/' + "qdevelop-completion-" + QFileInfo(filename).baseName() + "-tags";
    smallTagsFilePath = Path + '/' + "qdevelop-completion-" + QFileInfo(filename).baseName() + "-small-tags";
    parsedFilePath = Path + '/' + "qdevelop-completion-" + QFileInfo(filename).baseName() + "-parsed_file";

    this->start();
}

void InitCompletion::addIncludes (QStringList includesPath, QString projectDirectory)
{
    QDir dir;
    QFileInfoList list;

    for (int i = 0; i < includesPath.size(); i++)
    {
        dir.setPath(includesPath[i]);
        if (dir.exists() && !cpp_includes.contains(includesPath[i]))
            cpp_includes << includesPath[i];

        list = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Hidden);
        for (int j = 0; j < list.size(); ++j)
            includesPath.insert(i+1, list[j].absoluteFilePath());
    }
    m_projectDirectory = projectDirectory;
}

QStringList InitCompletion::includesList(const QString &parsedText)
{
    QStringList list;

    /* find include'ed files */
    QRegExp rx("#\\s*include\\s*(<[^>]+>|\"[^\"]+\")");
    QString include;

    int pos = 0;
    while ((pos = rx.indexIn(parsedText, pos)) != -1)
    {
        include = rx.cap(1);

        include.remove(0, 1);
        include.remove(include.length() - 1, 1);
        list << include;

        pos += rx.matchedLength();
    }

    return list;
}

QString InitCompletion::includesPathList(const QString &parsedText)
{
    QString list, fullpath, buf;
    QStringList includes = includesList(parsedText);
    while (!includes.empty())
    {
        QFile *temp_fd = getFiledescriptor(includes.first(), fullpath);
        includes.removeFirst();
        if (temp_fd)
        {
            if (!list.contains(fullpath))
            {
                list += fullpath + NEW_LINE;
                buf = temp_fd->readAll();
                includes << includesList(buf);
            }

            temp_fd->close();
            delete temp_fd;
        }
    }

    return list;
}

/* creates a simple hash for all #include lines of a line */
long InitCompletion::calculateHash(const QString &parsedText)
{
    long res = 0;

    QStringList includes = includesList(parsedText);
    QString s;
    foreach (s, includes)
    {
        for (int i = 0; i < s.length(); i++)
            res += (i + 1) * s[i].toAscii();
    }

    return res;
}

/* forks and executes ctags to rebuild a tags file
 * storing cache_value in the tags file */
bool InitCompletion::buildTagsFile(long cache_value, const QString &parsedText)
{
    QString pathList = includesPathList(parsedText);

    QFile includesListFile(tagsIncludesPath);
    if (includesListFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        includesListFile.write (pathList.toLocal8Bit());
        includesListFile.close();

        /* save the cache information in the tags file */
        QFile tags(tagsFilePath);
        if (tags.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QString head = "!_TAG_FILE_FORMAT	2	/extended format; --format=1 will not append ;\" to lines/" NEW_LINE
                           "!_TAG_FILE_SORTED	1	/0=unsorted, 1=sorted, 2=foldcase/" NEW_LINE
                           "!_TAG_PROGRAM_AUTHOR	Darren Hiebert	/dhiebert@users.sourceforge.net/" NEW_LINE
                           "!_TAG_PROGRAM_NAME	Exuberant Ctags	//" NEW_LINE
                           "!_TAG_PROGRAM_URL	http://ctags.sourceforge.net	/official site/" NEW_LINE
                           "!_TAG_PROGRAM_VERSION	5.5.4	//" NEW_LINE
                           "!_TAG_CACHE\t" + QString::number(cache_value) + NEW_LINE;
            tags.write(head.toLocal8Bit());
            tags.close();
        }

        QString command = ctagsCmdPath + " -f \"" + tagsFilePath +
                          "\" --append --language-force=c++ --fields=afiKmsSzn --c++-kinds=cdefgmnpstuvx -L \""
                          + tagsIncludesPath + '\"';
        // I don't process any user input, so system() should be safe enough
        QProcess ctags;
        ctags.execute(command);

        includesListFile.remove();
        if (ctags.exitStatus() == QProcess::NormalExit)
            return true;
    }
    return false;
}

Expression InitCompletion::getExpression(const QString &text, Scope &sc, bool showAllResults)
{
    Tree::parent = this;

    long cache_value = calculateHash(text);

    /* automatic cache mode */
    /* we save a hash value for all included files from the current file
    in the tags file, if it matches the hash of the current buffer, nothing
    has changed, and reuse the existing tags file */

    bool build_cache = true;
    QFile fCache(tagsFilePath);
    if (fCache.open(QIODevice::ReadOnly))
    {
        QString pattern = "!_TAG_CACHE\t" + QString::number(cache_value) + NEW_LINE;
        for (int i=0; i<10; i++)
            if (fCache.readLine() == pattern)
                build_cache = false;

        fCache.close();
    }
    if (build_cache)
        buildTagsFile(cache_value, text);

    /* We create a file with the parsed text */
    QFile f(parsedFilePath);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    f.write (text.toLocal8Bit());
    f.close();

    /* real parsing starts here */
    Parse parse(ctagsCmdPath, tagsFilePath, parsedFilePath, smallTagsFilePath);
    return parse.parseExpression(text, &sc, showAllResults);
}

void InitCompletion::run()
{
    if ( m_checkQt )
    {
        Expression exp;
        exp.className = "QString";
        TagList list;
        list = readFromDB(list, exp, QString());
        if ( list.count() )
        {
            return;
        }
   		emit showMessage( tr("The Qt database will be rebuilt now.") );
        populateQtDatabase();
    	emit showMessage( tr("The Qt classes database build is ended.") );
        return;
    }
    Scope sc;
    Expression exp = parseLine( m_text );
    /* we have all relevant information, so just list the entries */
    TagList list, newList, listForDB;
    if ( m_emitResults )
    {
        if (exp.access != ParseError )
        {
            if ( !exp.className.isEmpty() )
            {
                // Try to read from database
                list = readFromDB(list, exp, m_name.simplified());
                // If the list is empty, the classe is not present in database
                if ( list.isEmpty() )
                {
                    // Populate the list by reading the tag file on disk
                    list = Tree::findEntries(&exp, &sc);
                }
                if ( m_name.simplified().isEmpty() )
                    emit completionList( list );
                else
                    emit completionHelpList( list );
            }
        }

    }
}

QString InitCompletion::className(const QString &text)
{
    Scope sc;
    Expression exp = getExpression(text, sc);
    if (exp.access == ParseError)
        return QString();

    return exp.className;
}

void InitCompletion::setCtagsCmdPath (const QString &cmdPath)
{
    if (cmdPath.indexOf(' ')!=-1)
        ctagsCmdPath = QString('\"') + cmdPath + '\"';
    else
        ctagsCmdPath = cmdPath;
}

QFile* InitCompletion::getFiledescriptor(const QString &filename, QString &fullname)
{
    QFile *fd = new QFile();

    /* absolute path name */
    if (QFileInfo(filename).isAbsolute())
    {
        fd->setFileName(filename);
        if (fd->open(QIODevice::ReadOnly))
        {
            fullname = QFileInfo(filename).canonicalFilePath();
            return fd;
        }
    }
    else
    {
        /* relative pathname */
        for (int i = 0; i < cpp_includes.size(); i++)
        {
            fd->setFileName(cpp_includes.at(i) + '/' + filename);
            if (fd->open(QIODevice::ReadOnly))
            {
                fullname = QFileInfo(cpp_includes.at(i) + '/' + filename).canonicalFilePath();
                return fd;
            }
        }
    }

    // Nothing was found
    fd->close();
    delete fd;
    return NULL;
}
//
TagList InitCompletion::readFromDB(TagList list, Expression exp, QString functionName)
{
    if ( exp.className.isEmpty() )
        return TagList();
    QStringList classes;
    classes << exp.className;
    if ( !connectQDevelopDB( getQDevelopPath() + "qdevelop.db" ) )
    {
        return TagList();
    }
    /* The first character is not a 'Q', certainly an class created in the project. We read the list
    from the class browser which have all the classes and members of the project */
    classes = inheritanceList(exp.className, classes);
    const QList<ParsedItem> *itemsList = m_treeClasses->treeClassesItems();
    for (int i = 0; i < itemsList->size(); ++i)
    {
        ParsedItem parsedItem = itemsList->at( i );
        Tag tag;
        tag.access = parsedItem.access;
        tag.name = parsedItem.name;
        tag.parameters = parsedItem.signature;
        tag.signature = parsedItem.signature;
        tag.returned = parsedItem.ex_cmd.section(" ", 0, 0);
        if ( tag.returned.contains("<") )
            tag.returned = tag.returned.section("<", 0, 0);
        if ( parsedItem.kind == "f" )
            tag.kind = "function";
        else if ( parsedItem.kind == "p" )
            tag.kind = "prototype";
        else if ( parsedItem.kind == "c" )
        {
            tag.kind = "class";
            tag.parameters = "";
        }
        else if ( parsedItem.kind == "e" )
        {
            if (  parsedItem.enumname.section(":", 0, 0) != exp.className )
                continue;
            parsedItem.classname = exp.className;
            tag.parameters = "";
            tag.kind = "member";
        }
        else if ( parsedItem.kind == "m" )
        {
            tag.kind = "member";
            tag.parameters = "";
        }
        else if ( parsedItem.kind == "s" )
        {
            tag.kind = "struct";
            tag.parameters = "";
        }
        else
        {
            continue;
        }
        bool isStatic = parsedItem.ex_cmd.simplified().startsWith("static");
        if ( !classes.contains( parsedItem.classname ) 
        	&& !classes.contains( parsedItem.structname )
        	&& !classes.contains( parsedItem.structname.section("::", -1) )
        	 )
            continue;
        else if ( !functionName.isEmpty() && parsedItem.name != functionName )
            continue;
        else if ( parsedItem.name == parsedItem.classname ||  "~"+parsedItem.name == parsedItem.classname )
            continue;
        else if ( tag.access != "public" && !m_text.simplified().endsWith("this->") )
            continue;
        else if ( (exp.access == AccessStatic && isStatic != true && tag.kind != "struct" && tag.kind != "class")
                  || (exp.access != AccessStatic && isStatic == true))
            continue;
        list << tag;
    }
    // Continue the reading in qdevelop.db
    createTables();
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");
    //
    QString queryString = QString()
                          + "select * from inheritance where child='"+exp.className+"'";
    query.exec(queryString);
    while (query.next())
    {

        QString parent = query.value(0).toString().replace("$", "'");
        if ( !parent.isEmpty() )
        {
            classes << parent;
        }
    }
    //
    queryString = QString()
                  + "select * from tags where class in ( ";
    foreach(QString c, classes)
    {
        queryString = queryString + " '" + c + "',";
    }
    queryString = queryString.left( queryString.length()-1 );
    queryString += " )";
    if ( !functionName.simplified().isEmpty() )
    {
        queryString += " and name='" + functionName + "'";
    }
    //
    query.exec(queryString);
    while (query.next())
    {
        Tag tag;
        tag.name = query.value(1).toString().replace("$", "'");
        tag.parameters = query.value(2).toString().replace("$", "'");
        tag.longName = query.value(3).toString().replace("$", "'");
        tag.kind = query.value(4).toString().replace("$", "'");
        tag.access = query.value(5).toString().replace("$", "'");
        tag.signature = query.value(6).toString().replace("$", "'");
        tag.returned = query.value(7).toString().replace("$", "'");
        tag.isFunction = query.value(8).toInt();
        tag.isStatic = query.value(9).toInt();
        if ( (exp.access == AccessStatic && tag.isStatic != true)
                || (exp.access != AccessStatic && tag.isStatic == true))
            continue;
        else if ( tag.access != "public" && !m_text.simplified().endsWith("this->") )
            continue;
        if ( list.count() && functionName.simplified().isEmpty())
        {
            Tag lastTag = list.last();
            if ( lastTag.name == tag.name )
                list.pop_back();
        }
        list << tag;
    }
    return list;
}
//
bool InitCompletion::connectQDevelopDB(QString const& dbName)
{
    QSqlDatabase database;

    if ( QSqlDatabase::database().databaseName() != dbName )
    {
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(dbName);
    }
    else
    {
        database = QSqlDatabase::database();
        if ( database.isOpen() )
            return true;
    }
    //
    if (!database.open())
    {
        QMessageBox::critical(0, "QDevelop",
                              QObject::tr("Unable to establish a database connection.")+"\n"+
                              QObject::tr("QDevelop needs SQLite support. Please read "
                                          "the Qt SQL driver documentation for information how "
                                          "to build it."), QMessageBox::Cancel,
                              QMessageBox::NoButton);
        return false;
    }
    return true;
}
//
void InitCompletion::writeToDB(Expression exp, TagList list, QSqlQuery query)
{
    foreach(Tag tag, list)
    {
        QString queryString = "insert into tags values(";
        queryString = queryString
                      + "'" + exp.className.replace("'", "$") + "', "
                      + "'" + tag.name.replace("'", "$") + "', "
                      + "'" + tag.parameters.replace("'", "$") + "', "
                      + "'" + tag.longName.replace("'", "$") + "', "
                      + "'" + tag.kind.replace("'", "$") + "', "
                      + "'" + tag.access.replace("'", "$") + "', "
                      + "'" + tag.signature.replace("'", "$") + "', "
                      + "'" + tag.returned.replace("'", "$") + "', "
                                        + "'" + QString::number(tag.isFunction) + "', "
                                        + "'" + QString::number(tag.isStatic) + "')";
        bool rc = query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to insert record to db" << query.lastError();
            qDebug() << queryString;
            return;
        }
    }
}
//
void InitCompletion::createTables()
{
    QSqlQuery query;
    QString queryString = "create table tags ("
                          "class string,"
                          "name string,"
                          "parameters string,"
                          "longName string,"
                          "kind string,"
                          "access string,"
                          "signature string,"
                          "returned string,"
                          "isFunction int,"
                          "isStatic int"
                          ")";
    query.exec(queryString);
    //
    queryString = "create table inheritance ("
                  "parent string,"
                  "child string"
                  ")";
    query.exec(queryString);
}
//

void InitCompletion::populateQtDatabase()
{
    QString command = ctagsCmdPath + " -R -f \"" + QDir::tempPath()+"/qttags" +
                      "\" --language-force=c++ --fields=afiKmsSzn --c++-kinds=cdefgmnpstuvx \""
                      + m_qtInclude + '\"';
    QProcess ctags;
    if( ctags.execute(command) != 0 )
    {
    	emit showMessage( tr("Unable to launch %1").arg(command) );
    	return;
   	}
    ctags.waitForFinished(-1);
    QFile file(QDir::tempPath()+"/qttags");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    	emit showMessage( tr("Unable to open %1").arg(QDir::tempPath()+"/qttags") );
        return;
   	}
    QString read;
    read = file.readAll();
    file.close();
    file.remove();

    QMap<QString, QString> inheritsList;
    QMap<QString, TagList> map;
    foreach(QString s, read.split("\n", QString::SkipEmptyParts) )
    {
        QString classname;
        Tag tag;
        if ( !s.isEmpty() && s.simplified().at(0) == '!' )
            continue;
        s += '\t';
        tag.name = s.section("\t", 0, 0).simplified();
        QString ex_cmd = s.section("/^", -1, -1).section("$/", 0, 0).simplified();
        tag.isStatic = ex_cmd.startsWith("static");
        classname = s.section("class:", -1, -1).section("\t", 0, 0).simplified();
        tag.access =  s.section("access:", -1, -1).section("\t", 0, 0).simplified();
        if ( !QString("public:private:protected").contains( tag.access ) )
            tag.access = "public";
        tag.longName =  ex_cmd;
        tag.kind = s.section("kind:", -1, -1).section("\t", 0, 0).simplified();
        if ( classname.isEmpty() )
            continue;
        tag.parameters = "(" + tag.longName.section("(", 1, 1).section(")", -2, -2) + ")";
        tag.signature = ex_cmd;
        tag.isFunction = true;
        tag.signature = s.section("signature:", -1, -1).section("\t", 0, 0).simplified();
        tag.kind = s.section("kind:", -1, -1).section("\t", 0, 0).simplified();
        tag.returned = ex_cmd;
        if ( tag.returned.startsWith("static ") )
            tag.returned = tag.returned.section("static ", 1);
        if ( tag.returned.startsWith("inline ") )
            tag.returned = tag.returned.section("inline ", 1);
        if ( tag.returned.startsWith("const ") )
            tag.returned = tag.returned.section("const ", 1);
        if ( tag.returned.startsWith("QT3_SUPPORT ") )
            tag.returned = tag.returned.section("QT3_SUPPORT ", 1);
        tag.returned = tag.returned.section(" ", 0, 0);
        if ( tag.returned.contains("<") )
            tag.returned = tag.returned.section("<", 0, 0);
        QString inherits;
        if ( s.contains("inherits:") )
            inherits = s.section("inherits:", -1, -1).section("\t", 0, 0).simplified();
        if ( !inherits.isEmpty() )
        {
            inheritsList[ classname ] = inherits;
        }
        if ( tag.kind != "prototype" && tag.kind != "function" && tag.kind != "member" )
            continue;
        if ( tag.name=="Q_REQUIRED_RESULT" )
        {
            if ( tag.longName.contains("(") )
            {
                tag.name = tag.longName.remove("const ").section(" ", 1, 1).section("(", 0, 0);
                tag.kind = "function";
                tag.parameters = tag.signature = "(" + tag.longName.section("(", 1, 1).section(")", -2, -2) + ")";
                tag.access = "public";
                tag.isFunction = true;
            }
        }
        if ( tag.name=="Q_REQUIRED_RESULT" || tag.name.at(0)=='&' || tag.name.at(0)=='*' )
            continue;
        if( tag.name == classname || tag.name == "~"+classname )
        	continue;
        TagList list;
        list = map[classname];
        bool alreadyInserted = false;
        foreach(Tag t, list)
        {
            if ( t.name == tag.name && t.parameters == tag.parameters )
            {
                alreadyInserted = true;
                break;
            }
        }
        if ( !alreadyInserted )
        {
            list << tag;
            map[classname] = list;
        }
    }
    if ( !connectQDevelopDB( getQDevelopPath() + "qdevelop.db" ) )
    {
        return;
    }
    createTables();
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");
    query.exec("delete from tags");
    QStringList keys = map.keys();
    foreach(QString key, keys)
    {
        if ( m_stopRequired )
            break;
        TagList tagList = map[key];
        Expression exp;
        exp.className = key;
        writeToDB(exp, tagList, query);
    }
    writeInheritanceToDB(inheritsList, query);
    query.exec("END TRANSACTION;");
}
//
//
void InitCompletion::writeInheritanceToDB(QMap<QString, QString> inheritsList, QSqlQuery query)
{
    QMapIterator<QString, QString> it(inheritsList);
    while (it.hasNext())
    {
        it.next();
        foreach(QString parent, it.value().split(",", QString::SkipEmptyParts ) )
        {
            QString queryString = "insert into inheritance values(";
            queryString = queryString
                          + "'" + parent.replace("'", "$") + "', "
                          + "'" + QString(it.key()).replace("'", "$") + "')";
            bool rc = query.exec(queryString);
            if (rc == false)
            {
                qDebug() << "Failed to insert record to db" << query.lastError();
                qDebug() << queryString;
                return;
            }
        }
    }
}
//
QStringList InitCompletion::inheritanceList( const QString classname, QStringList &list )
{
	/* Return a string list with inheritance classes for classname. Work with the classes browser. 
	For the Qt classes, the inheritance is read in qdevelop.db. */
    const QList<ParsedItem> *itemsList = m_treeClasses->treeClassesItems();
    for (int i = 0; i < itemsList->size(); ++i)
    {
        if ( itemsList->at( i ).kind == "c" && itemsList->at( i ).name == classname 
        	&& itemsList->at( i ).ex_cmd.contains(":") )
        {
            QString ex_cmd = itemsList->at( i ).ex_cmd;
            ex_cmd = ex_cmd.left( ex_cmd.length()-3 );
            ex_cmd = ex_cmd.section(":", 1);
            ex_cmd = ex_cmd.remove("public").remove("protected").remove("private").remove(" ");
            foreach(QString c, ex_cmd.split(",") )
            {
                list << c;
                list = inheritanceList(c, list);
            }
        }
    }
    //
    QSqlQuery query;
    QString queryString = QString()
                          + "select * from inheritance where child='"+classname+"'";
    query.exec(queryString);
    while (query.next())
    {

        QString parent = query.value(0).toString().replace("$", "'");
        if ( !parent.isEmpty() )
        {
            list << parent;
    		list = inheritanceList(parent, list);
        }
    }
    return list;
}
//
Expression InitCompletion::parseLine( QString text )
{
    Expression exp;
    Scope sc;
    int p = 0;
    QString simplified = simplifiedText( text );
    int begin = simplified.length()-1;
    do
    {
        begin--;
    	if( simplified[begin]=='(' )
    		p--;
    	else if( simplified[begin]==')' )
    		p++;
    	if( p > 0 && !(p == 1 && simplified[begin]==')') )
    		simplified[begin]=' ';
   	} while ( begin>0 && !QString(";{}=*/+~&|!^?:,").contains(simplified[begin]) && !( simplified[begin]=='(' && p<0) );
	//
	QString word = simplified.mid(begin+1).simplified();
	int posWord = 0;
	while( posWord<word.length()-1 && (word.at( posWord ).isLetterOrNumber() || word.at( posWord )=='_') )
		posWord++;
	word = word.left(posWord);
	//
    while ( begin<simplified.length() && simplified[begin]!='.' && simplified[begin]!='>' 
    		&& !(begin>0 && simplified[begin-1]==':' && simplified[begin]==':' )
    	)
        begin++;
    QString varName = simplified.left(begin+1);
    QString line = simplified.mid(begin+1);
    exp = getExpression(varName, sc, m_showAllResults);
    QString className;
    if ( exp.access == ParseError )
    {
    	className = word;
    	exp.access = AccessMembers;
   	}
    else
    	className = exp.className;
    //
    while ( line.indexOf('.')!=-1 || line.indexOf(">")!=-1  || line.indexOf("::")!=-1 )
    {
        QString function;
        int end = 9999;
        if( line.indexOf('.') != -1 )
        	end = qMin(end, line.indexOf('.'));
        if( line.indexOf('>') != -1 )
        	end = qMin(end, line.indexOf('>'));
        if( line.indexOf(':') != -1 )
        	end = qMin(end, line.indexOf(':')+1);
       	function = line.left(end);
        line = line.mid(end+1);
        className = returned(className, function, exp);
    }
    exp.className = className;
    return exp;
}
//
QString InitCompletion::returned(QString className, QString function, Expression &exp)
{
    QStringList classes;
    classes << className;
    classes = inheritanceList(className, classes);
    //
	function = function.section("(", 0, 0).simplified();
    const QList<ParsedItem> *itemsList = m_treeClasses->treeClassesItems();
    for (int i = 0; i < itemsList->size(); ++i)
    {
        ParsedItem parsedItem = itemsList->at( i );
        Tag tag;
        tag.access = parsedItem.access;
        tag.name = parsedItem.name;
        tag.parameters = parsedItem.signature;
        tag.returned = parsedItem.ex_cmd.section(" ", 0, 0);
        if ( tag.returned.contains("<") )
            tag.returned = tag.returned.section("<", 0, 0);
        if ( parsedItem.kind == "f" )
            tag.kind = "function";
        else if ( parsedItem.kind == "p" )
            tag.kind = "prototype";
        else if ( parsedItem.kind == "e" )
        {
            if (  parsedItem.enumname.section(":", 0, 0) != className )
                continue;
            parsedItem.classname = className;
            tag.parameters = "";
            tag.kind = "member";
        }
        else if ( parsedItem.kind == "m" )
        {
            tag.kind = "member";
            tag.parameters = "";
        }
        else
        {
            continue;
        }
//        bool isStatic = parsedItem.ex_cmd.simplified().startsWith("static");
        if ( !classes.contains( parsedItem.classname ) )
            continue;
        if( classes.contains( parsedItem.classname ) && function == parsedItem.name )
        {
        	
        	exp.access = AccessMembers;
        	return tag.returned;
       	}
    }
    // Continue the reading in qdevelop.db
    createTables();
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");
    QString queryString = QString()
                          + "select * from inheritance where child='"+className+"'";
    query.exec(queryString);
    while (query.next())
    {

        QString parent = query.value(0).toString().replace("$", "'");
        if ( !parent.isEmpty() )
        {
            classes << parent;
        }
    }
    queryString = QString()
                  + "select * from tags where class in ( ";
    foreach(QString c, classes)
    {
        queryString = queryString + " '" + c + "',";
    }
    queryString = queryString.left( queryString.length()-1 );
    queryString += " )";
    //
    queryString += " and name = '" + function + "'";
    query.exec(queryString);
    if (query.next())
    {
      	exp.access = AccessMembers;
       	return query.value(7).toString().replace("$", "'");
    }
	return QString();
}
//
