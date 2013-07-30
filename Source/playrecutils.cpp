#include "playrecutils.h"

PlayRecUtils::PlayRecUtils()
{
}

QString PlayRecUtils::decodeInternalAudioErrorToString(const QAudio::Error &err) {
    QString retval="";
    switch (err) {
        case QAudio::NoError:
            break;
        case QAudio::OpenError:
            retval="OpenError: An error opening the audio device";
            break;
        case QAudio::IOError:
            retval="IOError: An error occurred during read/write of audio device";
            break;
        case QAudio::UnderrunError:
            retval="UnderrunError: Audio data is not being fed to the audio device at a fast enough rate";
            break;
        case QAudio::FatalError:
            retval="FatalError: A non-recoverable error has occurred, the audio device is not usable at this time";
            break;
        default:
            retval="Unknown code error";
            break;
    }
    return retval;
}

QString PlayRecUtils::qAudioStateToString(const QAudio::State &state) {
    QString retval="";
    switch (state) {
        case QAudio::ActiveState:
            retval="Active State";
            break;
        case QAudio::SuspendedState:
            retval="Suspended State";
            break;
        case QAudio::StoppedState:
            retval="Stopped State";
            break;
        case QAudio::IdleState:
            retval="Idle State";
            break;
        default:
            retval="Unknown state";
            break;
    }
    return retval;
}

QString PlayRecUtils::playrecReturnValueToString(int status ) {
    QString retval ="";
    switch (status) {
    case PLAY_OK:
        retval="PLAY OK";
        break;
    case PLAY_FAIL:
        retval="PLAY_FAIL";
        break;
    case PLAY_AUDIO_OUTPUT_NOT_READY:
        retval="PLAY_AUDIO_OUTPUT_NOT_READY";
        break;
    case PLAY_FUNCTION_NOT_IMPLEMENTED:
        retval="PLAY_FUNCTION_NOT_IMPLEMENTED";
        break;
    case PLAY_ALREADY_PAUSED:
        retval="PLAY_ALREADY_PAUSED";
        break;
    case PLAY_ALREADY_PLAYING:
        retval="PLAY_ALREADY_PLAYING";
        break;
    case PLAY_ALREADY_STOPPED:
        retval="PLAY_ALREADY_STOPPED";
        break;
    default:
        retval="UNKNOWN CODE";
        break;
    }
    return retval;
}

QString PlayRecUtils::formatToString(const QAudioFormat &format)
{
    QString result;

    if (QAudioFormat() != format) {
        if (format.codec() == "audio/pcm") {
            Q_ASSERT(format.sampleType() == QAudioFormat::SignedInt);

            const QString formatEndian = (format.byteOrder() == QAudioFormat::LittleEndian)
                ?   QString("LE") : QString("BE");

            QString formatType;
            switch(format.sampleType()) {
            case QAudioFormat::SignedInt:
                formatType = "signed";
                break;
            case QAudioFormat::UnSignedInt:
                formatType = "unsigned";
                break;
            case QAudioFormat::Float:
                formatType = "float";
                break;
            case QAudioFormat::Unknown:
                formatType = "unknown";
                break;
            }

            QString formatChannels = QString("%1 channels").arg(format.channels());
            switch (format.channels()) {
            case 1:
                formatChannels = "mono";
                break;
            case 2:
                formatChannels = "stereo";
                break;
            }

            result = QString("%1 Hz %2 bit %3 %4 %5")
                .arg(format.frequency())
                .arg(format.sampleSize())
                .arg(formatType)
                .arg(formatEndian)
                .arg(formatChannels);
        } else {
            result = format.codec();
        }
    }

    return result;
}
