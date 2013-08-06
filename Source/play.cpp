#include "play.h"

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------
Play::Play(QObject *parent)  :
    QObject(parent) ,
    m_audioStream(0),
    m_audioOutputStream (0) ,
    m_audioOutput(0) ,
    m_audioOutputBufferLength(Play::defaultBufferLen),//samples
    m_previousBytePosition(0),
    m_status(PLAY_NOT_INIT),
    m_notifyInterval(Play::defaultNotifyInterval) //ms
{

}

Play::~Play()
{
    resetDevice();
}


//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------
const quint64 Play::streamLength()
{
    if (!m_audioStream.isNull() && !m_audioOutput.isNull())
        return qMax(static_cast<qint64> (0),
                    PlayRecUtils::convertByteToSample(m_audioStream.data()->size()-WAVEFILE_HEADER,m_audioOutput.data()->format()));
    else
        return 0;
}

const quint64 Play::position()
{
    if (!m_audioStream.isNull() && !m_audioOutput.isNull())
        return qMax(static_cast<qint64> (0),
                    PlayRecUtils::convertByteToSample(m_audioStream.data()->pos()-WAVEFILE_HEADER,m_audioOutput.data()->format()));
    else
        return 0;
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
    if (m_audioStream.isNull()) {
        qDebug() << Q_FUNC_INFO << "Notifed without a valid stream";
        return;
    }
    if (m_audioOutput.isNull()) {
        qDebug() << Q_FUNC_INFO << "Notifed without a valid audio output";
        return;
    }
//-----TODO: this is only for debug. IT will be the stream that must provides this info
qint64 _actualPos=m_audioStream.data()->pos();
qint64 _bufLen=_actualPos-m_previousBytePosition;
if (_bufLen==0) {
    qDebug() << Q_FUNC_INFO << " with no audio data to process";
    return;
}
Q_ASSERT(_bufLen>=0);
QByteArray _buffer= m_audioStream.data()->peek(m_audioOutputBufferLength);
const char * _data=_buffer.constData();
PlayRec_structMeter _meter=PlayRecUtils::getAudioPeak(_data,_bufLen,m_audioOutput.data()->format());

quint64 sample=PlayRecUtils::convertByteToSample(m_previousBytePosition,m_audioOutput.data()->format());
//-----TODO END
qDebug() << Q_FUNC_INFO << "Notifed ["<< PlayRecUtils::convertSampleToTime(sample,m_audioOutput.data()->format()) << " sec. ], peak/rms are " << _meter.peak << "/" << _meter.rms << " @ byte=" << m_previousBytePosition <<  "sample=" << sample;
    //emit meter
    emit notifyMeters(sample,_meter);
    //set byte position
    setPreviousBytePosition(_actualPos);
}

