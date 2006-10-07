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
	Build(QObject * parent, QString qmakeName, QString rep, bool qmake, bool n, bool g, QString compileFile=0);
      void run();
private:
	bool m_qmake;
	QString projectDirectory;
	QString nameProjectFile;
	bool m_clean;
	bool m_build;
	bool m_isStopped;
	QString m_compileFile;
	QProcess *m_buildProcess;
	//QDateTime derniereModifProjet;
	QString buildOnly( QString sourceFile );
	QString m_qmakeName;
signals:
	void message(QString, QString=0);
protected slots:
	void slotBuildMessages();
	void slotStopBuild();
};

#endif
