#include "play.h"

class DummyStream : public QIODevice {

    virtual qint64  writeData(const char *data, qint64 len) {
        return 0;
    }

    virtual qint64 readData(char *data, qint64 maxlen) {
        return 0;
    }

};

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------
Play::Play(QObject *parent)  :
    QObject(parent) ,
    m_audioStream(0),
    m_audioOutput(0) ,
    m_audioOutputInfo(),
    m_audioOutputBufferLength(Play::defaultBufferLen),//samples
    m_previousBytePosition(0),
    m_status(PLAY_NOT_INIT),
    m_notifyInterval(Play::defaultNotifyInterval) //ms
{
    initDevice();
}

Play::~Play()
{
 //   resetDevice();
}

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------
const quint64 Play::streamLength()
{
    if (m_audioStream && m_audioOutput)
        return qMax(static_cast<qint64> (0),
                    PlayRecUtils::convertByteToSample(m_audioStream->size()-WAVEFILE_HEADER,m_audioOutput->format()));
    else
        return 0;
}

const quint64 Play::position()
{
    if (m_audioStream && m_audioOutput)
        return qMax(static_cast<qint64> (0),
                    PlayRecUtils::convertByteToSample(m_audioStream->pos()-WAVEFILE_HEADER,m_audioOutput->format()));
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
    if (!m_audioStream) {
        qDebug() << Q_FUNC_INFO << "Notifed without a valid stream";
        return;
    }
    if (!m_audioOutput) {
        qDebug() << Q_FUNC_INFO << "Notifed without a valid audio output";
        return;
    }
//-----TODO: this is only for debug. IT will be the stream that must provides this info
qint64 _actualPos=m_audioStream->pos();
qint64 _bufLen=_actualPos-m_previousBytePosition;
if (_bufLen==0) {
    qDebug() << Q_FUNC_INFO << " with no audio data to process";
    return;
}
Q_ASSERT(_bufLen>=0);
QByteArray _buffer= m_audioStream->peek(m_audioOutputBufferLength);
const char * _data=_buffer.constData();
PlayRec_structMeter _meter=PlayRecUtils::getAudioPeak(_data,_bufLen,m_audioOutput->format());

quint64 sample=PlayRecUtils::convertByteToSample(m_previousBytePosition,m_audioOutput->format());
//-----TODO END
qDebug() << Q_FUNC_INFO << "Notifed ["<< PlayRecUtils::convertSampleToTime(sample,m_audioOutput->format()) << " sec. ], peak/rms are " << _meter.peak << "/" << _meter.rms << " @ byte=" << m_previousBytePosition <<  "sample=" << sample;
    //emit meter
    emit notifyMeters(sample,_meter);
    //set byte position
    setPreviousBytePosition(_actualPos);
}

