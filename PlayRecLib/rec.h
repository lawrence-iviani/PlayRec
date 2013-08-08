#ifndef REC_H
#define REC_H

#include  "playrec_global.h"
#include  <QObject>

class Rec : public QObject
{
    Q_OBJECT
public:
    Rec(QObject * parent);
};

#endif // REC_H
