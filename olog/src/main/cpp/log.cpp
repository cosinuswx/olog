#include "log.h"
#include "common.h"
#include <inttypes.h>

/* 定义magic数*/
#define LOG_MAGIC_HEAD 0x41530949
#define LOG_MAGIC_END  0xb303423a

typedef char* PCHAR;

// 将内容放在缓存里面，到达一定数目再调用fwrite
void OLog::cacheWrite(FILE *fp, const void *vdata, size_t len) {
    if (NULL == file) {
        return;
    }

    char *data = (char *) vdata;

    // 写入的数据会超出缓冲区，先填满，然后写入
    while (writeBufIndex + len > WRITE_BUF_LEN) {
        memcpy(writeBuf + writeBufIndex, data, WRITE_BUF_LEN - writeBufIndex);
        data += (WRITE_BUF_LEN - writeBufIndex);
        len = len - (WRITE_BUF_LEN - writeBufIndex);
        fwrite(writeBuf, WRITE_BUF_LEN, 1, fp);
        writeBufIndex = 0;
    }

    memcpy(writeBuf + writeBufIndex, data, len);
    writeBufIndex += len;
}

void OLog::writeNewLine(FILE *fp, int encode) {
    char newLine = '\n';
    if (encode == 1) {
        if (encodeIndex == LOG_ENCODE_LENGTH) encodeIndex = 0;
        newLine = newLine ^ encodeArr[encodeIndex];
        encodeIndex = encodeIndex + 1;
    }
    cacheWrite(fp, &newLine, 1);
}

// 根据已经存在的文件来初始化
int OLog::initFromExistFile(const char *pcPath) {
    if (access(pcPath, R_OK) == -1) {
        return -1;
    }

    int i;
    file = fopen(pcPath, "a+");

    // 检查下文件的长度，最少要等于int + LOG_ENCODE_LENGTH + int
    size_t prefLen = sizeof(int) + LOG_ENCODE_LENGTH * sizeof(char) + sizeof(int);
    fseek(file, 0L, SEEK_END);
    size_t size = (size_t) ftell(file);
    if (size < prefLen) {
        fclose(file);
        return -1;
    }

    // 读取出来内容
    fseek(file, 0L, SEEK_SET);
    char *buf = (char *) malloc(sizeof(char) * prefLen);
    fread(buf, 1, prefLen, file);
    for (i = 0; i < LOG_ENCODE_LENGTH; ++i) {
        encodeArr[i] = buf[sizeof(int) + i];
    }
    free(buf);

    encodeIndex = size % LOG_ENCODE_LENGTH;
    writeNewLine(file, 1);

    return 0;
}

// 没有文件，需要自己创建并初始化
void OLog::createAndInit(const char *pcPath) {
    file = fopen(pcPath, "ar+");
    if (NULL == file) {
        return;
    }

    // 写入个文件头
    int magic = LOG_MAGIC_HEAD;
    cacheWrite(file, &magic, sizeof(int));
    cacheWrite(file, encodeArr, LOG_ENCODE_LENGTH);

    // 密码完了之后，再写入个magic
    magic = LOG_MAGIC_END;
    cacheWrite(file, &magic, sizeof(int));

    // 对齐，按照2 * LOG_ENCODE_LENGTH的长度对齐
    int len = LOG_ENCODE_LENGTH - sizeof(int) * 2;
    for (int i = 0; i < len; ++i) {
        writeNewLine(file, 0);
    }
}

void OLog::formatLogFileDate(char *dateStrBuf) {
    time_t currentTime = (time_t) (time(NULL) + CHINA_UTC_OFFSET / 1000);
    struct tm dateBuf;
    struct tm *curDate = gmtime_r(&currentTime, &dateBuf);

    sprintf(dateStrBuf, "%04d%02d%02d", 1900 + curDate->tm_year, 1 + curDate->tm_mon,
            curDate->tm_mday);
}

