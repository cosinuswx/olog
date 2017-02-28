#include <jni.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <unistd.h>
#include <errno.h>
#define __STDC_FORMAT_MACROS

#define TRUE  1
#define FALSE 0

#define MILLSECOND_OF_HOUR (60LL * 60 * 1000)
#define MILLSECOND_OF_DAY  (24LL * 60 * 60 * 1000)
#define CHINA_UTC_OFFSET   (8 * MILLSECOND_OF_HOUR)

uint64_t currentTimeMillis();
char* duplicateString(const char* origStr);