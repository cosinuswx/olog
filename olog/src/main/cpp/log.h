#include <jni.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

#define LOG_TAG    "olog"

/* 密码的长度 */
#define LOG_ENCODE_LENGTH   32
#define WRITE_BUF_LEN       1024        // 这个必须是LOG_ENCODE_LENGTH的倍数，因为写入的时候加密时，直接从0开始的
#define LOG_BUF_SIZE        1024

// 这些常量和Log.java里的对应，不能变动
static const int LEVEL_VERBOSE = 0;
static const int LEVEL_DEBUG   = 1;
static const int LEVEL_INFO    = 2;
static const int LEVEL_WARNING = 3;
static const int LEVEL_ERROR   = 4;

class OLog {
private:
    char *logDir = NULL;
    char *logFilePrefix = NULL;
    char *logFilePostfix = NULL;
    uint64_t startTimeOfThisDay = 0;

    FILE *file = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char encodeArr[LOG_ENCODE_LENGTH];
    size_t encodeIndex;

    size_t writeBufIndex;
    char writeBuf[WRITE_BUF_LEN];
    char encodeBuf[WRITE_BUF_LEN];

private:
    int  initFromExistFile(const char* pcPath);
    void createAndInit(const char* pcPath);

    void cacheWrite(FILE* fp, const void* vdata, size_t len);
    void writeNewLine(FILE* fp, int encode);

    void logInit_noLock();
    void logWrite_noLock(const char *logCont);
    void logUninit_noLock();

    void formatLogFileDate(char *dateStrBuf);
    char fake_rand(char cur);

public:
    void logInit(const char *dir, const char *prefix, const char *postfix, const char *logEncodeArr, size_t encodeArrLength);
    void logWrite(const char *str);
    void log_vprint(char prio, const char *tag, const char *fmt, va_list args);
    void logUninit();
};

extern void logPrint(int level, const char *tag, const char *fmt, ...);
extern void logVPrint(int level, const char *tag, const char *fmt, va_list args);

#define LOGV(fmt, ...) logPrint(LEVEL_VERBOSE, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) logPrint(LEVEL_DEBUG,   LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) logPrint(LEVEL_INFO,    LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) logPrint(LEVEL_WARNING, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) logPrint(LEVEL_ERROR,   LOG_TAG, fmt, ##__VA_ARGS__)
