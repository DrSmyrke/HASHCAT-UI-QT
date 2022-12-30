#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"



#include <QFileDialog>
#include <QSettings>
#ifdef QT_DEBUG
	#include <QDebug>
#endif

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);



	m_pTimer				= new QTimer( this );
	m_running				= false;
	m_pause					= false;
	m_help					= true;
	m_progress				= 0;
	m_attackMode			= MainWindow::AttackMode::wordlist;
	m_pProcess				= new QProcess( this );
	m_pHelpProcess			= new QProcess( this );
	m_pHelpWindow			= new HelpWindow( this );



	ui->attackModeComboBox->clear();
	ui->attackModeComboBox->addItem( "Wordlist", QVariant( MainWindow::AttackMode::wordlist ) );
	ui->attackModeComboBox->addItem( "Wordlist + Rules", QVariant( MainWindow::AttackMode::wordlist_rules ) );
	ui->attackModeComboBox->addItem( "Brutforce", QVariant( MainWindow::AttackMode::brutforce ) );

	ui->speedComboBox->clear();
	ui->speedComboBox->addItem( tr( "Low" ), QVariant( MainWindow::AttackSpeed::Low ) );
	ui->speedComboBox->addItem( tr( "Economic" ), QVariant( MainWindow::AttackSpeed::Economic ) );
	ui->speedComboBox->addItem( tr( "High" ), QVariant( MainWindow::AttackSpeed::High ) );
	ui->speedComboBox->addItem( tr( "Insane" ), QVariant( MainWindow::AttackSpeed::Insane ) );

	ui->speedComboBox->setCurrentIndex( app::conf.speed - 1 );

	ui->commandLE->setText( app::conf.command );
	ui->hashFileLE->setText( app::conf.hashFile );
	ui->ruleFileLE->setText( app::conf.ruleFile );
	ui->wordlistLE->setText( app::conf.wordList );
	ui->sessionNameLE->setText( app::conf.sessionName );
	ui->bruteforceMaskLE->setText( app::conf.brutforceMask );
	ui->workDirectoryLE->setText( app::conf.workDir );




	ui->pauseB->setIcon( this->style()->standardIcon( QStyle::SP_MediaPause ) );
	ui->startStopB->setText( "" );
	ui->pauseB->setText( "" );

	m_pTimer->setInterval( 250 );
	m_pTimer->start();

	m_inputBuff.clear();

	m_pProcess->setWorkingDirectory( app::conf.workDir );
	m_pHelpProcess->setWorkingDirectory( app::conf.workDir );

	updateCharsets();



	connect( ui->hashTypeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value){
		for( uint32_t i = 0; i < ui->hashTypeComboBox->count(); i++ ){
			auto valueCB = ui->hashTypeComboBox->itemData( i ).toUInt();
			if( value == valueCB ){
				ui->hashTypeComboBox->setCurrentIndex( i );
				break;
			}
		}
	});
	connect( ui->hashTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		auto value = ui->hashTypeComboBox->itemData( index ).toUInt();
		ui->hashTypeSpinBox->setValue( value );
	});
	connect( ui->speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		app::conf.speed = ui->speedComboBox->itemData( index ).toUInt();
	});
	connect( ui->helpB, &QPushButton::clicked, m_pHelpWindow, &HelpWindow::show );
	connect( ui->actionHelp, &QAction::triggered, m_pHelpWindow, &HelpWindow::show );
	connect( m_pProcess, &QProcess::stateChanged, this, [this](){
		switch( m_pProcess->state() ){
			case QProcess::NotRunning:
				ui->statusGroupBox->setTitle( tr( "Not Running" ) );
				m_pause = false;
				m_running = false;
				m_inputBuff.clear();
				if( m_help ) m_help = false;
			break;
			case QProcess::Starting:
				ui->statusGroupBox->setTitle( tr( "Starting" ) );
				ui->statusL->clear();
				ui->passwordL->clear();
				ui->etaL->clear();
				ui->startTimeL->clear();
				ui->speedL->clear();
				m_progress = 0;
			break;
			case QProcess::Running:
				ui->statusGroupBox->setTitle( tr( "Running" ) );
				m_running = true;
			break;
		}
	} );
	connect( m_pProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
		qDebug()<<"EXIT"<<exitCode << exitStatus;
	});
	connect( m_pProcess, &QProcess::readyReadStandardError, this, [this](){
		qDebug()<<"E"<<m_pProcess->readAllStandardError();
	} );
	connect( m_pProcess, &QProcess::readyReadStandardOutput, this, [this](){
		m_inputBuff.append( m_pProcess->readAllStandardOutput() );
		inputProcess();
	} );
	connect( ui->charsetPatternAddB, &QPushButton::clicked, this, [this](){
		auto string = ui->charsetPatternLE->text();
		if( string.isEmpty() ) return;
		app::conf.charsets.push_back( string );
		updateCharsets();
	} );
	connect( ui->startStopB, &QPushButton::clicked, this, [this](){
		if( !m_running ){
			updateData();

			m_pProcess->setWorkingDirectory( app::conf.workDir );

			QStringList arguments;

//			arguments.push_back( "--show" );

			if( !ui->restoreSessionCB->isChecked() ) {
				arguments.push_back( "--status" );
				arguments.push_back( "--status-timer=1" );
				arguments.push_back( "-w" );
				arguments.push_back( QString::number( app::conf.speed ) );
				arguments.push_back( "-a" );

				switch( m_attackMode ){
					case MainWindow::AttackMode::wordlist:
					case MainWindow::AttackMode::wordlist_rules:
						arguments.push_back( "0" );
					break;
					case MainWindow::AttackMode::brutforce:
						arguments.push_back( "3" );
					break;
				}

				arguments.push_back( "-m" );
				arguments.push_back( QString::number( app::conf.hashType ) );
			}

			if( !app::conf.sessionName.isEmpty() ){
				arguments.push_back( "--session" );
				arguments.push_back( app::conf.sessionName );	
			}

			if( ui->restoreSessionCB->isChecked() ) arguments.push_back( "--restore" );

			if( !ui->restoreSessionCB->isChecked() ) {
				if( !app::conf.hashFile.isEmpty() ){
					arguments.push_back( app::conf.hashFile );
				}else{
					arguments.push_back( "--stdout" );
				}

				switch( m_attackMode ){
					case MainWindow::AttackMode::wordlist:
					case MainWindow::AttackMode::wordlist_rules:
						arguments.push_back( app::conf.wordList );
						if( !app::conf.ruleFile.isEmpty() ){
							arguments.push_back( "-r" );
							arguments.push_back( app::conf.ruleFile );
						}
					break;
					case MainWindow::AttackMode::brutforce:
						uint8_t num = 1;
						for( const auto &elem:app::conf.charsets ){
							arguments.push_back( QString( "-%1 %2" ).arg( num++ ).arg( elem ) );
						}
						arguments.push_back( app::conf.brutforceMask );
					break;
				}
			}

			m_pProcess->start( app::conf.command, arguments );
#ifdef QT_DEBUG
			qDebug() << "RUN >:" << m_pProcess->program() << m_pProcess->arguments().join( ' ' );
#endif
		}else{
			sendProcess( "q" );
		}
	} );
	connect( ui->pauseB, &QPushButton::clicked, this, [this](){
		if( !m_running ) return;
		if( m_pause ){
			sendProcess( "p" );
			m_pause = false;
		}else{
			sendProcess( "r" );
			m_pause = true;
		}
	} );
	connect( m_pTimer, &QTimer::timeout, this, [this](){
		ui->pauseB->setEnabled( m_running );
		ui->pauseB->setChecked( m_pause );
		if( m_running ){
			ui->startStopB->setIcon( this->style()->standardIcon( QStyle::SP_MediaStop ) );
		}else{
			ui->startStopB->setIcon( this->style()->standardIcon( QStyle::SP_MediaPlay ) );
		}
		ui->progressBar->setValue( m_progress );
	} );
	connect( ui->workDirSelectB, &QPushButton::clicked, this, [this](){
		QString path = QFileDialog::getExistingDirectory( this, tr( "Open work directory" ), ui->workDirectoryLE->text());
		if( path != "" ){
			ui->workDirectoryLE->setText( path );
		}
	} );
	connect( ui->hashFileSelectB, &QPushButton::clicked, this, [this](){
		QString fileName = QFileDialog::getOpenFileName( this, tr( "Open hash file" ), ( app::conf.hashFile.isEmpty() ) ? QDir::homePath() : app::conf.hashFile, tr( "All Files (*)" ));
		if( fileName != "" ){
			ui->hashFileLE->setText( fileName );
		}
	} );
	connect( ui->ruleFileSelectB, &QPushButton::clicked, this, [this](){
		QString fileName = QFileDialog::getOpenFileName( this, tr( "Open rule file" ), QDir::homePath(), tr( "Rule Files (*.rule)" ));
		if( fileName != "" ){
			ui->ruleFileLE->setText( fileName );
		}
	} );
	connect( ui->wordListSelectB, &QPushButton::clicked, this, [this](){
		QString fileName = QFileDialog::getOpenFileName( this, tr( "Open wordlist file" ), ( app::conf.wordList.isEmpty() ) ? QDir::homePath() : app::conf.wordList, tr( "All Files (*)" ));
		if( fileName != "" ){
			ui->wordlistLE->setText( fileName );
		}
	} );

	connect( ui->attackModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		m_attackMode = ui->attackModeComboBox->itemData( index ).toUInt();
#ifdef QT_DEBUG
		qDebug()<<"SET ATTACK MODE" << m_attackMode;
#endif
		setAttackMode( m_attackMode );
	});


	//HASH TYPES FILL
	ui->hashTypeComboBox->clear();


	m_pHelpProcess->start( app::conf.command, QStringList() << "--help" );
	m_pHelpProcess->waitForStarted();
	m_pHelpProcess->waitForFinished();
	m_inputBuff.append( m_pHelpProcess->readAll() );
	inputProcess();
	m_pHelpProcess->deleteLater();
}

