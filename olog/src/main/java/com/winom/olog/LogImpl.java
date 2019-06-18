package com.winom.olog;

import java.security.InvalidParameterException;
import java.util.Random;

/**
 * @since 2016-07-23
 * @author kevinhuang
 */
public class LogImpl implements OLog.ILog {
    private final static String[] lvlStr = {"V", "D", "I", "W", "E"};

    // 日志的加密字串的长度，要与log.h保持同步
    private final static int LOG_ENCODE_LENGTH = 32;

    private String mLogDir;
    private String mPrefix;
    private String mPostfix;

    public LogImpl(String logDir, String perfix, String postfix) {
        mLogDir = logDir;
        mPrefix = perfix;
        mPostfix = postfix;

        init();
    }

    private void init() {
        // ndk中srand在某些机器上面找不到，还是直接传入吧
        byte[] key = generateRandomByteArr(LOG_ENCODE_LENGTH);
        LogEntry.logInit(mLogDir, mPrefix, mPostfix, key);
    }

    @Override
    public void logWriter(int lvl, String tag, String text) {
        LogEntry.logWrite("[" + lvlStr[lvl] + "]" + "[" + System.currentTimeMillis() + "]" + "[" + tag + "]" +
                "[" + Thread.currentThread().getId() + "]" + "[" + text);
    }

    @Override
    public void uninit() {
        LogEntry.logUninit();
    }

    @SuppressWarnings("SameParameterValue")
    private static byte[] generateRandomByteArr(int len) {
        if ((len % 4) != 0) {
            throw new InvalidParameterException("length must be in multiples of four");
        }

        byte[] data = new byte[len];
        Random random = new Random();
        for (int i = 0; i < len; i += 4) {
            int val = random.nextInt();
            data[i] = (byte) (val >> 24);
            data[i + 1] = (byte) (val >> 16);
            data[i + 2] = (byte) (val >> 8);
            data[i + 3] = (byte) val;
        }
        return data;
    }
}
