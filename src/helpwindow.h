#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QMainWindow>

namespace Ui {
class HelpWindow;
}

class HelpWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit HelpWindow(QWidget *parent = nullptr);
	~HelpWindow();
	void addHelpLine(const QString &string);
private:
	Ui::HelpWindow *ui;
};

#endif // HELPWINDOW_H
