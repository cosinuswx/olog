package com.winom.ologsample;

import android.app.Application;

import com.winom.olog.Log;
import com.winom.olog.LogImpl;

/**
 * @author kevinhuang
 * @since 2017-02-28
 */
public class SampleApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();

        Log.setLogImpl(new LogImpl(getExternalFilesDir(null).getAbsolutePath(), "Sample", ".flog"));
        Log.setLogLevel(Log.LEVEL_VERBOSE);
        Log.setLogToLogcat(false);
    }
}
