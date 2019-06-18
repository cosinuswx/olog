package com.winom.ologsample;

import android.app.Application;

import com.winom.olog.OLog;
import com.winom.olog.LogImpl;

/**
 * @author kevinhuang
 * @since 2017-02-28
 */
public class SampleApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();

        OLog.setLogImpl(new LogImpl(getExternalFilesDir(null).getAbsolutePath(), "sample", ".olog"));
        OLog.setLogLevel(OLog.LEVEL_VERBOSE);
        OLog.setLogToLogcat(false);
    }
}
