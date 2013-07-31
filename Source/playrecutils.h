#ifndef PLAYRECUTILS_H
#define PLAYRECUTILS_H

#include "playrec_global.h"
#include <QtEndian>
#include <qmath.h>

class PlayRecUtils
{
public:
    PlayRecUtils();

    //Convert an error code of the IO device to string
    static const QString decodeInternalAudioErrorToString(const QAudio::Error &err);

    //Convert the format to a string
    static const QString formatToString(const QAudioFormat &format);

    //Convert the internal Audio IO device status to a string
    static const QString qAudioStateToString(const QAudio::State &state);

    //Playrec retval also return a message, this function return the error code as string
    static const QString playrecReturnValueToString(const int status );

    //Calculate the audio peak and rms from raw data. It recognizes the format (ie number of channels) and gives the maximum between all channels
    static const PlayRec_structMeter getAudioPeak(const char *data, const qint64 &length, const QAudioFormat &format );

    //Return the number of bytes respect to  sample and the format
    static const qint64 convertSampleToByte(const quint64& sample,const QAudioFormat& format);

    //Return the number of bytes respect to  sample and the format
    static const qint64 convertByteToSample(const qint64 &byte, const QAudioFormat& format);

    //Return the time of this sample respect to the sample and the format
    static const qreal convertSampleToTime(const quint64& sample,const QAudioFormat& format);
};

#endif // PLAYRECUTILS_H
