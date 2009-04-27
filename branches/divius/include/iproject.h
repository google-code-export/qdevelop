#ifndef __IPROJECT_H__
#define __IPROJECT_H__

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QMap>

namespace QmakeProject
{

//! Type of an item
enum ItemType
{
	Scope,			//!< Anything like "unix {} else {}" or include
	Subproject,		//!< SUBDIRS=...
	Variable,
	Command,	//!< Something that cannot be handled and that is neither a scope not a variable
	AnyType		//!< Pseudo-type that matches any type (of course items of this type cannot be created)
};

//! Role of stored data
enum DataRole
{
	Plain,	//!< Data as it's written in project file
	Short,	//!< Short representation (for files - relative file names)
	Full,	//!< Full representation (for files - absolute file names, for scopes - with contents)
	User	//!< Data type for user purpose (has no effect on .pro file)
};

//! Type of a variable
enum VariableType
{
	FileVariable,	//!< Variable like SOURCES, HEADERS etc
	ListVariable,	//!< Variable like QT, CONFIG etc
	UserVariable,	//!< Unknown variable
};

class IScope;
class IProject;

class IItem
{
public:
	//! Unique for the current scope identefier of the item
	virtual int id() const = 0;
	//! Item type
	virtual ItemType type() const = 0;
	//! Item name
	virtual QString name() const = 0;
	//! Variable type
	virtual VariableType variableType() const = 0;
	//! Fetch data for the chosen role
	virtual QString data(DataRole role = Plain) const = 0;
	//! Set data for the chosen role
	virtual bool setData(const QString & newData, DataRole role = Plain) = 0;
	//! Parent of the item
	virtual IScope * parent() const = 0;
	//! Try to convert item to scope (returns 0 on failure)
	virtual IScope * toScope() = 0;
	//! Try to convert item to scope (returns 0 on failure)
	virtual const IScope * toScope() const = 0;
	//! Try to convert item to project (returns 0 on failure)
	virtual IProject * toProject() = 0;
	//! Try to convert item to project (returns 0 on failure)
	virtual const IProject * toProject() const = 0;
	//! List available for this item variable types
	virtual QMap<VariableType, QStringList> variableTypes() = 0;
	//! Register new variable as a variable of chosen type
	virtual void registerVariable(const QString & name, VariableType type) = 0;
	//! Get type of the variable
	virtual VariableType variableTypeForName(const QString & name) = 0;
};

class IScope : public IItem
{
public:
	virtual QList<IItem *> items() = 0;
	virtual QList<IItem *> itemsOfType(ItemType type) = 0;
	virtual QList<IItem *> variablesOfType(VariableType varType) = 0;
	
	virtual IItem * addItem(ItemType type, const QString & name, 
		VariableType varType = UserVariable);
	virtual bool removeItem(IItem * item);
	
	virtual QVariant variableData(const QString & name, DataRole role = Plain) const = 0;
	virtual bool setVariableData(const QString & name, const QVariant & newData, DataRole role = Plain) = 0;
	virtual bool sortVariable(const QString & name, DataRole role = Plain) = 0;
	
	virtual QString scopeFileName() const = 0;
};

class IProject : public IScope
{
public:
	virtual bool loadFromFile(const QString & fileName) = 0;
	
	virtual QString projectFileName() const = 0;
	virtual QString projectDirName() const = 0;
	virtual QString target() const = 0;
	virtual QString targetFullName() const = 0;
	
	virtual bool save(bool saveSubprojects = true) = 0;
};

};

#endif // __IPROJECT_H__
