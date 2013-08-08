#include "playrec.h"


//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------

PlayRec::PlayRec(QObject *parent) :
    QObject(parent),
    m_play(NULL),
    m_rec(NULL),
    m_audioMode(PlayRec::PLAY),
    m_lastPlaybackStream(NULL)
{
    //init playback
    m_play=new Play(parent);
    m_lastPlaybackAudioInfo=QAudioDeviceInfo::defaultOutputDevice();
    m_lastPlaybackFormat=m_lastPlaybackAudioInfo.nearestFormat(QAudioFormat());

    //m_rec=new Rec(parent);
    connectSignals();
}

PlayRec::~PlayRec()
{
    if (m_play) delete m_play;
    if (m_rec) delete m_rec;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void PlayRec::connectSignals()
{
    //connect playback position change, a private slot convert sample in time and re-emit the signal
    CHECKED_CONNECT(m_play,SIGNAL(positionChanged(quint64)),this,SLOT(playbackPositionHasChanged(quint64)));

    //connect playback status,Signal2Signal
    CHECKED_CONNECT(m_play,SIGNAL(statusChanged(int)),this,SIGNAL(playbackStatusChanged(int)));

    //Connect playback modification
    CHECKED_CONNECT(m_play,SIGNAL(audioInterfaceChanged(QAudioDeviceInfo)), this, SLOT(playbackInterfaceHasChanged(QAudioDeviceInfo)));
    CHECKED_CONNECT(m_play,SIGNAL(audioFormatChanged(QAudioFormat)) ,this,SLOT(playbackFormatHasChanged(QAudioFormat)));
    CHECKED_CONNECT(m_play,SIGNAL(audioStreamChanged(QIODevice*)),this,SLOT(playbackStreamHasChanged(QIODevice*)));
    CHECKED_CONNECT(m_play,SIGNAL(audioReset()),this,SLOT(playbackHasBeenReset()));

}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------
void PlayRec::start()
{
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    switch (m_audioMode) {
        case PLAY:
            if (m_play->status()==Play::PLAY_NOT_INIT)  {
                qDebug() << Q_FUNC_INFO << "Playback: Resuing previous config";
                result=m_play->init(m_lastPlaybackStream,m_lastPlaybackAudioInfo, m_lastPlaybackFormat);
                if (result.status!=PLAY_OK) {
                    qDebug() << Q_FUNC_INFO << "Can''t init," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
                    break;
                }
            }
            result=m_play->start();
            break;

        case REC:

            break;

        case PLAYANDREC:
            if (m_play->status()==Play::PLAY_NOT_INIT)  {
                qDebug() << Q_FUNC_INFO << "Playback: Resuing previous config";
                result=m_play->init(m_lastPlaybackStream,m_lastPlaybackAudioInfo, m_lastPlaybackFormat);
                if (result.status!=PLAY_OK) {
                    qDebug() << Q_FUNC_INFO << "Can''t init," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
                    break;
                }
            }
            result=m_play->start();
            break;
    }
    if (result.status!=PLAY_OK)
        qDebug() << Q_FUNC_INFO << " operation error," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;

}

void PlayRec::stop()
{
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    switch (m_audioMode) {
        case PLAY:
            m_play->stop();
            break;

        case REC:

            break;

        case PLAYANDREC:
            m_play->stop();
            break;
    }
    if (result.status!=PLAY_OK)
        qDebug() << Q_FUNC_INFO << " operation error," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;

}

void PlayRec::pause(bool pause)
{
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    switch (m_audioMode) {
        case PLAY:
            if (pause)
                result=m_play->pause();
            else
                result=m_play->unpause();
            break;

        case REC:

            break;

        case PLAYANDREC:
            if (pause)
                result=m_play->pause();
            else
                result=m_play->unpause();
            break;
    }
    if (result.status!=PLAY_OK)
        qDebug() << Q_FUNC_INFO << " operation error,"<< (pause ? "pausing" : "unpausing")  << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
}

void PlayRec::resetPlaybackStream() {
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    result=m_play->resetDevice();
    if (result.status!=PLAY_OK)
        qDebug() << Q_FUNC_INFO << " operation error resetting playback," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
}

void PlayRec::setPlaybackStream(QIODevice * stream)
{
    //init playback
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    if (m_play) {
        result=m_play->changeAudioStream(stream);
        if (result.status!=PLAY_OK) {
            qDebug() << Q_FUNC_INFO << "RESET operation error," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
        }
    } else {
        m_play=new Play(this);
        connectSignals();
        result=m_play->init(stream,m_lastPlaybackAudioInfo, m_lastPlaybackFormat);
        if (result.status!=PLAY_OK) {
            qDebug() << Q_FUNC_INFO << "INIT operation error," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
          //  delete m_play;
        }
    }
}

void PlayRec::setPlaybackAudioDevice(const QAudioDeviceInfo &device, const QAudioFormat &format) {
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    if (m_play) {
        delete m_play;
     }
    m_play=new Play(this);
    connectSignals();
    result=m_play->init(m_lastPlaybackStream ,device,format);
    if (result.status!=PLAY_OK) {
        qDebug() << Q_FUNC_INFO << "INIT operation error," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
      //  delete m_play;

    }
}

void PlayRec::setPlaybackAudioDevice(const QAudioDeviceInfo &device) {
    return (m_play ? setPlaybackAudioDevice(device,m_play->audioFormat()) :
                     setPlaybackAudioDevice(device,device.preferredFormat()));
}


void PlayRec::setPlaybackPosition(quint64 sample)
{
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();
    result=m_play->setPosition(sample);
    if (result.status!=PLAY_OK) {
        qDebug() << Q_FUNC_INFO << "setPosition fail," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
    }
}

void PlayRec::setPlaybackPosition(qreal timePosition)
{
    PLAYREC_RETVAL result=PLAYREC_INIT_OK_RETVAL();

    quint64 sample= static_cast<quint64> (timePosition*(static_cast<qreal> (m_play->audioFormat().sampleRate())));
    result=m_play->setPosition(sample);
    if (result.status!=PLAY_OK) {
        qDebug() << Q_FUNC_INFO << "setPosition fail," << PlayRecUtils::playrecReturnValueToString(result.status)<< " - " << result.message;
    }
}

//-----------------------------------------------------------------------------
// Public static methods
//-----------------------------------------------------------------------------
QMap<QString, QAudioDeviceInfo> PlayRec::availablePlaybackDevices()
{
    QMap<QString, QAudioDeviceInfo> retval;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        retval.insert(deviceInfo.deviceName(),deviceInfo);
    }
    return retval;
}

