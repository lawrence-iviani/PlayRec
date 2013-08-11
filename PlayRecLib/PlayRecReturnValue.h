#ifndef PLAYRECRETURNVALUE_H
#define PLAYRECRETURNVALUE_H

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//Format a playrec retval message to  string
#define PLAYREC_MESSAGE(result) QString("%1: --%2--").arg(PlayRecUtils::playrecReturnValueToString(result.status)).arg(result.message)

//Define the OK value, should be used by any enumerator
#define PLAYREC_RETVAL_OK 1

//-----------------------------------------------------------------------------
// Return Value Stuff
//-----------------------------------------------------------------------------



/**
 * A structucre with the return value a class
 */
typedef struct {
    qint16 status;
    QString message;
} PLAYREC_RETVAL;


/**
 * @brief SET_PLAYREC_RETVAL This inline function set a plyrec struct with a code and a message error
 * @param retval
 * @param status
 * @param msg
 */
inline void SET_PLAYREC_RETVAL(PLAYREC_RETVAL& retval,qint16 status, QString msg) {
    retval.status=status;
    retval.message=msg;
}

/**
 * @brief PLAYREC_INIT_RETVAL This inline function return an empty playrec sturcture
 * @param status
 * @param msg
 * @return
 */
inline PLAYREC_RETVAL PLAYREC_INIT_RETVAL(qint16 status, QString msg) {
    PLAYREC_RETVAL retval;
    retval.status=status;
    retval.message=msg;
    return retval;
}

/**
 * @brief PLAYREC_INIT_OK_RETVAL This inline function set a playrec struct with an ok status
 * @return
 */
inline PLAYREC_RETVAL PLAYREC_INIT_OK_RETVAL() {
    return PLAYREC_INIT_RETVAL(PLAYREC_RETVAL_OK,"");
}



#endif // PLAYRECRETURNVALUE_H