MainWindow::~MainWindow()
{
	if( m_running ){
		m_pProcess->kill();
		m_pProcess->close();
	}
	updateData();
	app::saveSettings();
	delete ui;
}

void MainWindow::init()
{
	ui->attackModeComboBox->setCurrentIndex( 0 );
	uint8_t mode = ui->attackModeComboBox->itemData( 0 ).toUInt();
	setAttackMode( mode );
}

void MainWindow::setAttackMode(const uint8_t mode)
{
	switch( mode ){
		case 1:
			ui->rulesGroupBox->setVisible( false );
			ui->bruteforceGroupBox->setVisible( false );
			ui->wordlistGroupBox->setVisible( true );
		break;
		case 2:
			ui->rulesGroupBox->setVisible( true );
			ui->wordlistGroupBox->setVisible( true );
			ui->bruteforceGroupBox->setVisible( false );
		break;
		case 3:
			ui->rulesGroupBox->setVisible( false );
			ui->wordlistGroupBox->setVisible( false );
			ui->bruteforceGroupBox->setVisible( true );
		break;
		default:
			ui->rulesGroupBox->setVisible( false );
			ui->bruteforceGroupBox->setVisible( false );
			ui->wordlistGroupBox->setVisible( false );
		break;
	}

	this->resize( this->width(), this->minimumHeight() );
}

