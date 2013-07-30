#ifndef PLAYRECUTILS_H
#define PLAYRECUTILS_H

#include "playrec_global.h"


class PlayRecUtils
{
public:
    PlayRecUtils();

    //Convert an error code of the IO device to string
    static QString decodeInternalAudioErrorToString(const QAudio::Error &err);

    //Convert the format to a string
    static QString formatToString(const QAudioFormat &format);

    //Convert the internal Audio IO device status to a string
    static QString qAudioStateToString(const QAudio::State &state);

    //Playrec retval also return a message, this function return the error code as string
    static QString playrecReturnValueToString(int status );
};

#endif // PLAYRECUTILS_H
