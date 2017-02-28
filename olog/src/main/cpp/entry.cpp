#include "log.h"

// index与上面的对应
static const int LogCatPriority[] = {
        ANDROID_LOG_VERBOSE,
        ANDROID_LOG_DEBUG,
        ANDROID_LOG_INFO,
        ANDROID_LOG_WARN,
        ANDROID_LOG_ERROR};

// index与上面的对应
static const char LogLevel[] = {
        'V',
        'D',
        'I',
        'W',
        'E'
};

static int gLogLevel = LEVEL_INFO;
static bool gLogToLogCat = false;

OLog *gLogInstance = NULL;

extern "C"
JNIEXPORT void JNICALL Java_com_winom_olog_LogEntry_setLogProperty(JNIEnv *env,
                                                                   jclass __unused clazz,
                                                                   jint logLevel,
                                                                   jboolean toLogCat) {
    gLogLevel = logLevel;
    gLogToLogCat = toLogCat;
}

extern "C"
JNIEXPORT void JNICALL Java_com_winom_olog_LogEntry_logInit(JNIEnv *env,
                                                            jclass __unused clazz,
                                                            jstring jLogDir,
                                                            jstring jLogFilePrefix,
                                                            jstring jLogFilePostfix,
                                                            jbyteArray jencode) {
    if (gLogInstance) {
        return;
    }

    const char *logDir = env->GetStringUTFChars(jLogDir, NULL);
    const char *logFilePrefix = env->GetStringUTFChars(jLogFilePrefix, NULL);
    const char *logFilePostfix = env->GetStringUTFChars(jLogFilePostfix, NULL);
    jbyte *encodeArrPtr = env->GetByteArrayElements(jencode, NULL);
    jsize lengthOfArray = env->GetArrayLength(jencode);
    if (lengthOfArray > LOG_ENCODE_LENGTH) {
        lengthOfArray = LOG_ENCODE_LENGTH;
    }

    gLogInstance = new OLog();
    gLogInstance->logInit(logDir, logFilePrefix, logFilePostfix, (const char *) encodeArrPtr,
                          (size_t) lengthOfArray);

    env->ReleaseStringUTFChars(jLogDir, logDir);
    env->ReleaseStringUTFChars(jLogFilePrefix, logFilePrefix);
    env->ReleaseStringUTFChars(jLogFilePostfix, logFilePostfix);
    env->ReleaseByteArrayElements(jencode, encodeArrPtr, JNI_ABORT);
}

extern "C"
JNIEXPORT void JNICALL Java_com_winom_olog_LogEntry_logWrite(JNIEnv *env,
                                                             jclass __unused clazz,
                                                             jstring jlog) {
    const char *log = env->GetStringUTFChars(jlog, NULL);
    gLogInstance->logWrite(log);
    gLogInstance->logWrite("\n");

    env->ReleaseStringUTFChars(jlog, log);
}

extern "C"
JNIEXPORT void JNICALL Java_com_winom_olog_LogEntry_logUninit(JNIEnv __unused *env,
                                                              jclass __unused clazz) {
    if (!gLogInstance) {
        gLogInstance->logUninit();
        delete gLogInstance;
        gLogInstance = NULL;
    }
}

void logPrint(int level, const char *tag, const char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    logVPrint(level, tag, fmt, arg);
    va_end(arg);
}

void logVPrint(int level, const char *tag, const char *fmt, va_list args) {
    if (gLogInstance && level >= gLogLevel) {
        gLogInstance->log_vprint(LogLevel[level], tag, fmt, args);

        if (gLogToLogCat) {
            __android_log_vprint(LogCatPriority[level], tag, fmt, args);
        }
    }
}