void MainWindow::updateData()
{
	app::conf.command = ui->commandLE->text();
	app::conf.workDir = ui->workDirectoryLE->text();
	app::conf.hashFile = ui->hashFileLE->text();
	app::conf.ruleFile = ui->ruleFileLE->text();
	app::conf.hashType = ui->hashTypeSpinBox->value();
	app::conf.wordList = ui->wordlistLE->text();
	app::conf.sessionName = ui->sessionNameLE->text();
	app::conf.brutforceMask = ui->bruteforceMaskLE->text();
}

void MainWindow::inputProcess()
{
	static QString mayebePassword;
	static bool helpMode = false;
	m_inputBuff.replace( '\r', "" );
	int index = m_inputBuff.indexOf( '\n' );
	if( index == -1 ) return;

	while( index >= 0 ){
		if( index == 0 ){
			m_inputBuff.remove( 0, 1 );
		}else{
			QString string = QString( m_inputBuff.left( index ) );
			m_inputBuff.remove( 0, index );

			auto list = string.split( ':' );
			if( list.size() >= 2 ) mayebePassword = list[ list.size() - 1 ];

			if( m_help ){
				m_pHelpWindow->addHelpLine( string );
			}else{
				qDebug() << ">:" << string;
			}

			if( helpMode ){
				auto list = string.split( " | " );
				if( list.size() >= 2 ){
					list[ 0 ].replace( ' ', "" );
					list[ 1 ].replace( ' ', "" );
					bool ok = false;
					uint32_t code = list[ 0 ].toUInt( &ok, 10 );
					if( ok ){
						ui->hashTypeComboBox->addItem( list[ 1 ], QVariant( code ) );
//						qDebug() << code << list[ 1 ];
//						continue;
					}
				}
			}else{
				auto list = string.split( ": " );
				if( list.size() == 2 ){
					QString param = list[ 0 ];
					while( param.indexOf( ' ' ) == 0 ) param.remove( 0, 1 );
					param.replace( '.', "" );
//					list[ 1 ].replace( ' ', "" );

					if( param == "Speed#*" ){
							QString value = list[ 1 ];
							while( value.indexOf( ' ' ) == 0 ) value.remove( 0, 1 );
							ui->speedL->setText( value );
					}else if( param == "TimeStarted" ){
						QString value = list[ 1 ];
						while( value.indexOf( ' ' ) == 0 ) value.remove( 0, 1 );
						ui->startTimeL->setText( value );
					}else if( param == "TimeEstimated" ){
						QString value = list[ 1 ];
						while( value.indexOf( ' ' ) == 0 ) value.remove( 0, 1 );
						ui->etaL->setText( value );
					}else if( param == "Status" ){
						QString value = list[ 1 ];
						while( value.indexOf( ' ' ) == 0 ) value.remove( 0, 1 );
						ui->statusL->setText( value );
						if( value == "Cracked" ){
							ui->passwordL->setText( mayebePassword );
						}
					}else if( param == "Progress" ){
						QString value = list[ 1 ];
						while( value.indexOf( ' ' ) == 0 ) value.remove( 0, 1 );
						auto tmp = value.split( " " );
						if( tmp.size() == 2 ){
							tmp[ 1 ].replace( '(', "" );
							tmp[ 1 ].replace( ')', "" );
							tmp[ 1 ].replace( '%', "" );
							m_progress = tmp[ 1 ].toFloat();
						}
					}
				}
			}

			if( string == "- [ Hash modes ] -" ){
				helpMode = true;
			}else if( helpMode && string == "- [ Brain Client Features ] -" ){
				helpMode = false;
				ui->hashTypeSpinBox->setValue( app::conf.hashType );
			}
		}
		index = m_inputBuff.indexOf( '\n' );
	}
}

void MainWindow::sendProcess(const QByteArray &data)
{
	m_pProcess->setCurrentWriteChannel( QProcess::StandardOutput );
	auto len = m_pProcess->write( data );
	fflush( stdout );
}

void MainWindow::updateCharsets()
{
	ui->charsetsListW->clear();

	uint8_t num = 1;

	for( const auto &elem:app::conf.charsets ){
		ui->charsetsListW->addItem( QString( "-%1 %2" ).arg( num++ ).arg( elem ) );
	}
}
