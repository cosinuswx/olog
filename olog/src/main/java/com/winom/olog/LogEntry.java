package com.winom.olog;

/**
 * @author kevinhuang
 * @since 2017-02-23
 */
public class LogEntry {
    static {
        System.loadLibrary("olog");
    }

    public static native void setLogProperty(int logLevel, boolean toLogCat);

    public static native void logInit(String logDir, String logFilePrefix, String logFilePostfix, byte[] encodeArr);

    public static native void logWrite(String log);

    public static native void logUninit();
}
