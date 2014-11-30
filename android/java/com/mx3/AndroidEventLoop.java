package com.mx3;

import android.os.Handler;
import android.os.Looper;

public class AndroidEventLoop extends EventLoop {
    Handler mHandler;
    public AndroidEventLoop() {
        mHandler = new Handler(Looper.getMainLooper());
    }
    public void post(final AsyncTask task) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                task.execute();
            }
        });
    }
}