void OLog::logInit_noLock() {
    // 今天的开始时间(加上8小时偏移)
    uint64_t tick = currentTimeMillis() + CHINA_UTC_OFFSET;
    startTimeOfThisDay = tick / MILLSECOND_OF_DAY * MILLSECOND_OF_DAY;

    file = NULL;
    encodeIndex = 0;
    writeBufIndex = 0;

    char *dateBuf = (char *) malloc(8 + 1);
    formatLogFileDate(dateBuf);

    size_t filePathLen = strlen(logDir) + 1 + strlen(logFilePrefix) + 1 + strlen(dateBuf) +
                         strlen(logFilePostfix);
    char *filePath = (char *) malloc(filePathLen + 1);

    sprintf(filePath, "%s/%s_%s%s", logDir, logFilePrefix, dateBuf, logFilePostfix);
    if (initFromExistFile(filePath) == -1) {
        createAndInit(filePath);
    }

    free(filePath);
    free(dateBuf);
}

void OLog::logWrite_noLock(const char *logCont) {
    size_t len = strlen(logCont), tmpLen;
    size_t i, decodedLen;

    decodedLen = 0;
    while (len > 0) {
        if (len >= sizeof(encodeBuf)) {
            tmpLen = sizeof(encodeBuf);
        } else {
            tmpLen = len;
        }

        for (i = 0; i < tmpLen; ++i, ++encodeIndex) {
            if (encodeIndex == LOG_ENCODE_LENGTH) {
                encodeIndex = 0;
            }
            encodeBuf[i] = logCont[decodedLen + i] ^ encodeArr[encodeIndex];
        }
        cacheWrite(file, encodeBuf, tmpLen);
        len -= tmpLen;
        decodedLen += tmpLen;
    }
}

void OLog::logUninit_noLock() {
    if (NULL != file) {
        if (writeBufIndex > 0) {
            fwrite(writeBuf, writeBufIndex, 1, file);
        }
        fflush(file);
        fclose(file);

        file = NULL;
    }
}

void OLog::logInit(const char *dir, const char *prefix, const char *postfix,
                   const char *logEncodeArr, size_t encodeArrLength) {
    logDir = duplicateString(dir);
    logFilePrefix = duplicateString(prefix);
    logFilePostfix = duplicateString(postfix);
    memcpy(encodeArr, logEncodeArr, encodeArrLength);

    pthread_mutex_lock(&mutex);
    logInit_noLock();
    pthread_mutex_unlock(&mutex);
}

char OLog::fake_rand(char cur) {
    return (char) (((cur * 214013L + 2531011L) >> 16) & 0x7fff);
}

void OLog::logWrite(int arrNum, ...) {
    pthread_mutex_lock(&mutex);

    // 到了第二天,生成另外一个日志文件
    if (currentTimeMillis() + CHINA_UTC_OFFSET - startTimeOfThisDay > MILLSECOND_OF_DAY) {
        logUninit_noLock();

        // 在当前加密串的基础上处理下加密串
        for (int i = 0; i < LOG_ENCODE_LENGTH; ++i) {
            encodeArr[i] = fake_rand(encodeArr[i]);
        }

        logInit_noLock();
    }

    va_list argptr;
    va_start(argptr, arrNum);
    const char* str;
    while (arrNum--) {
        str = va_arg(argptr, PCHAR);
        logWrite_noLock(str);
    }

    pthread_mutex_unlock(&mutex);
}

void OLog::logUninit() {
    pthread_mutex_lock(&mutex);
    logUninit_noLock();
    pthread_mutex_unlock(&mutex);
}

void OLog::log_vprint(char prio, const char *tag, const char *fmt, va_list args) {
    char buf1[LOG_BUF_SIZE + 1];
    char buf2[LOG_BUF_SIZE + 1];

    const char *formatstr = "[%c][%"PRIu64"][%s][%ld][";
    snprintf(buf1, LOG_BUF_SIZE, formatstr, prio, currentTimeMillis(), tag, gettid());
    vsnprintf(buf2, LOG_BUF_SIZE, fmt, args);

    logWrite(3, buf1, buf2, "\n");
}
