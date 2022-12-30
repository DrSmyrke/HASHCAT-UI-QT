#include "helpwindow.h"
#include "ui_helpwindow.h"

HelpWindow::HelpWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::HelpWindow)
{
	ui->setupUi(this);
}

HelpWindow::~HelpWindow()
{
	delete ui;
}

void HelpWindow::addHelpLine(const QString &string)
{
	ui->textBrowser->append( string );
}
