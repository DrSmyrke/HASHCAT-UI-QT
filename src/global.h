#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDir>
#include <QSettings>
#include <QString>
#include <list>

struct Config{
	QString version;
	QString appName					= "Hashcat UI";
	QString command					= "hashcat";
	QString workDir					= QDir::homePath();
	QString hashFile				= "";
	QString ruleFile				= "";
	QString wordList				= "";
	QString sessionName				= "";
	QString brutforceMask			= "";
	uint32_t hashType				= 22000;
	uint8_t speed					= 1;
	QStringList charsets;
};

namespace app {
	extern Config conf;

	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);
	void loadSettings();
	void saveSettings();
}

#endif // GLOBAL_H