void Play::audioStateChanged(QAudio::State state)
{
    if (m_audioOutput.isNull()) Q_ASSERT_X(0, Q_FUNC_INFO, "Audio state changed without a valid reference to m_audioOutput???");
    if (m_audioStream.isNull()) Q_ASSERT_X(0, Q_FUNC_INFO, "Audio state changed without a valid reference to m_audioStream???");

    qDebug() << Q_FUNC_INFO << " Entering, internal status is "
             << Play::internalPlaybackStateToString(m_status)
             << " Audio Output status changed to "
             << PlayRecUtils::qAudioStateToString(state);

    //check error only if audiostream is valid and not at the end of the stream
    if (!m_audioStream->atEnd()) {
        QAudio::Error error = m_audioOutput.data()->error();
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
    }
    //check status consistency
    switch (state) {
        case QAudio::ActiveState:
            setInternalStatus(Play::PLAY_PLAY);
            qDebug() << Q_FUNC_INFO << "QAudio::ActiveState";
            break;
        case QAudio::SuspendedState:
            setInternalStatus(Play::PLAY_SUSPENDED);
            qDebug() << Q_FUNC_INFO << "QAudio::SuspendedState";
            break;
        case QAudio::StoppedState:
            qDebug() << Q_FUNC_INFO << "QAudio::StoppedState";
            stop();
            setInternalStatus(Play::PLAY_READY);
            break;
        case QAudio::IdleState:
            //If at the end of the stream, reset it
            if (m_audioStream && m_audioStream->atEnd()) {
                m_audioStream->reset();
                setPreviousBytePosition(m_audioStream->pos());
                m_audioOutput->reset();
                qDebug() << Q_FUNC_INFO << "Reset Stream";
            }
            qDebug() << Q_FUNC_INFO << "QAudio::IdleState";
            setInternalStatus(Play::PLAY_READY);
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
const PLAYREC_RETVAL Play::init(QIODevice *audioStream, const QAudioDeviceInfo &outputDevice, const QAudioFormat &format)
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    Q_ASSERT_X(m_status==PLAY_NOT_INIT, Q_FUNC_INFO, "Internal status already init");
    Q_ASSERT_X(m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device should be empty");

    //Check and assign format
    QAudioFormat outputFormat=QAudioFormat();
    if (format==outputFormat) //an empty (and invalid) audio format
    {
        outputFormat=outputDevice.nearestFormat(format);
        emit audioFormatChanged(outputFormat);
    }
    else {
        outputFormat=format;
    }

    //opening IO audio output
    m_audioOutput = new QAudioOutput(outputDevice, outputFormat, this);
    m_audioOutputInfo=outputDevice;
    m_audioOutput.data()->setBufferSize(m_audioOutputBufferLength);
    m_audioOutput.data()->setNotifyInterval(m_notifyInterval);

    qDebug() << Q_FUNC_INFO << "Open " << outputDevice.deviceName()
             << " with format: " << PlayRecUtils::formatToString(outputFormat)
             << "Buffer size" << m_notifyInterval << "Samples"
             << "Notify interval " << m_notifyInterval << "ms";

    setAudioStream(audioStream);

    qDebug() << Q_FUNC_INFO << "Audio Output status at init is " << PlayRecUtils::qAudioStateToString(m_audioOutput->state());

    //connect signal from m_audioOutput
    CHECKED_CONNECT(m_audioOutput.data(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));
    CHECKED_CONNECT(m_audioOutput.data(), SIGNAL(notify()), this, SLOT(notified()));

    setInternalStatus(Play::PLAY_READY);
    return  retval;
}

const PLAYREC_RETVAL Play::start()
{
    return start(0);
}

const PLAYREC_RETVAL Play::start(const quint64 startSample)
{
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
            Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init before start execution!!!");
            Q_ASSERT_X(!m_audioStream.isNull(), Q_FUNC_INFO, "Audio stream must be init before start execution!!!");
    //TODO: SKIPPING THE HEADER 48 bytes JUST FOR DEBUG,
    //EVERY STREAM MUST SKIP ITS HEADER. TODO!!!
    qDebug() << Q_FUNC_INFO << "SKIPPING 48 BYTES HEADER FOR DEBUG PURPOSE";
    m_audioStream.data()->read(WAVEFILE_HEADER+startSample);
    setPreviousBytePosition(WAVEFILE_HEADER+startSample);
    //END TODO
            m_audioOutput.data()->start(m_audioStream);
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

const PLAYREC_RETVAL Play::stop()
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready. Can''t stop");
            break;
        case PLAY_SUSPENDED:
        case PLAY_PLAY:
        case PLAY_READY:
            Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init before stopping or unpause!!");
            Q_ASSERT_X(!m_audioStream.isNull(), Q_FUNC_INFO, "Audio stream must be init before stopping or unpause!!!");
            m_audioOutput.data()->stop();
            m_audioStream.data()->reset();
            QCoreApplication::instance()->processEvents();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput.data()->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO <<"Error stopping " << errMsg;
                break;
            }         
            reinit();
            setPreviousBytePosition(0);
            break;
        // case PLAY_READY:
         //   SET_PLAYREC_RETVAL(retval,PLAY_ALREADY_STOPPED,"Internal status not in execution");
         //   break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t stop");
            break;
    }

    return retval;
}

