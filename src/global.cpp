#include "global.h"

namespace app {
	Config conf;

	bool parsArgs(int argc, char *argv[])
	{
		bool ret = true;
		for(int i=0;i<argc;i++){
			if(QString(argv[i]).indexOf("-")==0){
				if(QString(argv[i]) == "--help" or QString(argv[1]) == "-h"){
					printf("Usage: %s [OPTIONS]\n"
							"  -t <PATH>	target dir\n"
						   "  -r <URL>	repository url\n"
						   "  -k <KEY>	repository\n"
							"\n", argv[0]);
					ret = false;
				}
//				if(QString(argv[i]) == "-t") app::conf.targetDir = QString(argv[++i]);
//				if(QString(argv[i]) == "-r") app::conf.repository = QString(argv[++i]);
//				if(QString(argv[i]) == "-k") app::conf.key = QString(argv[++i]);
			}
		}
		return ret;
	}

	void setLog(const uint8_t logLevel, const QString &mess)
	{
		Q_UNUSED( logLevel )
		Q_UNUSED( mess )
		return;
	}

	void loadSettings()
	{
		QSettings settings( "MySoft", app::conf.appName );

		app::conf.command = settings.value("MAIN/command", app::conf.command).toString();
		app::conf.workDir = settings.value("MAIN/workDir", app::conf.workDir).toString();
		app::conf.hashFile = settings.value("MAIN/hashFile", app::conf.hashFile).toString();
		app::conf.ruleFile = settings.value("MAIN/ruleFile", app::conf.hashFile).toString();
		app::conf.wordList = settings.value("MAIN/wordList", app::conf.wordList).toString();
		app::conf.sessionName = settings.value("MAIN/sessionName", app::conf.sessionName).toString();
		app::conf.brutforceMask = settings.value("MAIN/brutforceMask", app::conf.brutforceMask).toString();
		app::conf.hashType = settings.value("MAIN/hashType", app::conf.hashType).toUInt();
		app::conf.speed = settings.value("MAIN/speed", app::conf.speed).toUInt();
		app::conf.charsets = settings.value("MAIN/charsets", "" ).toString().split( '	' );


		uint8_t i = 0;
		for( const auto &elem:app::conf.charsets ){
			if( elem.isEmpty() ){
				app::conf.charsets.removeAt( i );
				break;
			}
			i++;
		}
	}

	void saveSettings()
	{
		QSettings settings( "MySoft", app::conf.appName );
		settings.clear();

		settings.setValue( "MAIN/command", app::conf.command );
		settings.setValue( "MAIN/workDir", app::conf.workDir );
		settings.setValue( "MAIN/hashFile", app::conf.hashFile );
		settings.setValue( "MAIN/ruleFile", app::conf.ruleFile );
		settings.setValue( "MAIN/wordList", app::conf.wordList );
		settings.setValue( "MAIN/sessionName", app::conf.sessionName );
		settings.setValue( "MAIN/brutforceMask", app::conf.brutforceMask );
		settings.setValue( "MAIN/hashType", app::conf.hashType );
		settings.setValue( "MAIN/speed", app::conf.speed );
		settings.setValue( "MAIN/charsets", app::conf.charsets.join( '	' ) );
	}
}