void Play::audioStateChanged(QAudio::State state)
{
    Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio state changed without a valid reference to m_audioOutput???");
    Q_ASSERT_X(m_audioStream, Q_FUNC_INFO, "Audio state changed without a valid reference to m_audioStream???");

    qDebug() << Q_FUNC_INFO << " Entering, internal status is "
             << Play::internalPlaybackStateToString(m_status)
             << " Audio Output status changed to "
             << PlayRecUtils::qAudioStateToString(state);

    //check error only if audiostream is valid and not at the end of the stream
    if (!m_audioStream->atEnd()) {
        QAudio::Error error = m_audioOutput->error();
        switch (error) {
            case QAudio::NoError:
                break;
            case QAudio::OpenError:
            case QAudio::IOError:
            case QAudio::UnderrunError:
            case QAudio::FatalError:
                qDebug() << Q_FUNC_INFO << "AudioOutput error: " << PlayRecUtils::decodeInternalAudioErrorToString(error);
                reinitDevice();
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
//const PLAYREC_RETVAL Play::init(QIODevice *audioStream, const QAudioDeviceInfo &outputDevice, const QAudioFormat &format)
//{
//    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

//    retval=initDevice(outputDevice,&format);
//    if (retval.status!=PLAY_OK) {
//        qDebug() << Q_FUNC_INFO << "Can't init device: " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
//        return retval;
//    }

//    setAudioStream(audioStream);

//    qDebug() << Q_FUNC_INFO << "Audio Output status at init is " << PlayRecUtils::qAudioStateToString(m_audioOutput->state());

//    setInternalStatus(Play::PLAY_READY);
//    return  retval;
//}

const PLAYREC_RETVAL Play::start()
{
    return start(0);
}

const PLAYREC_RETVAL Play::start(const quint64 startSample)
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    if (!m_audioStream) {
        SET_PLAYREC_RETVAL(retval,PLAY_STREAM_NOT_INIT,"Audio stream not init, can't play");
        return retval;
    }

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
    quint64 bytesPosition=qMin(PlayRecUtils::convertSampleToByte(startSample,m_audioOutput->format())+WAVEFILE_HEADER,m_audioStream->size() );
    m_audioStream->read(bytesPosition);
    setPreviousBytePosition(bytesPosition);
    //END TODO
            m_audioOutput->start(m_audioStream);
            errMsg=PlayRecUtils::decodeInternalAudioErrorToString(m_audioOutput->error());
            if (!errMsg.isEmpty()) {
                qDebug() << Q_FUNC_INFO << errMsg;
                break;
            }
            //setPosition(startSample);

            break;
        default:
            SET_PLAYREC_RETVAL(retval,PLAY_FAIL,"Unknown status, can''t start");
            break;
    }

    return retval;
}

const PLAYREC_RETVAL Play::stop()
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    if (!m_audioStream) {
        SET_PLAYREC_RETVAL(retval,PLAY_STREAM_NOT_INIT,"Audio stream not init, can't stop");
        return retval;
    }

    QString  errMsg="";
    switch (m_status) {
        case PLAY_NOT_INIT:
            SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready. Can''t stop");
            break;
        case PLAY_SUSPENDED:
        case PLAY_PLAY:
        case PLAY_READY:
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
            reinitDevice();
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

    if (m_audioOutput) {
        m_audioOutput->disconnect();//avoid sending signal to the this state machine
        m_audioOutput->reset();
        QCoreApplication::instance()->processEvents();
    }
    delete m_audioOutput;
    m_audioOutput=NULL;
    m_audioOutputInfo=QAudioDeviceInfo();//empty the information associated to the m_audioOutput device
    setPreviousBytePosition(0);
    if (m_audioStream && m_audioStream->isOpen()) m_audioStream->reset();//reset the audio stream
    setInternalStatus(Play::PLAY_NOT_INIT);
    emit audioReset();
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

const PLAYREC_RETVAL Play::setPosition(const quint64 sample)
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();
    if (!m_audioStream) {
        SET_PLAYREC_RETVAL(retval,PLAY_STREAM_NOT_INIT,"Audio stream not init, set position");
        return retval;
    }

    if (m_status!=PLAY_NOT_INIT) {
        Q_ASSERT_X(m_audioOutput, Q_FUNC_INFO, "Audio device must be init");
        Q_ASSERT_X(m_audioStream, Q_FUNC_INFO, "Audio stream must be init");
        if (m_audioStream->isOpen()) {
            quint64 bytesPosition=qMin(PlayRecUtils::convertSampleToByte(sample,m_audioOutput->format())+WAVEFILE_HEADER,m_audioStream->size() );
            bool retval=m_audioStream->seek(bytesPosition);
            if (!retval)
                qDebug() << Q_FUNC_INFO << "can''t move to sample "<< sample <<" (bytes/size "<< bytesPosition << "/ "<< m_audioStream->size() <<")";
            else setPreviousBytePosition(m_audioStream->pos());
        }
        //TODO set position base on the sample
      //  emit positionChanged(sample);
    }  else  SET_PLAYREC_RETVAL(retval,PLAY_AUDIO_OUTPUT_NOT_READY,"Internal status not ready");
    return retval;
}

const PLAYREC_RETVAL Play::changeAudioStream(QIODevice *playbackOutputStream) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    if (m_status==PLAY_NOT_INIT) {
        retval=initDevice();
        if (retval.status!=PLAY_OK) {
            qDebug() << Q_FUNC_INFO << "Can't init device: " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
            return retval;
        }
        retval=initDevice();
    } else {
        retval=reinitDevice();
        if (retval.status!=PLAY_OK) {
            qDebug() << Q_FUNC_INFO << "Can't init device: " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
            return retval;
        }
    }
    setAudioStream(playbackOutputStream);
    return retval;
}

