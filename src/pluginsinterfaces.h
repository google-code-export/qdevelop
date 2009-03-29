#ifndef __PLUGINSINTERFACES_H__
#define __PLUGINSINTERFACES_H__

class SimplePluginInterface
{
public:
    virtual ~SimplePluginInterface() {}
	virtual QString menuName() const = 0;
	virtual void start(QWidget * owner = 0) = 0;
    virtual bool hasConfigDialog() const = 0;
    virtual void config() = 0;
};

class TextEditInterface
{
public:
	enum Action{ReplaceSelection, ReplaceAll, Append};
    virtual ~TextEditInterface() {}
	virtual QString menuName() const = 0;
    virtual QString text(QString text, QString selectedText, QTextCodec *codec) = 0;
    virtual TextEditInterface::Action action() const = 0;
    virtual bool hasConfigDialog() const = 0;
    virtual void config() = 0;
};

#include <QStringList>

class VcsInterface
{
public:
	virtual ~VcsInterface() {}
	virtual QString name() const = 0;
	
	virtual void prepareRootTree(const QString & treeDir) const = 0;
	
	virtual QString info(const QString & item) = 0;
	virtual bool isVersioned(const QString & item) = 0;
	virtual QString currentCheckOutPath() const = 0;
	virtual QString conflicts() const = 0;
	
	virtual bool checkOut(const QString & path, const QString & destination) = 0;
	
	virtual bool mergeFrom(const QString & path, QString & output) = 0;
	
	virtual bool add(const QStringList & itemList) = 0;
	virtual bool mkdir(const QStringList & dirList) = 0;
	virtual bool removeFiles(const QStringList & itemList) = 0;
	
	virtual bool renameFile(const QString & oldName, const QString & newName) = 0;
	
	virtual QString status(const QStringList & itemList) = 0;
	
	virtual bool commit(const QStringList & itemList, const QString & message, QString & output) = 0;
	
	virtual bool update(const QStringList & itemList, QString & output) = 0;
	
	virtual QString log(const QString & itemName) = 0;
	
	virtual bool revert(const QStringList & itemList) = 0;
	
	virtual bool resolve(const QStringList & itemList) = 0;
	
	virtual bool hasConfigDialog() const = 0;
	virtual void config() = 0;
	
};

Q_DECLARE_INTERFACE(SimplePluginInterface,
                    "qdevelop.SimplePluginInterface/1.0")
Q_DECLARE_INTERFACE(TextEditInterface,
                    "qdevelop.TextEditInterface/1.0")
Q_DECLARE_INTERFACE(VcsInterface,
                    "qdevelop.VcsInterface/1.0")

#endif // __PLUGINSINTERFACES_H__
