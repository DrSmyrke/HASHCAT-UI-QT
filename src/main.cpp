#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include "global.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QString version = QString("%1.%2").arg(APP_VER_FIRST).arg(APP_VER_SECOND);
#ifdef QT_DEBUG
	version += " (DEV)";
#endif

	QString filename = QLocale::system().name();
	QTranslator translator(&a);
	if(translator.load(filename,"://lang/")) a.installTranslator(&translator);

	app::loadSettings();

	MainWindow w;
	w.setWindowTitle( app::conf.appName + " v" + version );
	w.setWindowIcon( QIcon( "://index.ico" ) );
	w.init();
	w.show();

	return a.exec();
}
