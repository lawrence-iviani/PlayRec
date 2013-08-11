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
// Generic meter struct
//-----------------------------------------------------------------------------
typedef struct {
    qreal peak;
    qreal rms;
} PlayRec_structMeter;



#endif // PLAYREC_GLOBAL_H
