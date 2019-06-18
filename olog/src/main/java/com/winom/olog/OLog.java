package com.winom.olog;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;

public class OLog {
    final static String TAG = "OLog";

    public interface ILog {
        void logWriter(int lvl, String tag, String text);

        void uninit();
    }

    public final static int LEVEL_VERBOSE = 0;
    public final static int LEVEL_DEBUG = 1;
    public final static int LEVEL_INFO = 2;
    public final static int LEVEL_WARNING = 3;
    public final static int LEVEL_ERROR = 4;

    public static ILog gLogImpl = new EmptyLog();

    private static int gLogLevel = LEVEL_INFO;
    private static boolean gLogToLogcat = false;

    public static void setLogLevel(int level) {
        gLogLevel = level;
        LogEntry.setLogProperty(gLogLevel, gLogToLogcat);
    }

    public static void setLogImpl(ILog impl) {
        gLogImpl = impl;
    }

    public static void setLogToLogcat(boolean logToLogcat) {
        gLogToLogcat = logToLogcat;
        LogEntry.setLogProperty(gLogLevel, gLogToLogcat);
    }

    public static void uninitLog() {
        gLogImpl.uninit();
    }

    public static void v(String tag, String text) {
        if (LEVEL_VERBOSE < gLogLevel) {
            return;
        }

        gLogImpl.logWriter(LEVEL_VERBOSE, tag, text);
        if (gLogToLogcat) {
            android.util.Log.v(tag, text);
        }
    }

    public static void d(String tag, String text) {
        if (LEVEL_DEBUG < gLogLevel) {
            return;
        }

        gLogImpl.logWriter(LEVEL_DEBUG, tag, text);
        if (gLogToLogcat) {
            android.util.Log.d(tag, text);
        }
    }

    public static void i(String tag, String text) {
        if (LEVEL_INFO < gLogLevel) {
            return;
        }

        gLogImpl.logWriter(LEVEL_INFO, tag, text);
        if (gLogToLogcat) {
            android.util.Log.i(tag, text);
        }
    }

    public static void w(String tag, String text) {
        if (LEVEL_WARNING < gLogLevel) {
            return;
        }

        gLogImpl.logWriter(LEVEL_WARNING, tag, text);
        if (gLogToLogcat) {
            android.util.Log.w(tag, text);
        }
    }

    public static void e(String tag, String text) {
        if (LEVEL_ERROR < gLogLevel) {
            return;
        }

        gLogImpl.logWriter(LEVEL_ERROR, tag, text);
        if (gLogToLogcat) {
            android.util.Log.e(tag, text);
        }
    }

    public static void e(String tag, String text, Throwable ex) {
        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        ex.printStackTrace(printWriter);
        Throwable cause = ex.getCause();
        while (cause != null) {
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();

        e(tag, text + "\n" + writer.toString());
    }

    public static void printStack(String tag, Throwable ex) {
        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        ex.printStackTrace(printWriter);
        Throwable cause = ex.getCause();
        while (cause != null) {
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();

        e(tag, writer.toString());
    }

    public static void v(String tag, String format, Object... args) {
        v(tag, String.format(format, args));
    }

    public static void d(String tag, String format, Object... args) {
        d(tag, String.format(format, args));
    }

    public static void i(String tag, String format, Object... args) {
        i(tag, String.format(format, args));
    }

    public static void w(String tag, String format, Object... args) {
        w(tag, String.format(format, args));
    }

    public static void e(String tag, String format, Object... args) {
        e(tag, String.format(format, args));
    }

    static class EmptyLog implements ILog {

        @Override
        public void logWriter(int lvl, String tag, String text) {
        }

        @Override
        public void uninit() {
        }
    }
}