#ifndef BUILD_H
#define BUILD_H

#include <QThread>
#include <QDateTime>
#include <QStringList>
//
class QProcess;

class Build : public QThread
{
Q_OBJECT
public:
	Build(QObject * parent, QString qmakeName, QString makename, QString rep, bool qmake, bool n, bool g, QString compileFile=0);
    void run();
    void incErrors() { m_errors++; }
    void incWarnings() { m_warnings++; }
private:
	bool m_qmake;
	QString projectDirectory;
	QString nameProjectFile;
	bool m_clean;
	bool m_build;
	bool m_isStopped;
	QString m_compileFile;
	QProcess *m_buildProcess;
	QString buildOnly( QString sourceFile );
	QString m_qmakeName;
	QString m_makeName;
	int m_errors;
	int m_warnings;
signals:
	void message(QString, QString=0);
protected slots:
	void slotBuildMessages();
	void slotStopBuild();
};

#endif
