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
    void start(const quint64 sample=0);

    /**
     * @brief startAtTimePosition convert the time position in samples
     * @param timePosition
     */
    void startAtTimePosition(const qreal timePosition) {
        quint64 sample=static_cast<quint64> (timePosition*(static_cast<qreal> (m_play->audioFormat().sampleRate())));
        start(sample);
    }

    void stop();
    void pause(bool pause);
    void setPlaybackPosition(quint64 sample); //in sample
    void setPlaybackPosition(qreal timePosition); //in seconds
    void setAudioMode(PlayRecMode mode) {}//TODO PROPERTY!!

    //TODO: solo un'idea, intese come property... (QIODevice va cambiato con qualcosa di perszonalizzto tipo generico audiostream,
    // il problema del multicanale.......
    // in realta' la cosa si potrebbe anche gestire con semplici getRecord e getPlayback dove viene passata la classe. ma e' un po' piu' sporca..
    //Set file/generic stream
    void setPlaybackStream(QIODevice * stream);
    void setRecordingStream(QIODevice * stream) {}

    //Set the play/rec device, based on the sound card
    void setPlaybackAudioDevice(const QString& deviceName);
    void setRecordingAudioDevice(const QString& deviceName) {}

    //Reset, some or all device
    void resetPlaybackStream();
    void resetRecStream() {}
    void resetAllStream() {resetPlaybackStream(); resetRecStream();}

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

    void connectSignals();
    void setPlaybackAudioDevice(const QAudioDeviceInfo& device);//try to reuse previous format
    void setPlaybackAudioDevice(const QAudioDeviceInfo& device, const QAudioFormat& format) {}
    void setRecordingAudioDevice(const QAudioDeviceInfo device) {}//try to reuse previous format
    void setRecordingAudioDevice(const QAudioDeviceInfo device,const QAudioFormat& format) {}

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
