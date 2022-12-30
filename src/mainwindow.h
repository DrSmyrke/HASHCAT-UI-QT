#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QProcess>
#include "helpwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	struct AttackMode{
		enum{
			wordlist		= 1,
			wordlist_rules,
			brutforce,
		};
	};
	struct AttackSpeed{
		enum{
			Low		= 1,
			Economic,
			High,
			Insane,
		};
	};
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	void init();
//private slots:
//	void slot_readyRead();
//	void slot_findHubs();
//	void slot_updateVHubsConfig();
private:
	Ui::MainWindow *ui;
	HelpWindow* m_pHelpWindow;
//	QMenu* m_pHubMenu;
//	QSystemTrayIcon* m_pTreeIco;
	QTimer* m_pTimer;
	bool m_running;
	bool m_pause;
	bool m_help;
	float m_progress;
	uint8_t m_attackMode;
	QProcess* m_pProcess;
	QProcess* m_pHelpProcess;
	QByteArray m_inputBuff;

	void setAttackMode(const uint8_t mode);
	void updateData();
	void inputProcess();
	void sendProcess(const QByteArray &data);
	void updateCharsets();
};
#endif // MAINWINDOW_H
