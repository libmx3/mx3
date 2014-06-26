package com.mx3;

import android.util.Log;

public class Api {
    static {
        System.loadLibrary("mx3_jni");
    }
    
    private long nativeHandle;
    
    Api() {}

    public native String nativeSayHi();
    
    //Trivial method that prints a welcome message with the date and time.
    public String sayHi() {
        return nativeSayHi();
    }


    public static void main(String[] args) {
        Api api = new Api();
        api.sayHi();
    }
}
    