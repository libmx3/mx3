package com.mx3;

public class Api {
    static {
        System.loadLibrary("mx3_jni");
    }

    Api() {}

    public void sayHi() {
        nativeSayHi();
    }

    private static native void nativeSayHi();

    public static void main(String[] args) {
        Api api = new Api();
        api.sayHi();
        System.out.println("Hello, World!");
    }
}
