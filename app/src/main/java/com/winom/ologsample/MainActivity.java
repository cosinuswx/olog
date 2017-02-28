package com.winom.ologsample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.winom.olog.Log;

import java.util.Random;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        for (int i = 0; i < 10; ++i) {
            new WriteThread().start();
        }
    }
    
    public static class WriteThread extends Thread {

        @Override
        public void run() {
            Random rand = new Random(System.currentTimeMillis());
            for (int i = 0; i < 100; ++i) {
                int len = rand.nextInt() % 100 + 10;
                StringBuilder builder = new StringBuilder();
                for (int j = 0; j < len; ++j) {
                    builder.append(j % 10);
                }
                Log.d(TAG, builder.toString());

                try {
                    sleep(3);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
