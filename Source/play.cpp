#include "play.h"

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------
Play::Play(QObject *parent)  :
    QObject(parent) ,
    m_audioOutput(0) ,
    m_status(PLAY_NOT_INIT),
    m_notifyInterval(Play::defaultNotifyInterval) //ms
{

}

Play::~Play()
{
    resetDevice();
}

//-----------------------------------------------------------------------------
// Public static Methods
//-----------------------------------------------------------------------------
QString Play::internalPlaybackStateToString(const Play::PLAY_STATUS &state) {
    QString retval="";
    switch (state) {
        case Play::PLAY_NOT_INIT:
            retval="Play Not Init";
            break;
        case Play::PLAY_PLAY:
            retval="Playing";
            break;
        case Play::PLAY_READY:
            retval="Play ready";
            break;
        case Play::PLAY_SUSPENDED:
            retval="Play Suspended";
            break;
        default:
            retval="Unknown state";
            break;
    }
    return retval;
}

//-----------------------------------------------------------------------------
// Private  slots
//-----------------------------------------------------------------------------
void Play::notified() {
    qDebug() << Q_FUNC_INFO << "Notifed!!";
}

void Play::audioStateChanged(QAudio::State state)
{
    qDebug() << Q_FUNC_INFO << " Entering, internal status is "
             << Play::internalPlaybackStateToString(m_status)
             << " Audio Output status changed to "
             << PlayRecUtils::qAudioStateToString(state);

    //check error
    QAudio::Error error = m_audioOutput->error();
    switch (error) {
        case QAudio::NoError:
            break;
        case QAudio::OpenError:          
        case QAudio::IOError:           
        case QAudio::UnderrunError:           
        case QAudio::FatalError:
            qDebug() << Q_FUNC_INFO << "AudioOutput error: " << PlayRecUtils::decodeInternalAudioErrorToString(error);
            reinit();
            break;
        default:
            Q_ASSERT_X(0, Q_FUNC_INFO, "Unkwon internal error, should never been here!!!");
            break;
    }

    //check status consistency
    switch (state) {
        case QAudio::ActiveState:
            setInternalStatus(Play::PLAY_PLAY);
            qDebug() << Q_FUNC_INFO << "QAudio::ActiveState";
//            if (m_status!=Play::PLAY_PLAY)
//                Q_ASSERT_X(0, Q_FUNC_INFO, "Start to play but internal status is not consistent");
            break;
        case QAudio::SuspendedState:
            setInternalStatus(Play::PLAY_SUSPENDED);
            qDebug() << Q_FUNC_INFO << "QAudio::SuspendedState";
//            if (m_status!=Play::PLAY_SUSPENDED)
//                Q_ASSERT_X(0, Q_FUNC_INFO, "Suspended to play but internal status is not consistent");
            break;
        case QAudio::StoppedState:
            setInternalStatus(Play::PLAY_READY);
            qDebug() << Q_FUNC_INFO << "QAudio::StoppedState";
            break;
        case QAudio::IdleState:
            setInternalStatus(Play::PLAY_READY);
            qDebug() << Q_FUNC_INFO << "QAudio::IdleState";
            break;
        default:
            Q_ASSERT_X(0, Q_FUNC_INFO, "Unkwon internal status, should never been here!!!");
            break;
    }
    qDebug() << Q_FUNC_INFO << "exiting, update internal status is "
             << Play::internalPlaybackStateToString(m_status)
             << " Audio Output is "
             << PlayRecUtils::qAudioStateToString(state);
}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------
const PLAYREC_RETVAL Play::init(QIODevice *audioStream, const QAudioDeviceInfo &outputDevice, const QAudioFormat &format) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    Q_ASSERT_X(m_status==PLAY_NOT_INIT, Q_FUNC_INFO, "Internal status already init");
    Q_ASSERT_X(!m_audioOutput, Q_FUNC_INFO, "Audio device should be empty");

    //Check and assign format
    QAudioFormat outputFormat=QAudioFormat();
    if (format==outputFormat) //an empty (and invalid) audio format
        outputFormat=outputDevice.nearestFormat(format);
    else
        outputFormat=format;

    //opening IO audio output
    m_audioOutput = new QAudioOutput(outputDevice, outputFormat, this);
    m_outputStreamInfo=outputDevice;
    qDebug() << Q_FUNC_INFO << "Open " << outputDevice.deviceName() << " with format: " << PlayRecUtils::formatToString(outputFormat);
    m_audioOutput->setNotifyInterval(m_notifyInterval);
    m_audioStream=audioStream;
    qDebug() << Q_FUNC_INFO << "Audio Output status at init is " << PlayRecUtils::qAudioStateToString(m_audioOutput->state());

    //connect signal from m_audioOutput
    CHECKED_CONNECT(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));
    CHECKED_CONNECT(m_audioOutput, SIGNAL(notify()), this, SLOT(notified()));

    setInternalStatus(Play::PLAY_READY);
    return  retval;
}

