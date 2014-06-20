package com.mx3;

import android.util.Log;

public class Api {
    static {
        System.loadLibrary("mx3_jni");
    }
    
    private long nativeHandle;
    
    Api() {}

    public native String nativeSayHi();
    
    public String sayHi() {
    	Log.d("MyApp",nativeSayHi());
        return nativeSayHi();
    }


    public static void main(String[] args) {
        Api api = new Api();
        api.sayHi();
        //System.out.println("Hello, World!");
    }
}
    