#include "playrecutils.h"

PlayRecUtils::PlayRecUtils()
{
}

const QString PlayRecUtils::decodeInternalAudioErrorToString(const QAudio::Error &err) {
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

const QString PlayRecUtils::qAudioStateToString(const QAudio::State &state) {
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

const QString PlayRecUtils::playrecReturnValueToString(const int status ) {
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

const QString PlayRecUtils::formatToString(const QAudioFormat &format)
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

const PlayRec_structMeter PlayRecUtils::getAudioPeak(const char *data, const qint64 &length, const QAudioFormat &format ) {

    PlayRec_structMeter retval;
    retval.peak=0;
    retval.rms=0;
    int _maxAmplitude;
    quint16 _maxValue=0;
    qreal _rmsValue=0;

    switch (format.sampleSize()) {
        case 8:
            switch (format.sampleType()) {
            case QAudioFormat::UnSignedInt:
                _maxAmplitude = 255;
                break;
            case QAudioFormat::SignedInt:
                _maxAmplitude = 127;
                break;
            default:
                break;
            }
            break;
        case 16:
            switch (format.sampleType()) {
            case QAudioFormat::UnSignedInt:
                _maxAmplitude = 65535;
                break;
            case QAudioFormat::SignedInt:
                _maxAmplitude = 32767;
                break;
            default:
                qWarning() << Q_FUNC_INFO << formatToString(format)<< " format not supported";
                break;
            }
            break;
        default:
            break;
    }

    Q_ASSERT(format.sampleSize() % 8 == 0);
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channels() * channelBytes;
    Q_ASSERT(length % sampleBytes == 0);
    const int numSamples = length / sampleBytes;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

    for (int i = 0; i < numSamples; ++i) {
        for(int j = 0; j < format.channels(); ++j) {
            quint16 _value = 0;
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                _value = *reinterpret_cast<const quint8*>(ptr);
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                _value = qAbs(*reinterpret_cast<const qint8*>(ptr));
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    _value = qFromLittleEndian<quint16>(ptr);
                else
                    _value = qFromBigEndian<quint16>(ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    _value = qAbs(qFromLittleEndian<qint16>(ptr));
                else
                    _value = qAbs(qFromBigEndian<qint16>(ptr));
            }

            _maxValue = qMax(_value, _maxValue);
            _rmsValue+=qPow( qreal(_value) / qreal(_maxAmplitude) ,2);
            ptr += channelBytes;
        }
    }
    retval.rms=qSqrt(_rmsValue/qreal(numSamples*format.channels()));
    retval.peak=qreal(_maxValue) / qreal(_maxAmplitude);
    return retval;
}

const qint64 PlayRecUtils::convertSampleToByte(const quint64 &sample, const QAudioFormat &format)
{
    return static_cast<qint64> (format.channels()*(format.sampleSize()/8)*sample);
}

const qint64 PlayRecUtils::convertByteToSample(const qint64 &byte, const QAudioFormat &format)
{
    return static_cast<quint64> (  byte/static_cast<qreal>((format.channels()*(format.sampleSize()/8.0))) );
}

const qreal PlayRecUtils::convertSampleToTime(const quint64& sample,const QAudioFormat& format)
{
    return (static_cast<qreal> (sample))/(static_cast<qreal> (format.sampleRate()));
}

const quint64 PlayRecUtils::convertTimeToSample(const qreal &time, const QAudioFormat &format)
{
    return (static_cast<quint64> (time*(static_cast<qreal> (format.sampleRate()))));
}
