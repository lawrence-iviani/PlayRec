#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pauseSelect(false)
{
    ui->setupUi(this);
    ui->LabelStatus->setText(Play::internalPlaybackStateToString(Play::PLAY_NOT_INIT));
    setUILayout();
    populateComboBoxes();
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setUILayout() {
    //setting layout
    ui->centralWidget->setLayout(ui->centralWidgetLayout);
    ui->groupBoxControls->setLayout(ui->layoutControls);
    ui->groupBoxMessage->setLayout(ui->layoutMessage);
    ui->groupBoxPlayback->setLayout(ui->layoutPlayback);
    ui->groupBoxPlaybackInfo->setLayout(ui->layoutPlaybackInfo);
}

void MainWindow::populateComboBoxes() {
    QStringList _pbDevName=PlayRecUtils::availablePlaybackDevices();
    ui->comboBoxPlaybackDevice->addItems(_pbDevName);
}

void MainWindow::connectSignals() {
    //Connect playback controls
    connect(ui->comboBoxPlaybackDevice , SIGNAL(currentIndexChanged(QString)),this , SLOT(comboBoxPlaybackHasChanged(QString)));
    connect(ui->pushButtonOpenFile,SIGNAL(pressed()),this,SLOT(openNewPlaybackFile()));
    connect(ui->pushButtonResetStream,SIGNAL(pressed()),this,SLOT(resetPlaybackFile()));

    //Connect audio controls
    connect(ui->pushButtonPlay,SIGNAL(pressed()),this,SLOT(startPressed()));
    connect(ui->pushButtonStop,SIGNAL(pressed()),this,SLOT(stopPressed()));//&m_playrec,SLOT(stop()));
    connect(ui->pushButtonPause,SIGNAL(pressed()),this,SLOT(pauseToggled()));
    connect(ui->pushButtonResetALL,SIGNAL(pressed()),this,SLOT(resetALLPressed()));//&m_playrec,SLOT(resetAllStream()));
    connect(&m_playrec,SIGNAL(playbackPositionChanged(qreal)),this,SLOT(playbackPositionHasChanged(qreal)));
    connect(ui->sliderStreamPosition,SIGNAL(sliderMoved(int)),this,SLOT(faderPositionHasChanged(int)));

    //connect playrec system to mainwainwindow
    connect(&m_playrec,SIGNAL(playbackStatusChanged(int)),this,SLOT(playbackStatusHasChanged(int)));
    connect(&m_playrec,SIGNAL(playbackChanged(QIODevice*,QAudioDeviceInfo,QAudioFormat)),this,SLOT(playbackDeviceHasChanged(QIODevice*,QAudioDeviceInfo,QAudioFormat)));
}

//PRIVATE SLOTS
void MainWindow::playbackPositionHasChanged(qreal time) {
    int position=static_cast<int>(ui->sliderStreamPosition->maximum()*time/m_playrec.playbackTimeLength());
    ui->sliderStreamPosition->setValue(position);
}

void MainWindow::comboBoxPlaybackHasChanged(QString namePBDev) {
    if (m_playrec.setPlaybackAudioDevice(namePBDev)) {
        ui->messageText->append(QString("Playback device selection was fine"));
    } else {
        ui->messageText->append(QString("Playback device selection issue. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::openNewPlaybackFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home","Wav (*.wav)");
    if (fileName.isEmpty()) return;
    if (m_playbckFile.isOpen()) {
        m_playrec.stop();
        m_playbckFile.close();
    }
    m_playbckFile.setFileName(fileName);
    bool _isOpen=m_playbckFile.open(QIODevice::ReadOnly);

    if ( _isOpen) {
        if (m_playrec.setPlaybackStream(&m_playbckFile)) {
            ui->sliderStreamPosition->setRange( 0,static_cast<int>(m_playrec.playbackTimeLength() ));
            ui->messageText->append(QString("Open file: %1 and set as stream").arg(fileName));
        } else {
            ui->messageText->append(QString("Fail to set as stream file %1. Due to %2").arg(fileName).arg(m_playrec.lastOperationMessage()));
        }
    } else  {
        ui->messageText->append(QString("Can't open file: %1").arg(fileName));
    }
}

void MainWindow::playbackStatusHasChanged(int status) {
    switch(status) {
    case Play::PLAY_NOT_INIT:
        ui->pushButtonPause->setText("pause");
        ui->LabelStatus->setText(Play::internalPlaybackStateToString(Play::PLAY_NOT_INIT));
        m_pauseSelect=false;
        break;
    case Play::PLAY_PLAY:
        ui->pushButtonPause->setText("pause");
        ui->LabelStatus->setText(Play::internalPlaybackStateToString(Play::PLAY_PLAY));
        m_pauseSelect=false;
        break;
    case Play::PLAY_READY:
        ui->pushButtonPause->setText("pause");
        ui->LabelStatus->setText(Play::internalPlaybackStateToString(Play::PLAY_READY));
        m_pauseSelect=false;
        break;
    case Play::PLAY_SUSPENDED:
        ui->pushButtonPause->setText("unpause");
        ui->LabelStatus->setText(Play::internalPlaybackStateToString(Play::PLAY_SUSPENDED));
        m_pauseSelect=true;
        break;
    default:
        qDebug() << Q_FUNC_INFO << "unkwnon status";
        ui->LabelStatus->setText("Unknown status");
    }
}

void MainWindow::playbackDeviceHasChanged(QIODevice *stream, QAudioDeviceInfo device, QAudioFormat format) {

    ui->labelSoundcard->setText(device.deviceName());
    ui->labelFormat->setText(PlayRecUtils::formatToString(format));
    QString _msg;
    _msg.sprintf("%08p", stream);
    ui->labelStream->setText(QString("%1").arg(_msg));
}

void MainWindow::startPressed() {
    int position=ui->sliderStreamPosition->value();
    qreal pos=m_playrec.playbackTimeLength()* (static_cast<qreal>(position)/static_cast<qreal>(ui->sliderStreamPosition->maximum()));
    if (m_playrec.start(pos)) {
        ui->messageText->append(QString("Start was fine"));
    } else {
        ui->messageText->append(QString("Start issue. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::stopPressed() {
    if (m_playrec.stop()) {
        ui->messageText->append(QString("Stop was fine"));
    } else {
        ui->messageText->append(QString("Stop issue. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::resetALLPressed() {
    if (m_playrec.resetAllStream()) {
        ui->messageText->append(QString("Reset ALL  was fine"));
    } else {
        ui->messageText->append(QString("Reset ALL  issue. Due to ").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::resetPlaybackFile() {
    m_playbckFile.close();
    if (m_playrec.setPlaybackStream(NULL)) {
        ui->messageText->append(QString("Reset playbck stream was fine"));
    } else {
        ui->messageText->append(QString("Reset playbck stream issue. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::faderPositionHasChanged(int position)
{
    qreal pos=m_playrec.playbackTimeLength()* (static_cast<qreal>(position)/static_cast<qreal>(ui->sliderStreamPosition->maximum()));
    if (m_playrec.setPlaybackPosition(pos)) {
        ui->messageText->append(QString("Position change was fine"));
    } else {
        ui->messageText->append(QString("Position change issue. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}

void MainWindow::pauseToggled() {
    m_pauseSelect=!m_pauseSelect;
    if (m_playrec.pause(m_pauseSelect)) {
        ui->messageText->append(QString("Pause/unpause was fine"));
    } else {
        ui->messageText->append(QString("Pause/unpause. Due to %1").arg(m_playrec.lastOperationMessage()));
    }
}
