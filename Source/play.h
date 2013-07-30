#ifndef PLAY_H
#define PLAY_H

#include  "playrec_global.h"
#include "playrecutils.h"

/**
 * @brief The Play class allows to reproduce a byte stream from any QIODevice. This class simplify the manegment of the sound card,
 * is always possible change the sound card without loosing information of the stream and viceversa. Also it's possibile change the format of the output card
 */

class Play : public QObject
{
    Q_OBJECT
    Q_ENUMS(PLAY_STATUS)
    Q_PROPERTY(int status READ status  NOTIFY statusChanged)

public:
    Play(QObject* parent=0);
    virtual ~Play();

    int status() {return m_status;}
    quint64 position() {return 0;} //TODO

    typedef enum {
        PLAY_NOT_INIT = 0,
        PLAY_READY = 1,
        PLAY_PLAY = 2,
        PLAY_SUSPENDED = 3
    } PLAY_STATUS;

    //Convert the internal playback status to a string
    static QString internalPlaybackStateToString(const PLAY_STATUS &state);

    /**
     * @brief defaultNotifyInterval The default interval of play time
     */
    static const int defaultNotifyInterval = 50; //ms
public slots:
    //NOTE Audiostream dev'essere impacchettato in un vettore tipo QVector<QIODevice&>, in quanto devo suonare/registrare un numero multiplo di canali.
    // deve quindi essere modificato anche il numero
    const PLAYREC_RETVAL init(QIODevice *audioStream,  QAudioDeviceInfo  const &outputDevice = QAudioDeviceInfo::defaultOutputDevice(), const QAudioFormat &format = QAudioFormat());
    const PLAYREC_RETVAL resetDevice();
    const PLAYREC_RETVAL start(const quint64 startSample);
    const PLAYREC_RETVAL start();
    const PLAYREC_RETVAL stop();
    const PLAYREC_RETVAL pause();
    const PLAYREC_RETVAL unpause();
    const PLAYREC_RETVAL changeAudioInterface( QAudioDeviceInfo  const &outputDevice = QAudioDeviceInfo::defaultOutputDevice(), const QAudioFormat &format = QAudioFormat()) {return PLAYREC_INIT_RETVAL(PLAY_FUNCTION_NOT_IMPLEMENTED,"NOT YET IMPL");}//TODO
    const PLAYREC_RETVAL changeAudioFormat(const QAudioFormat &format = QAudioFormat()) {return  PLAYREC_INIT_RETVAL(PLAY_FUNCTION_NOT_IMPLEMENTED,"NOT YET IMPL");} //TODO
    const PLAYREC_RETVAL changeStreamOutput(const QIODevice &outputStream) {return  PLAYREC_INIT_RETVAL(PLAY_FUNCTION_NOT_IMPLEMENTED,"NOT YET IMPL");} //TODO
    const PLAYREC_RETVAL setPosition(const quint64 sample);

signals:
    void notifyMeters(quint64 lastSamplePosition, qreal peak, qreal rms);
    void statusChanged(int newStatus);
    void positionChanged(quint64 lastSamplePosition);

private:
    /**
     * @brief m_audioStream the stream (file, etc) that must be reproducted (not owned).
     */
    QIODevice *m_audioStream; // not owned

    /**
     * @brief m_outputStream the pointer to the output stream provided by the sound system
     */
    QIODevice *m_outputStream;

    /**
     * @brief m_audioOutput the output audio device where play the stream
     */
    QAudioOutput *m_audioOutput;

    /**
     * @brief m_outputStreamInfo The Information associated to the m_audioOutput selected
     */
    QAudioDeviceInfo m_outputStreamInfo;

    /**
     * @brief m_status the internal status
     */
    PLAY_STATUS m_status;

    /**
     * @brief m_notifyInterval The notification interval of played stream (see QAudioOutput::setNotifyInterval())
     */
    int m_notifyInterval;

    /**
     * @brief setInternalStatus An utility to set new internal status and emit signals
     * @param newStatus The new status to be set
     */
    void setInternalStatus(PLAY_STATUS newStatus);

    /**
     * @brief reinit reset and reinit with the previous audio device. Used when an internal error to the underlayer (IO AUDIO) happen
     * @return
     */
    const PLAYREC_RETVAL reinit();

private slots:
    void notified();
    void audioStateChanged(QAudio::State state);
};

#endif // PLAY_H
