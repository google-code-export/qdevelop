#ifndef LOGBUILD_H
#define LOGBUILD_H
//
#include <QTextEdit>
#include <QTextBlockUserData>
//
class MainImpl;
//
class BlockLogBuild : public QTextBlockUserData
{
public:
	BlockLogBuild(QString d) : QTextBlockUserData() { m_directory = d; }
	QString directory() { return m_directory; }
private:
	QString m_directory;
};
//
/*! \brief The LogBuild class is used as build logger displayed in the dock Outputs.
*           
*/
class LogBuild : public QTextEdit
{
Q_OBJECT
public:
	/**
	* The constructor
	*/
	LogBuild(QWidget * parent = 0);
	/**
	* Called by the mainimpl to set the pointer with the address of the main window.
	*/
	void setMainImpl( MainImpl *mainImpl ) { m_mainImpl = mainImpl; }
protected:	
	/**
	* When the user double-click on a error or warning line, the file is opened in a editor.
	*/
    void mouseDoubleClickEvent( QMouseEvent * event );
public slots:
	/**
	* In main window, the signal message(QString, QString) of the class Build is connected
	*			to the slot slotMessagesBuild in this class.
	*/
	void slotMessagesBuild(QString list, QString directory);
private:
	/** 
	* A pointer to the MainImpl class 
	*/
	MainImpl* m_mainImpl;
signals:
	void incErrors();
	void incWarnings();
};
#endif
