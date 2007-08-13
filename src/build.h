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
	Build(QObject * parent, QString qmakeName, QString makename, QString makeOptions, QString absoluteProjectName, bool qmake, bool n, bool g, QString compileFile=0);
    void run();
    int nbErrors() { return m_errors; }
    int nbWarnings() { return m_warnings; }
private:
	bool m_qmake;
	QString m_projectDirectory;
	QString m_projectName;
	bool m_clean;
	bool m_build;
	bool m_isStopped;
	QString m_compileFile;
	QProcess *m_buildProcess;
	QString buildOnly( QString sourceFile );
	QString m_qmakeName;
	QString m_makeName;
	QString m_makeOptions;
	int m_errors;
	int m_warnings;
signals:
	void message(QString, QString=0);
protected slots:
	void slotBuildMessages();
	void slotStopBuild();
public slots:
    void slotIncErrors();
    void slotIncWarnings();
};

#endif
