#ifndef PLAYREC_H
#define PLAYREC_H

#include "playrec_global.h"
#include "play.h"
#include "rec.h"
#include <QObject>


/**
 * @brief The PlayRec class
 * The class defines an interface for play, rec and play&rec. It hides the below layer of recording and playback
 * providing simple functions to external modules.
 */
class PLAYRECSHARED_EXPORT PlayRec : public QObject
{
    Q_OBJECT
    Q_ENUMS(PlayRecMode)
public:
    PlayRec(QObject* parent=0);
    virtual ~PlayRec();

    //Actual Playback stream lengths, in sample and seconds.
    const quint64 playbackSampleLength() { return m_play->streamLength(); }
    const qreal playbackTimeLength() {return (static_cast<qreal> (m_play->streamLength()))/(static_cast<qreal> (m_play->audioFormat().sampleRate()));}

    //Actual Playback stream position, in sample and seconds.
    const quint64 playbackSamplePosition() {return m_play->position();}
    const qreal playbackTimePosition() {return PlayRecUtils::convertSampleToTime(m_play->position(),m_play->audioFormat());}

    //Actual rec stream position, in sample and seconds.
    const quint64 recSamplePosition() {return 0;} //TODO
    const qreal recTimePosition() {return 0.0;} //TODO

    /**
     * @brief lastOperationMessage Return the message from the last operation. Use joint together with the return bool of most function
     * @return
     */
    const QString lastOperationMessage() {return m_lastMessage;}

    static QMap<QString, QAudioDeviceInfo> availablePlaybackDevices();
    static QMap<QString, QAudioDeviceInfo> availableRecordingDevices();

    enum PlayRecMode {
        PLAY,
        REC,
        PLAYANDREC
    };

public slots:
    /**
     * @brief start Start the playback at sample 0 (header should be skipped)
     * @param sample
     */
    bool start(const quint64 sample=0);

    /**
     * @brief startAtTimePosition convert the time position in samples
     * @param timePosition
     */
    bool startAtTimePosition(const qreal timePosition) {
        quint64 sample=static_cast<quint64> (timePosition*(static_cast<qreal> (m_play->audioFormat().sampleRate())));
        start(sample);
    }

    bool stop();
    bool pause(bool pause);
    bool setPlaybackPosition(quint64 sample); //in sample
    bool setPlaybackPosition(qreal timePosition); //in seconds
    bool setAudioMode(PlayRecMode mode) {}//TODO PROPERTY!!

    //TODO: solo un'idea, intese come property... (QIODevice va cambiato con qualcosa di perszonalizzto tipo generico audiostream,
    // il problema del multicanale.......
    // in realta' la cosa si potrebbe anche gestire con semplici getRecord e getPlayback dove viene passata la classe. ma e' un po' piu' sporca..
    //Set file/generic stream
    bool setPlaybackStream(QIODevice * stream);
    bool setRecordingStream(QIODevice * stream) {return true;}

    //Set the play/rec device, based on the sound card
    bool setPlaybackAudioDevice(const QString& deviceName);
    bool setRecordingAudioDevice(const QString& deviceName) {return true;}

    //Reset, some or all device
    bool resetPlaybackStream();
    bool resetRecStream() {}
    bool resetAllStream() {resetPlaybackStream(); resetRecStream();}

    //questo possono essere considerate proprieta', per cui mancano tutti i get e i notify change

signals:
    void playbackStatusChanged(int status);
    void playbackPositionChanged(qreal timePosition);
    void playbackChanged(QIODevice* stream,QAudioDeviceInfo device,QAudioFormat format);

    //tutti i notify rec and play!!
private:
    Play * m_play;
    Rec * m_rec;
    PlayRecMode m_audioMode;
    QString  m_lastMessage;

    void connectSignals();
    bool setPlaybackAudioDevice(const QAudioDeviceInfo& device);//try to reuse previous format
    bool setPlaybackAudioDevice(const QAudioDeviceInfo& device, const QAudioFormat& format) {return true;}
    bool setRecordingAudioDevice(const QAudioDeviceInfo device) {return true;}//try to reuse previous format
    bool setRecordingAudioDevice(const QAudioDeviceInfo device,const QAudioFormat& format) {return true;}

    //Utilit to use at the end of function
    inline bool  ef(const QString& message, const PLAYREC_RETVAL& retval);

private slots:
    //If playback position changed this slot get the sample and emit playbackPositionChanged in seconds.
    void playbackPositionHasChanged(quint64 sample);

    //new
    void playbackStreamHasChanged(QIODevice* stream);
    void playbackInterfaceHasChanged(QAudioDeviceInfo device);
    void playbackFormatHasChanged(QAudioFormat format);
    void playbackHasBeenReset();
};

#endif // PLAYREC_H