QMap<QString, QAudioDeviceInfo> PlayRec::availableRecordingDevices()
{
    QMap<QString, QAudioDeviceInfo> retval;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        retval.insert(deviceInfo.deviceName(),deviceInfo);
    }
    return retval;
}

//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------
void PlayRec::playbackPositionHasChanged(quint64 sample) {
    //Convert a sample in a time and emit signal
    emit playbackPositionChanged(PlayRecUtils::convertSampleToTime(sample,m_play->audioFormat()));
}

void PlayRec::playbackHasBeenReset() {
    emit playbackChanged(m_play->audioStream(),m_play->audioInterface(),m_play->audioFormat());
}

void PlayRec::playbackInterfaceHasChanged(QAudioDeviceInfo device) {
    m_lastPlaybackAudioInfo=device;
    emit playbackChanged(m_play->audioStream(),m_play->audioInterface(),m_play->audioFormat());
}

void PlayRec::playbackFormatHasChanged(QAudioFormat format) {
    m_lastPlaybackFormat=format;
    emit playbackChanged(m_play->audioStream(),m_play->audioInterface(),m_play->audioFormat());
}

void PlayRec::playbackStreamHasChanged(QIODevice *stream) {
    m_lastPlaybackStream=stream;
    emit playbackChanged(m_play->audioStream(),m_play->audioInterface(),m_play->audioFormat());
}
