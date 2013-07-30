#ifndef PLAYREC_H
#define PLAYREC_H

#include "playrec_global.h"
#include "play.h"
#include "rec.h"
#include <QObject>

class PLAYRECSHARED_EXPORT PlayRec : public QObject
{
    Q_OBJECT
    Q_ENUMS(PlayRecMode)
public:
    PlayRec(QObject* parent=0);
    virtual ~PlayRec();

    static QMap<QString, QAudioDeviceInfo> availablePlaybackDevices();
    static QMap<QString, QAudioDeviceInfo> availableRecordingDevices();

    enum PlayRecMode {
        PLAY,
        REC,
        PLAYANDREC
    };

public slots:
    void start();
    void stop();
    void pause(bool pause);
    void setPlaybackPosition(quint64 sample) {}//TODO
    void setAudioMode(PlayRecMode mode) {}//TODO PROPERTY!!

    //TODO: solo un'idea, intese come property... (QIODevice va cambiato con qualcosa di perszonalizzto tipo generico audiostream,
    // il problema del multicanale.......
    // in realta' la cosa si potrebbe anche gestire con semplici getRecord e getPlayback dove viene passata la classe. ma e' un po' piu' sporca..
    //Set file/generic stream
    void setPlaybackStream(QIODevice * stream);
    void setRecordingStream(QIODevice * stream) {}

    //Set the play/rec device, based on the sound card
    void setPlaybackAudioDevice(QAudioDeviceInfo device) {}
    void setRecordingAudioDevice(QAudioDeviceInfo device) {}

    //Reset, some or all device
    void resetPlaybackStream();
    void resetRecStream() {}
    void resetAllStream() {resetPlaybackStream(); resetRecStream();}

    //questo possono essere considerate proprieta', per cui mancano tutti i get e i notify change

signals:
    void playbackStatusChanged(int status);
    //tutti i notify rec and play!!
private:
    Play * m_play;
    Rec * m_rec;
    PlayRecMode m_audioMode;
};

#endif // PLAYREC_H
