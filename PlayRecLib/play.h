#ifndef PLAY_H
#define PLAY_H

#include  "playrec_global.h"
#include "playrecutils.h"

//THIS IS AS FIRST DEVEL. THE HEADER WILL BE RETURNED BY THE STREAM ITSELF!!
#define WAVEFILE_HEADER 48 //the number of bytes of a wavfile

/**
 * @brief The Play class allows to reproduce a byte stream from any QIODevice. This class simplify the manegment of the sound card,
 * is always possible change the sound card without loosing information of the stream and viceversa. Also it's possibile change the format of the output card
 */

class Play : public QObject
{
    Q_OBJECT
    Q_ENUMS(PLAY_STATUS)
 //   Q_PROPERTY(int status READ status  NOTIFY statusChanged)
 //   Q_PROPERTY(QAudioFormat audioFormat READ audioFormat WRITE changeAudioFormat NOTIFY audioFormatChanged)

public:
    Play(QObject* parent=0);
    virtual ~Play();

    const int status() {return m_status;}
    const quint64 position();
    const QAudioFormat audioFormat() {return (m_audioOutput ? m_audioOutput->format() : QAudioFormat());}
    const QAudioDeviceInfo audioInterface() {return (m_audioOutput ? m_audioOutputInfo : QAudioDeviceInfo());}
    QIODevice* audioStream() {return m_audioStream;}


    /**
     * @brief streamLength The stream length (if set) in samples
     * @return Number of samples
     */
    const quint64 streamLength();


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
    static const int defaultNotifyInterval = 100; //ms
    static const int defaultBufferLen = 32768; //Samples

public slots:
    //NOTE Audiostream dev'essere impacchettato in un vettore tipo QVector<QIODevice&>, in quanto devo suonare/registrare un numero multiplo di canali.
    // deve quindi essere modificato anche il numero
//    const PLAYREC_RETVAL init(QIODevice *audioStream,  QAudioDeviceInfo  const &outputDevice = QAudioDeviceInfo::defaultOutputDevice(), const QAudioFormat &format = QAudioFormat());

    const PLAYREC_RETVAL resetDevice();
    const PLAYREC_RETVAL start(const quint64 startSample);
    const PLAYREC_RETVAL start();
    const PLAYREC_RETVAL stop();
    const PLAYREC_RETVAL pause();
    const PLAYREC_RETVAL unpause();
    const PLAYREC_RETVAL changeAudioInterface( QAudioDeviceInfo  const &outputDevice = QAudioDeviceInfo::defaultOutputDevice(), const QAudioFormat &format = QAudioFormat());
    const PLAYREC_RETVAL changeAudioFormat(const QAudioFormat &format = QAudioFormat()) {return  PLAYREC_INIT_RETVAL(PLAY_FUNCTION_NOT_IMPLEMENTED,"NOT YET IMPL");} //TODO
    const PLAYREC_RETVAL changeAudioStream(QIODevice *playbackOutputStream);
    const PLAYREC_RETVAL setPosition(const quint64 sample);

signals:
    void notifyMeters(quint64 lastSamplePosition, PlayRec_structMeter meter);
    void statusChanged(int newStatus);
    void positionChanged(quint64 samplePosition);
    void audioReset();
    void audioInterfaceChanged(QAudioDeviceInfo audioDevice);
    void audioFormatChanged(QAudioFormat format);
    void audioStreamChanged(QIODevice * stream);//Some information about the format

private:

    /**
     * @brief m_audioStream the stream (file, etc) that must be reproducted (not owned).
     */
    QIODevice* m_audioStream; // not owned

    /**
     * @brief m_audioOutput the output audio device where play the stream
     */
    QAudioOutput* m_audioOutput;

    /**
     * @brief m_audioOutputInfo The Information associated to the m_audioOutput selected
     */
    QAudioDeviceInfo m_audioOutputInfo;

    /**
     * @brief m_audioOutputBufferLength The length of the audio buffer length in sample (see QAudioOutput::setBufferSize())
     */
    quint64 m_audioOutputBufferLength;

    /**
     * @brief m_previousPosition An indication of the last position read in samples
     */
    quint64 m_previousBytePosition;

    /**
     * @brief m_status the internal status
     */
    PLAY_STATUS m_status;

    /**
     * @brief m_notifyInterval The notification interval of played stream in ms (see QAudioOutput::setNotifyInterval())
     */
    int m_notifyInterval;

    /**
     * @brief initDevice Init a device with some format or the default device if any values is passed to the function
     * @param outputDevice
     * @param format
     * @return
     */
    const PLAYREC_RETVAL initDevice(QAudioDeviceInfo  const &outputDevice = QAudioDeviceInfo::defaultOutputDevice(), const QAudioFormat &format = QAudioFormat());


    /**
     * @brief setInternalStatus An utility to set new internal status and emit signals
     * @param newStatus The new status to be set
     */
    void setInternalStatus(PLAY_STATUS newStatus);

    /**
     * @brief Play::setAudioStream An utility to set a new audio stream, emit signals
     * @param audioStream
     */
    inline void setAudioStream(QIODevice* audioStream);

    /**
     * @brief setPreviousBytePosition An utility to set the variable m_previousBytePosition, it emits also signal that the position has been changed.
     * @param byte The position in byte
     */
    void setPreviousBytePosition(qint64 byte);

    /**
     * @brief reinit reset and reinit with the previous audio device. Used when an internal error to the underlayer (IO AUDIO) happen
     * @return
     */
    const PLAYREC_RETVAL reinitDevice();

private slots:
    void notified();
    void audioStateChanged(QAudio::State state);
};

#endif // PLAY_H
