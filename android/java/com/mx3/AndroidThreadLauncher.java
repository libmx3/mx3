package com.mx3;

public class AndroidThreadLauncher extends ThreadLauncher {
    public AndroidThreadLauncher() {
    }

    public void startThread(String name, final AsyncTask runTask) {
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                runTask.execute();
            }
        });
        if (name != null) {
            thread.setName(name);
        }
        thread.setDaemon(true);
        thread.start();
    }
}