const PLAYREC_RETVAL Play::changeAudioInterface(const QAudioDeviceInfo &outputDevice, const QAudioFormat &format) {
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    retval=resetDevice();
    if (retval.status!=PLAY_OK) {
        qDebug() << Q_FUNC_INFO << "Can't reset device: " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
        return retval;
    }
    retval=initDevice(outputDevice,format);
    if (retval.status!=PLAY_OK) {
        qDebug() << Q_FUNC_INFO << "Can't init device: " << PlayRecUtils::playrecReturnValueToString(retval.status) ;
        return retval;
    }
    return retval;
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

const PLAYREC_RETVAL Play::initDevice( const QAudioDeviceInfo &outputDevice, const QAudioFormat &format)
{
    PLAYREC_RETVAL retval=PLAYREC_INIT_OK_RETVAL();

    QAudioFormat _previousFormat;

    //There's a previous valid format?
    if (m_audioOutput && m_audioOutput->format().isValid())
        _previousFormat=m_audioOutput->format();

    QAudioFormat _definitiveFormat=outputDevice.nearestFormat(format);

    if (_definitiveFormat!=format || _definitiveFormat!=_previousFormat) {
         qDebug() << Q_FUNC_INFO << "Changing audio format from " << PlayRecUtils::formatToString(_previousFormat)
                  << " to candidate " << PlayRecUtils::formatToString(format)
                  << " finally " <<  PlayRecUtils::formatToString(_definitiveFormat) ;
        emit audioFormatChanged(_definitiveFormat);
    }


    //opening IO audio output
    m_audioOutput = new QAudioOutput(outputDevice, _definitiveFormat, this);
    m_audioOutputInfo=outputDevice;
    if (!m_audioOutput) {
        setInternalStatus(Play::PLAY_NOT_INIT);
        SET_PLAYREC_RETVAL(retval,PLAY_CANT_INIT_AUDIO,QString("Can''t open device %1").arg(outputDevice.deviceName()));
        return retval;
    }
    if (m_audioOutput->error()!=QAudio::NoError) {
        qDebug() <<  Q_FUNC_INFO << "Error opening device " << outputDevice.deviceName() << " error: " << PlayRecUtils::decodeInternalAudioErrorToString( m_audioOutput->error());
        SET_PLAYREC_RETVAL(retval,PLAY_CANT_INIT_AUDIO,QString("Error opening device %1 Error: %2").arg(PlayRecUtils::decodeInternalAudioErrorToString( m_audioOutput->error())).arg(outputDevice.deviceName()));
        delete m_audioOutput;//IF THE AUDIO OUTPUT IS AN INPUT TYPE HERE AN SEGFAULT IS GETTED. THIS LOOKS A BUG OF QUADIOINPUT/OUTPUT CLASS
        m_audioOutput=NULL;
        resetDevice();
        return retval;
    }
    m_audioOutput->reset();

    m_audioOutput->setBufferSize(m_audioOutputBufferLength);
    m_audioOutput->setNotifyInterval(m_notifyInterval);


    emit audioInterfaceChanged(m_audioOutputInfo);

    //connect signal from m_audioOutput
    CHECKED_CONNECT(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));
    CHECKED_CONNECT(m_audioOutput, SIGNAL(notify()), this, SLOT(notified()));

    qDebug() << Q_FUNC_INFO << "Open " << m_audioOutputInfo.deviceName()
             << " with format: " << PlayRecUtils::formatToString(m_audioOutput->format())
             << "Buffer size" << m_audioOutput->bufferSize() << "Samples"
             << "Notify interval " << m_audioOutput->notifyInterval() << "ms";


    setInternalStatus(Play::PLAY_READY);
    return  retval;
}

inline void Play::setInternalStatus(PLAY_STATUS newStatus)
{
    if (newStatus!=m_status) {
        m_status=newStatus;
        emit statusChanged( static_cast<int>(m_status) );
    }
}

inline void Play::setPreviousBytePosition(qint64 byte)
{
    if (!m_audioOutput)
        emit positionChanged(0);
    else
        if (m_previousBytePosition!=static_cast<quint64>(byte) ) {
            m_previousBytePosition=( byte >= 0 ?  byte : 0);
            emit positionChanged(PlayRecUtils::convertByteToSample(m_previousBytePosition,m_audioOutput->format()));
        }
}

inline void Play::setAudioStream(QIODevice* audioStream)
{
    if (m_audioStream!=audioStream)  {
        m_audioStream=audioStream;
        emit audioStreamChanged(m_audioStream);
    }
}

const PLAYREC_RETVAL Play::reinitDevice()
{
    qDebug() << Q_FUNC_INFO << "Reinit AudioDevice";
    QAudioDeviceInfo outputDeviceInfo=m_audioOutputInfo;
    QAudioFormat format=m_audioOutput->format();
    resetDevice();
    return initDevice(outputDeviceInfo,format);
}
