#include <QApplication>
#include $HEADERNAME
//
int main(int argc, char ** argv)
{
	QApplication app( argc, argv );
	$CLASSNAME win;
	win.show(); 
	app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	return app.exec();
}
