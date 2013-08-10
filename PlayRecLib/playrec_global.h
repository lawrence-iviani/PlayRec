#ifndef PLAYREC_GLOBAL_H
#define PLAYREC_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QCoreApplication>
#include <QDebug>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QAudioOutput>
#include <QMap>
#include <QPointer>

#if defined(PLAYREC_LIBRARY)
#  define PLAYRECSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PLAYRECSHARED_EXPORT Q_DECL_IMPORT
#endif


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Macro which connects a signal to a slot, and which causes application to
// abort if the connection fails.  This is intended to catch programming errors
// such as mis-typing a signal or slot name.  It is necessary to write our own
// macro to do this - the following idiom
//     Q_ASSERT(connect(source, signal, receiver, slot));
// will not work because Q_ASSERT compiles to a no-op in release builds.

#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if(!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

//-----------------------------------------------------------------------------
// Generic meter struct
//-----------------------------------------------------------------------------
typedef struct {
    qreal peak;
    qreal rms;
} PlayRec_structMeter;


//-----------------------------------------------------------------------------
// Return Value Stuff
//-----------------------------------------------------------------------------

// Define a generic error number and message error generated by an internal function
typedef struct {
    qint16 status;
    QString message;
} PLAYREC_RETVAL;

// The enum of error types that a PLAY class can return
typedef enum {
    PLAY_OK = 1,
    PLAY_FAIL = -1,
    PLAY_AUDIO_OUTPUT_NOT_READY= -2,
    PLAY_FUNCTION_NOT_IMPLEMENTED= -3,
    PLAY_ALREADY_PAUSED = -4,
    PLAY_ALREADY_PLAYING = -5,
    PLAY_ALREADY_STOPPED = -6,
    PLAY_CANT_INIT_AUDIO = -7,
    PLAY_CANT_INIT_STREAM = -8,
    PLAY_STREAM_NOT_INIT = -9
    //other, can't change interface, streamerror ecc
} PLAYREC_RETURNVALUE;

//Set a retval
inline void SET_PLAYREC_RETVAL(PLAYREC_RETVAL& retval,qint16 status, QString msg) {
    retval.status=status;
    retval.message=msg;
}

//Create and set a retval
inline PLAYREC_RETVAL PLAYREC_INIT_RETVAL(qint16 status, QString msg) {
    PLAYREC_RETVAL retval;
    retval.status=status;
    retval.message=msg;
    return retval;
}

//Create and set a retval with OK status
inline PLAYREC_RETVAL PLAYREC_INIT_OK_RETVAL() {
    return PLAYREC_INIT_RETVAL(PLAY_OK,"");
}

#endif // PLAYREC_GLOBAL_H
