package com.mx3;

import android.util.Log;

public class Api {
    static {
        System.loadLibrary("mx3_jni");
    }
    
    private long nativeHandle;
    
    Api() {}

    public native int nativeSayHi();
    
    public void sayHi() {
        Log.d("MyApp",Integer.toString(nativeSayHi()));
    }


    public static void main(String[] args) {
        Api api = new Api();
        api.sayHi();
        //System.out.println("Hello, World!");
    }
}
