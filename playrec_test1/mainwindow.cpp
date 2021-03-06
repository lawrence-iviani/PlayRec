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
    ui->frameControls->setLayout(ui->layoutControls);
    ui->frameMessage->setLayout(ui->layoutMessage);
    ui->framePlayback->setLayout(ui->layoutPlayback);
    ui->framePlaybackInfo->setLayout(ui->layoutPlaybackInfo);
}

void MainWindow::populateComboBoxes() {
    QMap<QString,QAudioDeviceInfo> _pbDev=PlayRec::availablePlaybackDevices();
    QStringList _lNames(_pbDev.uniqueKeys());
    ui->comboBoxPlaybackDevice->addItems(_lNames);
}

void MainWindow::connectSignals() {
    //Connect playback controls
    connect(ui->comboBoxPlaybackDevice , SIGNAL(currentIndexChanged(QString)),this , SLOT(comboBoxPlaybackHasChanged(QString)));
    connect(ui->pushButtonOpenFile,SIGNAL(pressed()),this,SLOT(openNewPlaybackFile()));
    connect(ui->pushButtonResetPlayback,SIGNAL(pressed()),&m_playrec,SLOT(resetPlaybackStream()));

    //Connect audio controls
    connect(ui->pushButtonPlay,SIGNAL(pressed()),&m_playrec,SLOT(start()));
    connect(ui->pushButtonStop,SIGNAL(pressed()),&m_playrec,SLOT(stop()));
    connect(ui->pushButtonPause,SIGNAL(pressed()),this,SLOT(pauseToggled()));
    connect(ui->pushButtonResetALL,SIGNAL(pressed()),&m_playrec,SLOT(resetAllStream()));
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
    QMap<QString,QAudioDeviceInfo> _pbDev=PlayRec::availablePlaybackDevices();

    QList<QAudioDeviceInfo> _valuesList = _pbDev.values(namePBDev);
    if (_valuesList.size()!=0) {
        //there should be only one device with that name
        m_playrec.setPlaybackAudioDevice( _valuesList.at(0));
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
        //m_playbckFile.seek(48); //Length of the wav header
        m_playrec.setPlaybackStream(&m_playbckFile);
        ui->sliderStreamPosition->setRange( 0,static_cast<int>(m_playrec.playbackTimeLength() ));
    } else  {
        qDebug() << Q_FUNC_INFO << "Can''t open file:" << fileName;
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

void MainWindow::faderPositionHasChanged(int position)
{
    qreal pos=m_playrec.playbackTimeLength()* (static_cast<qreal>(position)/static_cast<qreal>(ui->sliderStreamPosition->maximum()));
    m_playrec.setPlaybackPosition(pos);
}

void MainWindow::pauseToggled() {
    m_pauseSelect=!m_pauseSelect;
    m_playrec.pause(m_pauseSelect);
}