const PLAYREC_RETVAL Play::start() {
    return start(0);
}

const PLAYREC_RETVAL Play::start(const quint64 startSample) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready, init before start");
            break;
        case PLAY_SUSPENDED:
            unpause();
            break;
        case PLAY_PLAY:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_PLAYING,"Stream already in execution");
            break;
        case PLAY_READY:
            Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init before start execution!!!");
            Q_ASSERT_X(m_audioStream, Q_FUNC_INFO, "Audio stream must be init before start execution!!!");
    //TODO: SKIPPING THE HEADER 48 bytes JUST FOR DEBUG,
    //EVERY STREAM MUST SKIP ITS HEADER. TODO!!!
    qDebug() << Q_FUNC_INFO << "SKIPPING 48 BYTES HEADER FOR DEBUG PURPOSE";
    m_audioStream->read(48+startSample);
    //END TODO
            m_audioOutput->start(m_audioStream);
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO << errMsg;
                break;
            }
          //  setPosition(startSample);

            break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t start");
            break;
    }

    if (m_status!=PLAY_NOT_INIT) {

    } else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");
    return retval;
}

const PLAYREC_RETVAL Play::stop() {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready. Can''t stop");
            break;
        case PLAY_SUSPENDED:
        case PLAY_PLAY:
            Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init before stopping or unpause!!");
            Q_ASSERT_X(m_audioStream, Q_FUNC_INFO, "Audio stream must be init before stopping or unpause!!!");
            m_audioOutput->stop();
            m_audioStream->reset();
            QCoreApplication::instance()->processEvents();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO <<"Error stopping " << errMsg;
                break;
            }         
            reinit();
            break;
        case PLAY_READY:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_STOPPED,"Internal status not in execution");
            break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t stop");
            break;
    }

    return retval;
}

const PLAYREC_RETVAL Play::resetDevice() {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    if (m_status!=PLAY_NOT_INIT) {
        Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init before reset");
        m_audioOutput->reset();
        m_audioOutput->disconnect();
        QCoreApplication::instance()->processEvents();
        delete m_audioOutput;
        m_audioOutput=NULL;
        m_outputStreamInfo=QAudioDeviceInfo();//empty the information associated to the m_audioOutput device
        setInternalStatus(Play::PLAY_NOT_INIT);
    }  else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");
    return retval;
}

const PLAYREC_RETVAL Play::pause() {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready, can''t pause.");
            break;
        case PLAY_SUSPENDED:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_PAUSED,"The stream is already paused");
            break;
        case PLAY_READY:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_STOPPED,"The stream is not started, can''t pause it.");
            break;
        case PLAY_PLAY:
            Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init before pause it!!");
            m_audioOutput->suspend();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO << errMsg;
                break;
            }
            break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t pause");
            break;
    }
    return retval;
}

const PLAYREC_RETVAL Play::unpause() {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready, can''t unpause.");
            break;
        case PLAY_PLAY:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_PLAYING,"The stream is already playing, , can''t unpause.");
            break;
        case PLAY_READY:
            SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_STOPPED,"The stream is not yet started, , can''t unpause.");
            break;
        case PLAY_SUSPENDED:
            Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init before unpause it!!!");
            m_audioOutput->resume();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO << errMsg;
                break;
            }
            break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t unpause");
            break;
    }
    return retval;
}

const PLAYREC_RETVAL Play::setPosition(const quint64 sample) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    if (m_status!=PLAY_NOT_INIT) {
        Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init");
        //TODO set position base on the sample
      //  emit positionChanged(sample);
    }  else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");

    return retval;
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
inline void Play::setInternalStatus(PLAY_STATUS newStatus) {
    if (newStatus!=m_status) {
        m_status=newStatus;
        emit statusChanged( static_cast<int>(m_status) );
    }
}

const PLAYREC_RETVAL Play::reinit() {
    qDebug() << Q_FUNC_INFO << "Reinit AudioDevice";
    QIODevice *audioStream=m_audioStream;
    QAudioDeviceInfo outputDeviceInfo=m_outputStreamInfo;
    QAudioFormat format=m_audioOutput->format();
    resetDevice();
    return init(audioStream,outputDeviceInfo,format);
}