const PLAYREC_RETVAL Play::resetDevice()
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    if (m_status!=PLAY_NOT_INIT) {
        Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init before reset");
        m_audioOutput.data()->reset();
        m_audioOutput.data()->disconnect();
        QCoreApplication::instance()->processEvents();
        delete m_audioOutput;
        m_audioOutput=NULL;
        m_audioOutputInfo=QAudioDeviceInfo();//empty the information associated to the m_audioOutput device
        setInternalStatus(Play::PLAY_NOT_INIT);
        setPreviousBytePosition(0);
    }  else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");
    return retval;
}

const PLAYREC_RETVAL Play::pause()
{
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
            Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init before pause it!!");
            m_audioOutput.data()->suspend();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput.data()->error());
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

const PLAYREC_RETVAL Play::unpause()
{
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
            Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init before unpause it!!!");
            m_audioOutput.data()->resume();
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput.data()->error());
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

const PLAYREC_RETVAL Play::setPosition(const quint64 sample)
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    if (m_status!=PLAY_NOT_INIT) {
        Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init");
        Q_ASSERT_X(!m_audioStream.isNull(), Q_FUNC_INFO, "Audio stream must be init");
        if (m_audioStream.data()->isOpen()) {
            quint64 bytesPosition=qMin(PlayRecUtils::convertSampleToByte(sample,m_audioOutput.data()->format())+WAVEFILE_HEADER,m_audioStream->size() );
            bool retval=m_audioStream.data()->seek(bytesPosition);
            if (!retval)
                qDebug() << Q_FUNC_INFO << "can''t move to sample "<< sample <<" (bytes/size "<< bytesPosition << "/ "<< m_audioStream.data()->size() <<")";
            else setPreviousBytePosition(m_audioStream.data()->pos());
        }
        //TODO set position base on the sample
      //  emit positionChanged(sample);
    }  else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");
    return retval;
}

const PLAYREC_RETVAL Play::changeAudioStream(QIODevice *playbackOutputStream) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    if (m_status==PLAY_NOT_INIT) {
        retval=init(playbackOutputStream);
    } else {
        Q_ASSERT_X(!m_audioOutput.isNull(), Q_FUNC_INFO, "Audio device must be init");
        QAudioDeviceInfo outputDeviceInfo=m_audioOutputInfo;
        QAudioFormat format=m_audioOutput.data()->format();
        retval=stop();
        if (!retval.status) {
            qDebug() << Q_FUNC_INFO << "Stop error " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
            //return ???
        }
        retval=resetDevice();
        if (!retval.status) {
            qDebug() << Q_FUNC_INFO << "Reset error " << PlayRecUtils::playrecReturnValueToString(retval.status);
            //return ???
        }
        retval=init(playbackOutputStream,outputDeviceInfo,format);
    }
    return retval;
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
inline void Play::setInternalStatus(PLAY_STATUS newStatus)
{
    if (newStatus!=m_status) {
        m_status=newStatus;
        emit statusChanged( static_cast<int>(m_status) );
    }
}

inline void Play::setPreviousBytePosition(qint64 byte)
{
    if (m_audioOutput.isNull())
        emit positionChanged(0);
    else
        if (m_previousBytePosition!=static_cast<quint64>(byte) ) {
            m_previousBytePosition=( byte >= 0 ?  byte : 0);
            emit positionChanged(PlayRecUtils::convertByteToSample(m_previousBytePosition,m_audioOutput.data()->format()));
        }
}

inline void Play::setAudioStream(QIODevice* audioStream)
{
    if (m_audioStream!=audioStream)  {
        emit audioStreamChanged();
        m_audioStream=audioStream;
    }
}

const PLAYREC_RETVAL Play::reinit()
{
    qDebug() << Q_FUNC_INFO << "Reinit AudioDevice";
    QIODevice *audioStream=m_audioStream.data();
    QAudioDeviceInfo outputDeviceInfo=m_audioOutputInfo;
    QAudioFormat format=m_audioOutput.data()->format();
    resetDevice();
    return init(audioStream,outputDeviceInfo,format);
}
